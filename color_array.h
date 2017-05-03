#pragma once
#include "vec3.h"
#include <fstream>
#include <memory>

class ColorArray
{
public:
	const int width;
	const int height;
private:
	std::unique_ptr<Vec3[]> arr;
public:
	
	ColorArray(int width, int height) :width(width), height(height)
	{
		arr = std::make_unique<Vec3[]>(width*height);
	}

	const Vec3& operator()(int i, int j) const
	{
		return arr[i + j*width];
	}

	Vec3& operator()(int i, int j)
	{
		return arr[i + j*width];
	}

	void saveToPPM(const char* filename) const
	{
		std::ofstream file(filename);
		file << "P3\n";
		file << width << " " << height << "\n";
		file << "255\n";

		for (int j = 0; j < height; ++j)
		{
			for (int i = 0; i < width; ++i)
			{
				file << static_cast<int>(255 * operator()(i,j).x) << " " << static_cast<int>(255 * operator()(i, j).y) << " " << static_cast<int>(255 * operator()(i, j).z) << "\n";
			}
		}

		file.close();
	}
};