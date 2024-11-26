#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#include <iostream>
#include <assert.h>

#include <stdlib.h>

#include "option.h"
#include "nbody.h"

using namespace std;

#define THREAD_NUM 1024

class CUDAContext
{
public:
    CUDAContext();
    ~CUDAContext();

    static void createContext();

    static int inited;
    static point *CudaCurrpoints;
    static point *CudaResults;
};

CUDAContext::CUDAContext()
{
    cudaError_t cudaStatus;

    // Choose which GPU to run on, change this on a multi-GPU system
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaSetDevice failed! Do you have a CUDA-capable GPU installed?" << endl;
        exit(EXIT_FAILURE);
    }

    cudaStatus = cudaMalloc((void **)&CudaCurrpoints, sizeof(point) * POINT_CNT);
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaMalloc failed!" << endl;
        exit(EXIT_FAILURE);
    }

    cudaStatus = cudaMalloc((void **)&CudaResults, sizeof(point) * POINT_CNT);
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaMalloc failed!" << endl;
        exit(EXIT_FAILURE);
    }

    cout << "CUDA construct done" << endl;
}

CUDAContext::~CUDAContext()
{
    cudaFree(CudaResults);
    cudaFree(CudaCurrpoints);

    cout << "CUDA destruct done" << endl;
}

void CUDAContext::createContext()
{
    assert(POINT_CNT < BLOCK_NUM *THREAD_NUM && "POINT_CNT is too large");
    static CUDAContext context;
}

int CUDAContext::inited = 0;
point *CUDAContext::CudaCurrpoints = NULL;
point *CUDAContext::CudaResults = NULL;

__global__ static void _nBodyCalculateCUDA(const point *currpoints, point *newpoints, double dt)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;

    if (i >= POINT_CNT) {
        return;
    }

    float G = GRAVITATIONAL_G;
    float epi = 0.0000000000001;
    // Acceleration
    float ax = 0;
    float ay = 0;
    float az = 0;
    float sx = 0;
    float sy = 0;
    float sz = 0;
    float x = 0;
    float y = 0;
    float z = 0;

    #pragma unroll POINT_CNT
    for (int j = 0; j < POINT_CNT; ++j) {
        float rx;
        float ry;
        float rz;
        float lenpow2;
        float len;

        rx = currpoints[j]._x - currpoints[i]._x;
        ry = currpoints[j]._y - currpoints[i]._y;
        rz = currpoints[j]._z - currpoints[i]._z;

        lenpow2 = rx * rx + ry * ry + rz * rz + epi;
        len = sqrt(lenpow2);

        ax += currpoints[j]._mass * rx / len / lenpow2;
        ay += currpoints[j]._mass * ry / len / lenpow2;
        az += currpoints[j]._mass * rz / len / lenpow2;
    }

    ax *= G;
    ay *= G;
    az *= G;

    // Update speed
    sx = currpoints[i]._sx + ax * dt;
    sy = currpoints[i]._sy + ay * dt;
    sz = currpoints[i]._sz + az * dt;

    newpoints[i]._sx = sx;
    newpoints[i]._sy = sy;
    newpoints[i]._sz = sz;

    // Update position
    x = currpoints[i]._x + sx * dt;
    y = currpoints[i]._y + sy * dt;
    z = currpoints[i]._z + sz * dt;

    newpoints[i]._x = x;
    newpoints[i]._y = y;
    newpoints[i]._z = z;

    // Copy others property
    newpoints[i]._r = currpoints[i]._r;
    newpoints[i]._g = currpoints[i]._g;
    newpoints[i]._b = currpoints[i]._b;
    newpoints[i]._mass = currpoints[i]._mass;
    newpoints[i]._size = currpoints[i]._size;
}

void nBodyCalculateCUDA(const point *currpoints, point *newpoints, double dt)
{
    cudaError_t cudaStatus;
    int block_num = (POINT_CNT + THREAD_NUM - 1) / THREAD_NUM;

    if (!CUDAContext::inited) {
        CUDAContext::createContext();
        CUDAContext::inited = 1;
    }

    cudaStatus = cudaMemcpy(CUDAContext::CudaCurrpoints, currpoints, sizeof(point) * POINT_CNT, cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaMemcpy failed!" << endl;
        exit(EXIT_FAILURE);
    }

    _nBodyCalculateCUDA<<<block_num, THREAD_NUM>>>(CUDAContext::CudaCurrpoints, CUDAContext::CudaResults, dt);

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        cerr << "addKernel launch failed: " << cudaGetErrorString(cudaStatus) << endl;
        exit(EXIT_FAILURE);
    }

    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaDeviceSynchronize returned error code " << cudaStatus << " after launching addKernel!" << endl;
        exit(EXIT_FAILURE);
    }

    // Copy output vector from GPU buffer to host memory.
    cudaStatus = cudaMemcpy(newpoints, CUDAContext::CudaResults, sizeof(point) * POINT_CNT, cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        cerr << "cudaMemcpy failed!" << endl;
        exit(EXIT_FAILURE);
    }
}