#pragma once
#include "ray.h"
#include "intersection_info.h"
class Intersectable
{
public:
	virtual bool intersect(const Ray& r, IntersectionInfo& info) const = 0;
};