#pragma once
#include "ray.h"
#include "intersectable.h"
#include <vector>
#include <memory>

class Scene
{
public:
	std::vector<std::unique_ptr<Intersectable>> objects;
	bool intersect(const Ray& r, IntersectionInfo& info) const
	{
		for (auto& o : objects)
		{
			if (o->intersect(r, info))
				return true;
		}
		return false;
	}
};