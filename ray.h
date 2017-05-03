#pragma once
#include "vec3.h"

struct Ray
{
	Vec3 origin;
	Vec3 direction;

	Ray() {}
	Ray(const Vec3& origin, const Vec3& direction) : origin(origin), direction(direction) {}
	Ray(const Ray& other) : origin(other.origin), direction(other.direction) {}
	
	Ray& operator=(const Ray& other)
	{
		origin = other.origin;
		direction = other.direction;
		return *this;
	}


	Vec3 atPoint(float t) const
	{
		return origin + t*direction;
	}
};