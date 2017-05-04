#pragma once
#include "intersectable.h"

class Sphere : public Intersectable
{
public:
	Vec3 position;
	float radius;

	Sphere(float radius = 1.0f, const Vec3& position = Vec3(0.0f))
		:radius(radius), position(position)
	{

	}


	bool intersect(const Ray& r, IntersectionInfo& info) const
	{
		Vec3 op = position-r.origin;
		float opd = dotProduct(op, r.direction);
		//float dd = dotProduct(r.direction, r.direction); -should equal 1 if r.direction is normalized
		float discriminant = opd*opd + radius*radius - dotProduct(op, op);

		if (discriminant < 0.0f)
			return false;

		discriminant = sqrtf(discriminant);

		float t = opd + discriminant;
		if (t > 0.0f)
			return true;

		t = opd - discriminant;
		if (t > 0.0f)
			return true;

		return false;
	}
};