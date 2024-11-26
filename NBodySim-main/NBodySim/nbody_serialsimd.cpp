#include <math.h>
#include <intrin.h>

#include "option.h"
#include "nbody.h"

static void _nBodyCalculate(const point *currpoints, point *newpoint, int i, double dt)
{
	double G = GRAVITATIONAL_G;
	__m128 vepi;
	// Acceleration
	__m128 vaxyz;
	__m128 vG;
	__m128 vdt;
	__m128 vsxyz;
	__m128 vpxyz;
	__m128 vtmp;

	vaxyz = _mm_set_ps1(0.0);
	vepi = _mm_set_ps1(0.0000000000001);
	vG = _mm_set_ps1(G);
	vdt = _mm_set_ps1(dt);

	for (int j = 0; j < POINT_CNT; ++j) {
		__m128 vrxyz, vjxyz, vixyz;
		__m128 lenpow2;
		__m128 len;
		__m128 vmass;

		vjxyz = _mm_load_ps(&currpoints[j]._x);
		vixyz = _mm_load_ps(&currpoints[i]._x);
		vrxyz = _mm_sub_ps(vjxyz, vixyz);

		lenpow2 = _mm_mul_ps(vrxyz, vrxyz);

		// (a1, a2, a3, a4) hadd (a1, a2, a3, a4) = (a1+a2, a3+a4, a1+a2, a3+a4)
		lenpow2 = _mm_hadd_ps(lenpow2, lenpow2);
		lenpow2 = _mm_hadd_ps(lenpow2, lenpow2);
		lenpow2 = _mm_add_ps(lenpow2, vepi);
		len = _mm_sqrt_ps(lenpow2);

		vmass = _mm_set_ps1(currpoints[j]._mass);
		vmass = _mm_mul_ps(vmass, vrxyz);
		vmass = _mm_div_ps(vmass, lenpow2);
		vmass = _mm_div_ps(vmass, len);
		vaxyz = _mm_add_ps(vaxyz, vmass);
	}

	vaxyz = _mm_mul_ps(vaxyz, vG);

	// Update speed
	vsxyz = _mm_load_ps(&currpoints[i]._sx);
	vtmp = _mm_mul_ps(vaxyz, vdt);
	vsxyz = _mm_add_ps(vsxyz, vtmp);
	_mm_store_ps(&newpoint->_sx, vsxyz);

	// Update position
	vpxyz = _mm_load_ps(&currpoints[i]._x);
	vtmp = _mm_mul_ps(vsxyz, vdt);
	vpxyz = _mm_add_ps(vpxyz, vtmp);
	_mm_store_ps(&newpoint->_x, vpxyz);
}

void nBodyCalculateSerialSIMD(const point *currpoints, point *newpoints, double dt)
{
	for (int i = 0; i < POINT_CNT; ++i) {
		_nBodyCalculate(currpoints, &newpoints[i], i, dt);
	}
}
