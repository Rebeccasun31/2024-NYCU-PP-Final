#include <math.h>
#include <iostream>
#include <cuda.h>
#include <stdlib.h>

#include "./header/nbody_cuda.cuh"

// #define BLOCK_SIZE 256

__global__ void _nBodyCalculateKernel(const point* currpoints, point* newpoints, double dt) {
    int thisI = threadIdx.x + blockIdx.x * blockDim.x;
	int thisJ = threadIdx.y + blockIdx.y * blockDim.y;
    if (thisI >= POINT_CNT || thisJ >= POINT_CNT) {
		// std::cout << "thisI, thisJ out of range : " << thisI << ' ' << thisJ << '\n';
		return;
	}
	else if(thisI == thisJ) return;
    point currentPoint = currpoints[thisI];
    point targetPoint = currpoints[thisJ];
    if (currentPoint._mass == 0.0f || targetPoint._mass == 0.0f) {
        newpoints[thisI] = currentPoint;
        return;
    }

    float G = GRAVITATIONAL_G;
    float epi = 0.0000000000001;
    float ax = 0, ay = 0, az = 0;
    float r_c = currentPoint._size / 2;
    float r_t = targetPoint._size / 2;

	bool check_x = false;
	bool check_y = false;
	bool check_z = false;
	float rfx = 1.0; // border reflect check
	float rfy = 1.0;
	float rfz = 1.0;

    // Copy current point to avoid race conditions

	float rx;
	float ry;
	float rz;
	float lenpow2;
	float len;

	float dis = r_c + r_t;

	rx = targetPoint._x - currentPoint._x;
	ry = targetPoint._y - currentPoint._y;
	rz = targetPoint._z - currentPoint._z;

	lenpow2 = rx * rx + ry * ry + rz * rz + epi;
	len = sqrt(lenpow2);

	// gravity
	ax += targetPoint._mass * rx / len / lenpow2;
	ay += targetPoint._mass * ry / len / lenpow2;
	az += targetPoint._mass * rz / len / lenpow2;	
    
	// check collapse
	if (len <= dis) {
		// collapse: 0, be eaten: 1, eat: 2
		int diff_char = currentPoint._character - targetPoint._character;
		if (diff_char < 0) diff_char += 3;

		// collapse
		if (diff_char == 0) {
			float sum_m = currentPoint._mass + targetPoint._mass;
			float diff_m = currentPoint._mass - targetPoint._mass;
			// Update speed
			newpoints[thisI]._sx = (diff_m * currentPoint._sx + 2 * targetPoint._mass * targetPoint._sx) / sum_m;
			newpoints[thisI]._sy = (diff_m * currentPoint._sy + 2 * targetPoint._mass * targetPoint._sy) / sum_m;
			newpoints[thisI]._sz = (diff_m * currentPoint._sz + 2 * targetPoint._mass * targetPoint._sz) / sum_m;
		}
		// be eaten
		else if (diff_char == 1) {
			newpoints[thisI]._mass = 0;
			newpoints[thisI]._size = 0;
		}
		// eat
		else if (diff_char == 2) {
			newpoints[thisI]._mass += targetPoint._mass;
			// M1/s1 = M2/s2, s2 = M2s1/M1
			newpoints[thisI]._size = newpoints[thisI]._mass * currentPoint._size / currentPoint._mass;

			newpoints[thisI]._sx = (currentPoint._sx * currentPoint._mass + targetPoint._sx * targetPoint._mass) / newpoints[thisI]._mass;
			newpoints[thisI]._sy = (currentPoint._sy * currentPoint._mass + targetPoint._sy * targetPoint._mass) / newpoints[thisI]._mass;
			newpoints[thisI]._sz = (currentPoint._sz * currentPoint._mass + targetPoint._sz * targetPoint._mass) / newpoints[thisI]._mass;
		}
	}

	ax *= G;
	ay *= G;
	az *= G;
	
	// check border
	if(currentPoint._x >= POINT_XYZ_MAX) {
		rfx = -1.0;
		check_x = true;
	}
	else if (currentPoint._x <= -1 * POINT_XYZ_MAX){
		rfx = 1.0;
		check_x = true;
	}

	if(currentPoint._y >= POINT_XYZ_MAX) {
		rfy = -1.0;
		check_y = true;
	}
	else if (currentPoint._y <= -1 * POINT_XYZ_MAX){
		rfy = 1.0;
		check_y = true;
	}

	if(currentPoint._z >= POINT_XYZ_MAX) {
		rfz = -1.0;
		check_z = true;
	}
	else if (currentPoint._z <= -1 * POINT_XYZ_MAX){
		rfz = 1.0;
		check_z = true;
	}

	// Update speed
	newpoints[thisI]._sx += ax * dt;
	newpoints[thisI]._sy += ay * dt;
	newpoints[thisI]._sz += az * dt;

	if (check_x) newpoints[thisI]._sx = abs(newpoints[thisI]._sx) * rfx; 
	if (check_y) newpoints[thisI]._sy = abs(newpoints[thisI]._sy) * rfy; 
	if (check_z) newpoints[thisI]._sz = abs(newpoints[thisI]._sz) * rfz; 

	// Update position
	newpoints[thisI]._x = currentPoint._x + newpoints[thisI]._sx * dt;
	newpoints[thisI]._y = currentPoint._y + newpoints[thisI]._sy * dt;
	newpoints[thisI]._z = currentPoint._z + newpoints[thisI]._sz * dt;	
}

void nBodyCalculateCUDA(const point* h_currpoints, point* h_newpoints, double dt) {
    point *d_currpoints, *d_newpoints;
    size_t size = POINT_CNT * sizeof(point);

    // Allocate device memory
    cudaMalloc(&d_currpoints, size);
    cudaMalloc(&d_newpoints, size);

    // Copy input arrays to device
    cudaMemcpy(d_currpoints, h_currpoints, size, cudaMemcpyHostToDevice);

    // Configure grid and block dimensions
    // int gridSize = (POINT_CNT + BLOCK_SIZE - 1) / BLOCK_SIZE;
	dim3 threadsPerBlock(32, 32);
    dim3 numBlocks((POINT_CNT + threadsPerBlock.x - 1) / (threadsPerBlock.x), // ceiling()
                (POINT_CNT + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // Launch kernel
    _nBodyCalculateKernel<<<numBlocks, threadsPerBlock>>>(d_currpoints, d_newpoints, dt);

    // Copy result back to host
    cudaMemcpy(h_newpoints, d_newpoints, size, cudaMemcpyDeviceToHost);

    // Free device memory
    cudaFree(d_currpoints);
    cudaFree(d_newpoints);

    // Check for any CUDA errors
    cudaError_t error = cudaGetLastError();
    if (error != cudaSuccess) {
        // std::cerr << "CUDA error: " << cudaGetErrorString(error) << std::endl;
    }
}