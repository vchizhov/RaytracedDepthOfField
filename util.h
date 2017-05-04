#pragma once
#include <random>
#include <math.h>

std::random_device rd;



const double PI = 3.14159265358979323846;
const double PI2 = 2.0 * PI;

///returns a random number in [0,1]
//float random()
//{
//	return uniformDistribution(randomEngine);
//}

/// (r1,r2) from [0,1]^2
void uniformDiskSample(float r1, float r2, float& x, float& y)
{
	float r = sqrtf(r1);
	float phi = PI2*r2;
	x = r*cosf(phi);
	y = r*sinf(phi);
}

