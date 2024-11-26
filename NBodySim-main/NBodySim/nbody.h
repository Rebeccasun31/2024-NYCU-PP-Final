#pragma once
#include "point.h"

typedef void (*nBodyFunc)(const point *currpoints, point *newpoints, double dt);

void nBodyCalculateSerial(const point *currpoints, point *newpoints, double dt);

void nBodyCalculateSerialSIMD(const point *currpoints, point *newpoints, double dt);

void nBodyCalculateSerialSIMD256(const point *currpoints, point *newpoints, double dt);

void nBodyCalculateParallel(const point *currpoints, point *newpoints, double dt);

void nBodyCalculateParallelSIMD(const point *currpoints, point *newpoints, double dt);

void nBodyCalculateParallelSIMD256(const point *currpoints, point *newpoints, double dt);

void nBodyCalculateCUDA(const point *currpoints, point *newpoints, double dt);
