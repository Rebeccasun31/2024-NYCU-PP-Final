#pragma once

#define ESTIMATION

#define DEMO_TESTING
// #define DEMO_GRAVITATIONAL_SLINGSHOT
// #define DEMO_BINARY_STAR_SYSTEM
// #define DEMO_REAL_WORLD

// #define VERBOSE
// #define DISABLE_LIGHT

#ifdef DEMO_TESTING
#   define SEED 0xdeadbeef - 0x55665566 * i
#	define GRAVITATIONAL_G 6.6743
#   define DELTA_TIME_MUL 100
#	define POINT_CNT 10000
#	define POINT_XYZ_MAX  30000.0
#	define POINT_SIZE_MAX 10.0
#	define POINT_MASS_MAX 100.0
#	define POINT_SPEED_MAX 100.0
#endif

#ifdef DEMO_GRAVITATIONAL_SLINGSHOT
#   define SEED 0xdeadbeef - 7000 * i
#	define GRAVITATIONAL_G 6.6743
#   define DELTA_TIME_MUL 4000
#	define POINT_CNT 4
#	define POINT_XYZ_MAX  5000.0
#	define POINT_SIZE_MAX 50.0
#	define POINT_MASS_MAX 100.0
#	define POINT_SPEED_MAX 1.0
#endif

#ifdef DEMO_BINARY_STAR_SYSTEM
#   define SEED 12345680 - 2000 * i
#	define GRAVITATIONAL_G 6.6743
#   define DELTA_TIME_MUL 4000
#	define POINT_CNT 2
#	define POINT_XYZ_MAX  5000.0
#	define POINT_SIZE_MAX 50.0
#	define POINT_MASS_MAX 100.0
#	define POINT_SPEED_MAX 1.0
#endif

#ifdef DEMO_REAL_WORLD
#   define SEED 87654321 - 100 * i

// Unit:
// - Length: 1 (x/y/z) = 9.461 x 10^15 m (1 Light-Year)
// - Mass  : 1 (mass)  = 1.989 x 10^30 kg (Mass of sun)
// - G     : 6.67430 x 10^(-11) (N x m^2 x kg^(-2))
// - Speed : 1 = 9.461 x 10^15 m per second

// m = 9.461 * 10^15
// kg = 1.989 * 10^30
// kg * G = 6.67430 * 10^(-11) * m^2 * kg^(-1)
#	define GRAVITATIONAL_G 3.003620262997989e-09
// 1 Month
#   define DELTA_TIME_MUL 2.628e+6
#	define POINT_CNT 1000
#	define POINT_XYZ_MAX  10000.0
// Just for visualization...
#	define POINT_SIZE_MAX 5.0
// R136a1
#	define POINT_MASS_MAX 351.0
// S4716
#	define POINT_SPEED_MAX 8.455765775288024e-10
#endif

#ifndef GRAVITATIONAL_G
#	define GRAVITATIONAL_G 6.6743
#endif

#ifndef DELTA_TIME_MUL
#	define DELTA_TIME_MUL 1000
#endif

#ifndef POINT_CNT
#	define POINT_CNT 15
#endif

#ifndef POINT_XYZ_MAX
#	define POINT_XYZ_MAX  10000.0
#endif

#ifndef POINT_SIZE_MAX
#	define POINT_SIZE_MAX 50.0
#endif

#ifndef POINT_MASS_MAX
#	define POINT_MASS_MAX 100.0
#endif

#ifndef POINT_SPEED_MAX
#	define POINT_SPEED_MAX 1.0
#endif
