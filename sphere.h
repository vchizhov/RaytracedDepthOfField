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
		Vec3 po = r.origin - position;
		float pod = dotProduct(po, r.direction);
		float dd = dotProduct(r.direction, r.direction);
		float rr = dotProduct(po,po) - radius*radius;
		float discriminant = pod*pod - dd*rr;

		if (discriminant < 0.0f)
			return false;

		discriminant = sqrtf(discriminant);

		float t = (-pod + discriminant) / dd;
		if (t > 0.0f)
			return true;

		t = (-pod - discriminant) / dd;
		if (t > 0.0f)
			return true;

		return false;
	}
};