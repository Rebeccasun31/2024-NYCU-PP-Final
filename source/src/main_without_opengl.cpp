#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <bits/stdc++.h>
#include <chrono>

#include "./header/Object.h"
#include "./header/stb_image.h"
#include "./header/point.h"
#include "./header/nbody.h"
#include "./header/camera.h"

#ifdef USE_OMP
#include <omp.h>
#include <string>
#endif

#ifdef USE_MPI
#include "C:/Program Files (x86)/Microsoft SDKs/MPI/Include/mpi.h"
int world_rank, world_size;
#endif

static point vertices_1[POINT_CNT];
static point vertices_2[POINT_CNT];
point *points1 = vertices_1, *points2 = vertices_2;
int cnt = 0;
double total_time = 0.0f;

void init();


int main(int argc, char* argv[]) {

#ifdef USE_OMP
    if (argc >= 2) {
        omp_set_num_threads(std::stoi(argv[1]));
    } 
#endif

#ifdef USE_MPI
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    MPI_Datatype mpi_point_type;
    vertices_1[0] = point();

    int block_lengths[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    MPI_Datatype types[12] = {
        MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, // _x, _y, _z
        MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, // _r, _g, _b
        MPI_FLOAT,                       // _size
        MPI_FLOAT,                       // _mass
        MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, // _sx, _sy, _sz
        MPI_INT                          // _character
    };

    // 計算偏移量
    MPI_Aint base_address, displacements[12];
    MPI_Get_address(&vertices_1[0], &base_address);
    MPI_Get_address(&vertices_1[0]._x, &displacements[0]);
    MPI_Get_address(&vertices_1[0]._y, &displacements[1]);
    MPI_Get_address(&vertices_1[0]._z, &displacements[2]);
    MPI_Get_address(&vertices_1[0]._r, &displacements[3]);
    MPI_Get_address(&vertices_1[0]._g, &displacements[4]);
    MPI_Get_address(&vertices_1[0]._b, &displacements[5]);
    MPI_Get_address(&vertices_1[0]._size, &displacements[6]);
    MPI_Get_address(&vertices_1[0]._mass, &displacements[7]);
    MPI_Get_address(&vertices_1[0]._sx, &displacements[8]);
    MPI_Get_address(&vertices_1[0]._sy, &displacements[9]);
    MPI_Get_address(&vertices_1[0]._sz, &displacements[10]);
    MPI_Get_address(&vertices_1[0]._character, &displacements[11]);

    for (int i = 0; i < 12; ++i) {
        displacements[i] -= base_address;
    }

    MPI_Type_create_struct(12, block_lengths, displacements, types, &mpi_point_type);
    MPI_Type_commit(&mpi_point_type);

    int point_cnt_per_process = POINT_CNT / world_size;
	int start_i = point_cnt_per_process * world_rank;
	int end_i = (world_rank == world_size - 1) ? POINT_CNT : (start_i + point_cnt_per_process);

    if (world_rank != 0) {
        point vertices_tmp[end_i - start_i];
        point *points_tmp = vertices_tmp;

        double dt;
        while (true) {
            MPI_Bcast(&dt, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            if (dt == -1.0) {
                MPI_Finalize();
                return 0;
            }
            MPI_Bcast(points1, POINT_CNT, mpi_point_type, 0, MPI_COMM_WORLD);
            nBodyCalculateMPI(points1, points_tmp, dt * DELTA_TIME_MUL, start_i, end_i);
            MPI_Send(points_tmp, end_i - start_i, mpi_point_type, 0, 0, MPI_COMM_WORLD);
        }
    }
#endif

    init();

    // render loop variables
    double dt = 0.0;
    double lastTime = glfwGetTime();
    double currentTime;

    
    while (1) {
        point* tmp;
        
        // nbody
        auto start = std::chrono::high_resolution_clock::now();
#ifdef USE_SERIAL
        nBodyCalculateSerial(points1, points2, dt * DELTA_TIME_MUL);
#endif
#ifdef USE_OMP
        nBodyCalculateOMP(points1, points2, dt * DELTA_TIME_MUL);
#endif
#ifdef USE_MPI
        // std::cout << "[Bcast send] rank: " << world_rank << " dt: " << dt << " p0: " << points1[0]._sx << '\n';

        MPI_Bcast(&dt, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(points1, POINT_CNT, mpi_point_type, 0, MPI_COMM_WORLD);
        nBodyCalculateMPI(points1, points2, dt * DELTA_TIME_MUL, start_i, end_i);

        // std::cout << "[Nbody] rank: " << world_rank << " dt: " << dt << " p0: " << points1[0]._sx << '\n';

        int cur_recv = end_i;
        for (int i = 1; i < world_size; i++) {
            int cur_start_i = point_cnt_per_process * i;
	        int cur_end_i = (i == world_size - 1) ? POINT_CNT : (cur_start_i + point_cnt_per_process);
            MPI_Recv(&points2[cur_recv], cur_end_i - cur_start_i, mpi_point_type, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // std::cout << "[Recv] rank: " << world_rank << " dt: " << dt << " pi: " << points1[i]._sx << '\n';
            cur_recv += cur_end_i - cur_start_i;
        }
#endif
#ifdef USE_CUDA
        nBodyCalculateCUDA(points1, points2, dt * DELTA_TIME_MUL);
#endif
        auto end = std::chrono::high_resolution_clock::now();
        total_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cnt += 1;
        // std::cout << cnt << ' ';

        if (cnt >= 100) {
#ifdef USE_SERIAL
            std::cout << "[serial]\n";
#endif
#ifdef USE_OMP
            int num_threads;
            int num_processors;
            #pragma omp parallel
            {
                num_threads = omp_get_num_threads();
                num_processors = omp_get_num_procs();
            }
            std::cout << "[OpenMP] Number of Threads / Processors: " << num_threads << " / " << num_processors << '\n';
#endif
#ifdef USE_MPI
            dt = -1.0;
            MPI_Bcast(&dt, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            MPI_Finalize();
            std::cout << "[MPI] Number of Processors: " << world_size << '\n';
#endif
            std::cout << "Number of Points: " << POINT_CNT << '\n';
            std::cout << "N-Body Average Elapsed Time: " << total_time / (double) cnt << " ms\n\n";
            return 0;
        }

        tmp = points1;
        points1 = points2;
        points2 = tmp;

        // Status update
        currentTime = glfwGetTime();
        dt = currentTime - lastTime;
        lastTime = currentTime;
    }

#ifdef USE_MPI
    dt = -1.0;
    MPI_Bcast(&dt, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif

    return 0;
}


void init() {
    for (int i = 0; i < POINT_CNT; ++i) {
		unsigned int seed = SEED;
		vertices_1[i] = point(seed);
		vertices_2[i] = vertices_1[i];
	}
}