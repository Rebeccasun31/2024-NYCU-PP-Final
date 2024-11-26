#pragma once

__declspec(align(0x10)) struct point
{
    point();
    point(unsigned int seed);
    point(float x, float y, float z, float r, float g, float b, float size,
        float mass, float sx, float sy, float sz);

#ifdef _DEBUG
    void log();
    void log(unsigned int seed);
#endif

    __declspec(align(0x10)) float _x;
    float _y;
    float _z;
    __declspec(align(0x10)) float _r;
    float _g;
    float _b;
    float _size;
    __declspec(align(0x10)) float _sx; // speed
    float _sy;
    float _sz;
    __declspec(align(0x10)) float _mass;
};