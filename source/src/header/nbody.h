#pragma once
#include "point.h"

// #define USE_SERIAL 1
// #define USE_OMP 1
// #define USE_MPI 1
#define USE_CUDA 1
#ifndef KERNEL_H_
#define KERNEL_H_

// typedef void (*nBodyFunc)(const point *currpoints, point *newpoints, double dt);

void nBodyCalculateSerial(const point *currpoints, point *newpoints, double dt);
void nBodyCalculateOMP(const point *currpoints, point *newpoints, double dt);
void nBodyCalculateMPI(const point *currpoints, point *newpoints, double dt, int start_i, int end_i);
void nBodyCalculateCUDA(const point* h_currpoints, point* h_newpoints, double dt);

// void nBodyCalculateSerialSIMD(const point *currpoints, point *newpoints, double dt);

// void nBodyCalculateSerialSIMD256(const point *currpoints, point *newpoints, double dt);

// void nBodyCalculateParallel(const point *currpoints, point *newpoints, double dt);

// void nBodyCalculateParallelSIMD(const point *currpoints, point *newpoints, double dt);

// void nBodyCalculateParallelSIMD256(const point *currpoints, point *newpoints, double dt);

// void nBodyCalculateCUDA(const point *currpoints, point *newpoints, double dt);

#endif