#pragma once
#include "point.h"

// #define USE_SERIAL 1
#define USE_OMP 1
// #define USE_MPI 1
// #define USE_CUDA 1

// #define CHIIKAWA 1   // Reminder: High resource usage!

void nBodyCalculateSerial(const point *currpoints, point *newpoints, double dt);
void nBodyCalculateOMP(const point *currpoints, point *newpoints, double dt);
void nBodyCalculateMPI(const point *currpoints, point *newpoints, double dt, int start_i, int end_i);
void nBodyCalculateCUDA(const point* h_currpoints, point* h_newpoints, double dt);