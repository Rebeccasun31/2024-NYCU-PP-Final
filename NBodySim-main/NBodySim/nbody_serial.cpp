#include <math.h>

#include "option.h"
#include "nbody.h"

#ifdef VERBOSE
#include "stdio.h"
#endif

// Ref: https://yangwc.com/2019/06/20/NbodySimulation/
static void _nBodyCalculate(const point *currpoints, point *newpoint, int i, double dt)
{
	float G = GRAVITATIONAL_G;
	float epi = 0.0000000000001;
	// Acceleration
	float ax = 0;
	float ay = 0;
	float az = 0;

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
	newpoint->_sx = currpoints[i]._sx + ax * dt;
	newpoint->_sy = currpoints[i]._sy + ay * dt;
	newpoint->_sz = currpoints[i]._sz + az * dt;

	// Update position
	newpoint->_x = currpoints[i]._x + newpoint->_sx * dt;
	newpoint->_y = currpoints[i]._y + newpoint->_sy * dt;
	newpoint->_z = currpoints[i]._z + newpoint->_sz * dt;
}

void nBodyCalculateSerial(const point *currpoints, point *newpoints, double dt)
{
	for (int i = 0; i < POINT_CNT; ++i) {
		_nBodyCalculate(currpoints, &newpoints[i], i, dt);

#ifdef _DEBUG
#	ifdef VERBOSE
		newpoints[i].log();
#	endif
#endif
	}

#ifdef _DEBUG
#	ifdef VERBOSE
	printf("\n");
#	endif
#endif
}
