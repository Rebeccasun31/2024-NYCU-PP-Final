#include <math.h>
#include <iostream>

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

	bool check_x = false;
	bool check_y = false;
	bool check_z = false;
	float rfx = 1.0; // border reflect check
	float rfy = 1.0;
	float rfz = 1.0;

	for (int j = 0; j < POINT_CNT; ++j) {
		if (i == j) continue;
		// std::cout<<"\n\n\n\n\n";
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

	// std::cout << ax << ' ' << ay << ' ' << az << '\n';
	
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
	newpoint->_sx = currpoints[i]._sx + ax * dt;
	newpoint->_sy = currpoints[i]._sy + ay * dt;
	newpoint->_sz = currpoints[i]._sz + az * dt;

	if (check_x) newpoint->_sx = abs(newpoint->_sx) * rfx; 
	if (check_y) newpoint->_sy = abs(newpoint->_sy) * rfy; 
	if (check_z) newpoint->_sz = abs(newpoint->_sz) * rfz; 

	// std::cout << "speed z: " << newpoint->_sz << std::endl;
	// Update position
	newpoint->_x = currpoints[i]._x + newpoint->_sx * dt;
	newpoint->_y = currpoints[i]._y + newpoint->_sy * dt;
	newpoint->_z = currpoints[i]._z + newpoint->_sz * dt;
	// std::cout << "newpoint z: " << newpoint->_z << std::endl;
}

static void _nBodyCollapse(const point *currpoints, point *newpoint, int i, double dt) {
	float epi = 0.0000000000001;
	float r = currpoints[i]._size / 2;

	bool collapse = false;

	for (int j = 0; j < POINT_CNT; ++j) {
		if (i == j) continue;

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
		
		if (len <= dis) {
			float sum_m = currpoints[i]._mass + currpoints[j]._mass;
			float diff_m = currpoints[i]._mass - currpoints[j]._mass;
			// Update speed
			newpoint->_sx = (diff_m * currpoints[i]._sx + 2 * currpoints[j]._mass * currpoints[j]._sx) / sum_m;
			newpoint->_sy = (diff_m * currpoints[i]._sy + 2 * currpoints[j]._mass * currpoints[j]._sy) / sum_m;
			newpoint->_sz = (diff_m * currpoints[i]._sz + 2 * currpoints[j]._mass * currpoints[j]._sz) / sum_m;

			// std::cout << i << ": " << "dis: " << dis << " len: " << len << " sz: " << newpoint->_sz << '\n';
			collapse = true;
			break;
		}
	}

	if (!collapse) {
		// Copy speed
		newpoint->_sx = currpoints[i]._sx;
		newpoint->_sy = currpoints[i]._sy;
		newpoint->_sz = currpoints[i]._sz;
	}

	// Update position
	newpoint->_x = currpoints[i]._x + newpoint->_sx * dt;
	newpoint->_y = currpoints[i]._y + newpoint->_sy * dt;
	newpoint->_z = currpoints[i]._z + newpoint->_sz * dt;
}

void nBodyCalculateSerial(const point *currpoints, point *newpoints, double dt)
{
	point vertices_tmp[POINT_CNT];
	point *tmp = vertices_tmp;
	for (int i = 0; i < POINT_CNT; ++i) {
		vertices_tmp[i] = currpoints[i];
	}

	for (int i = 0; i < POINT_CNT; ++i) {
		_nBodyCalculate(currpoints, &vertices_tmp[i], i, dt);
	}
	for (int i = 0; i < POINT_CNT; ++i) {
		_nBodyCollapse(vertices_tmp, &newpoints[i], i, dt);
	}
}
