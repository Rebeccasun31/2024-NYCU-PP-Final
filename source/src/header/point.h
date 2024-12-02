#pragma once

#define SEED 0xdeadbeef - 0x55665566 * i
#define GRAVITATIONAL_G 6.6743
#define DELTA_TIME_MUL 100
#define POINT_CNT 2
#define POINT_XYZ_MAX  1000.0
#define POINT_SIZE_MAX 50.0
#define POINT_MASS_MAX 100.0
#define POINT_SPEED_MAX 100.0


typedef struct point point;
struct point
{
    point();
    point(unsigned int seed);
    point(float x, float y, float z, float r, float g, float b, float size,
        float mass, float sx, float sy, float sz, int character);

    float _x;
    float _y;
    float _z;
    float _r;
    float _g;
    float _b;
    float _size;
    float _sx; // speed
    float _sy;
    float _sz;
    float _mass;
    int _character;
};