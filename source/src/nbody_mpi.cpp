#include <math.h>
#include <iostream>
#include "C:/Program Files (x86)/Microsoft SDKs/MPI/Include/mpi.h"

#include "./header/nbody.h"
#include "./header/point.h"

// Ref: https://yangwc.com/2019/06/20/NbodySimulation/
static void _nBodyCalculate(const point *currpoints, point *newpoint, int i, double dt)
{
	float G = GRAVITATIONAL_G;
	float epi = 0.0000000000001;
	// Acceleration
	float ax = 0;
	float ay = 0;
	float az = 0;
	float r = currpoints[i]._size / 2;

	bool check_x = false;
	bool check_y = false;
	bool check_z = false;
	float rfx = 1.0; // border reflect check
	float rfy = 1.0;
	float rfz = 1.0;

	for (int j = 0; j < POINT_CNT; ++j) {
		if (i == j) continue;
		if (currpoints[j]._mass == 0.0f) continue;

		float rx;
		float ry;
		float rz;
		float lenpow2;
		float len;

		float dis = r + currpoints[j]._size / 2;

		rx = currpoints[j]._x - currpoints[i]._x;
		ry = currpoints[j]._y - currpoints[i]._y;
		rz = currpoints[j]._z - currpoints[i]._z;

		lenpow2 = rx * rx + ry * ry + rz * rz + epi;
		len = sqrt(lenpow2);

		// gravity
		ax += currpoints[j]._mass * rx / len / lenpow2;
		ay += currpoints[j]._mass * ry / len / lenpow2;
		az += currpoints[j]._mass * rz / len / lenpow2;

		// check collapse
		if (len <= dis) {
			// collapse: 0, be eaten: 1, eat: 2
			int diff_char = currpoints[i]._character - currpoints[j]._character;
			if (diff_char < 0) diff_char += 3;

			// collapse
			if (diff_char == 0) {
				float sum_m = currpoints[i]._mass + currpoints[j]._mass;
				float diff_m = currpoints[i]._mass - currpoints[j]._mass;
				// Update speed
				newpoint->_sx = (diff_m * currpoints[i]._sx + 2 * currpoints[j]._mass * currpoints[j]._sx) / sum_m;
				newpoint->_sy = (diff_m * currpoints[i]._sy + 2 * currpoints[j]._mass * currpoints[j]._sy) / sum_m;
				newpoint->_sz = (diff_m * currpoints[i]._sz + 2 * currpoints[j]._mass * currpoints[j]._sz) / sum_m;
			}
			// be eaten
			else if (diff_char == 1) {
				newpoint->_mass = 0;
				newpoint->_size = 0;
			}
			// eat
			else if (diff_char == 2) {
				newpoint->_mass += currpoints[j]._mass;
				// M1/s1 = M2/s2, s2 = M2s1/M1
				newpoint->_size = newpoint->_mass * currpoints[i]._size / currpoints[i]._mass;

				newpoint->_sx = (currpoints[i]._sx * currpoints[i]._mass + currpoints[j]._sx * currpoints[j]._mass) / newpoint->_mass;
				newpoint->_sy = (currpoints[i]._sy * currpoints[i]._mass + currpoints[j]._sy * currpoints[j]._mass) / newpoint->_mass;
				newpoint->_sz = (currpoints[i]._sz * currpoints[i]._mass + currpoints[j]._sz * currpoints[j]._mass) / newpoint->_mass;
			}
		}
	}

	ax *= G;
	ay *= G;
	az *= G;
	
	// check border
	if(currpoints[i]._x >= POINT_XYZ_MAX) {
		rfx = -1.0;
		check_x = true;
	}
	else if (currpoints[i]._x <= -1 * POINT_XYZ_MAX){
		rfx = 1.0;
		check_x = true;
	}

	if(currpoints[i]._y >= POINT_XYZ_MAX) {
		rfy = -1.0;
		check_y = true;
	}
	else if (currpoints[i]._y <= -1 * POINT_XYZ_MAX){
		rfy = 1.0;
		check_y = true;
	}

	if(currpoints[i]._z >= POINT_XYZ_MAX) {
		rfz = -1.0;
		check_z = true;
	}
	else if (currpoints[i]._z <= -1 * POINT_XYZ_MAX){
		rfz = 1.0;
		check_z = true;
	}

	// Update speed
	newpoint->_sx += ax * dt;
	newpoint->_sy += ay * dt;
	newpoint->_sz += az * dt;

	if (check_x) newpoint->_sx = abs(newpoint->_sx) * rfx; 
	if (check_y) newpoint->_sy = abs(newpoint->_sy) * rfy; 
	if (check_z) newpoint->_sz = abs(newpoint->_sz) * rfz; 

	// Update position
	newpoint->_x = currpoints[i]._x + newpoint->_sx * dt;
	newpoint->_y = currpoints[i]._y + newpoint->_sy * dt;
	newpoint->_z = currpoints[i]._z + newpoint->_sz * dt;
}

void nBodyCalculateMPI(const point *currpoints, point *newpoints, double dt, int start_i, int end_i)
{	
	for (int i = start_i; i < end_i; ++i) {
		newpoints[i - start_i] = currpoints[i];
		if (currpoints[i]._mass == 0.0f) {
			continue;
		}
		_nBodyCalculate(currpoints, &newpoints[i - start_i], i, dt);
	}
}
