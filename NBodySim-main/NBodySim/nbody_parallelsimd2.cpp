#include <math.h>
#include <intrin.h>
#include <string.h>

#include "option.h"
#include "nbody.h"

static void _nBodyCalculate(const point *currpoints, point *newpoint, int i, double dt,
	float *x, float *y, float *z, float *m)
{
	double G = GRAVITATIONAL_G;

	const __m256i allO = _mm256_set1_epi8(-1);
	const __m256 allZf = _mm256_set1_ps(0.0);
	const __m256 allEps = _mm256_set1_ps(0.0000000000001);

	__m256 curX = _mm256_set1_ps(x[i]);
	__m256 curY = _mm256_set1_ps(y[i]);
	__m256 curZ = _mm256_set1_ps(z[i]);

	__m256 NaX = allZf;
	__m256 NaY = allZf;
	__m256 NaZ = allZf;

	for (int j = 0; j < POINT_CNT; j += 8) {
		__m256 locX = _mm256_maskload_ps(x + j, allO);
		__m256 locY = _mm256_maskload_ps(y + j, allO);
		__m256 locZ = _mm256_maskload_ps(z + j, allO);
		__m256 mass = _mm256_maskload_ps(m + j, allO);

		__m256 difX = _mm256_sub_ps(locX, curX);
		__m256 difY = _mm256_sub_ps(locY, curY);
		__m256 difZ = _mm256_sub_ps(locZ, curZ);

		// dis = sqrt(rx * rx + ry * ry + rz * rz);
		__m256 len = _mm256_mul_ps(difX, difX);
		len = _mm256_add_ps(len, _mm256_mul_ps(difY, difY));
		len = _mm256_add_ps(len, _mm256_mul_ps(difZ, difZ));
		len = _mm256_add_ps(len, allEps);
		__m256 len2 = _mm256_sqrt_ps(len);

		// force = currpoints[j]._mass / (dis * dis * dis);
		__m256 force = _mm256_div_ps(mass, len2);
		force = _mm256_div_ps(force, len);

		// ax += force * rx;
		// ay += force * ry;
		// az += force * rz;

		__m256 accX = _mm256_mul_ps(force, difX);
		__m256 accY = _mm256_mul_ps(force, difY);
		__m256 accZ = _mm256_mul_ps(force, difZ);

		NaX = _mm256_add_ps(NaX, accX);
		NaY = _mm256_add_ps(NaY, accY);
		NaZ = _mm256_add_ps(NaZ, accZ);
	}

	NaX = _mm256_hadd_ps(NaX, NaX);
	NaX = _mm256_hadd_ps(NaX, NaX);
	float tmp[8];
	_mm256_maskstore_ps(tmp, allO, NaX);
	float ax = G * (tmp[0] + tmp[4]);

	NaY = _mm256_hadd_ps(NaY, NaY);
	NaY = _mm256_hadd_ps(NaY, NaY);
	_mm256_maskstore_ps(tmp, allO, NaY);
	float ay = G * (tmp[0] + tmp[4]);

	NaZ = _mm256_hadd_ps(NaZ, NaZ);
	NaZ = _mm256_hadd_ps(NaZ, NaZ);
	_mm256_maskstore_ps(tmp, allO, NaZ);
	float az = G * (tmp[0] + tmp[4]);

	// Update speed
	newpoint->_sx = currpoints[i]._sx + ax * dt;
	newpoint->_sy = currpoints[i]._sy + ay * dt;
	newpoint->_sz = currpoints[i]._sz + az * dt;

	// Update position
	newpoint->_x = currpoints[i]._x + newpoint->_sx * dt;
	newpoint->_y = currpoints[i]._y + newpoint->_sy * dt;
	newpoint->_z = currpoints[i]._z + newpoint->_sz * dt;
}

void nBodyCalculateParallelSIMD256(const point *currpoints, point *newpoints, double dt)
{
	static float *x = nullptr, *y, *z, *m;

	if (!x) {
		x = (float *)malloc((POINT_CNT + 16) * sizeof(float));
		y = (float *)malloc((POINT_CNT + 16) * sizeof(float));
		z = (float *)malloc((POINT_CNT + 16) * sizeof(float));
		m = (float *)malloc((POINT_CNT + 16) * sizeof(float));

		memset(&x[POINT_CNT], 0, sizeof(float) * 16);
		memset(&y[POINT_CNT], 0, sizeof(float) * 16);
		memset(&z[POINT_CNT], 0, sizeof(float) * 16);
		memset(&m[POINT_CNT], 0, sizeof(float) * 16);
	}

	for (int i = 0; i < POINT_CNT; i++) {
		x[i] = currpoints[i]._x;
		y[i] = currpoints[i]._y;
		z[i] = currpoints[i]._z;
		m[i] = currpoints[i]._mass;
	}

	#pragma omp parallel for
	for (int i = 0; i < POINT_CNT; ++i) {
		_nBodyCalculate(currpoints, &newpoints[i], i, dt, x, y, z, m);
	}
}
