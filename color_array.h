#pragma once
#include "vec3.h"
#include <fstream>
#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

class ColorArray
{
public:
	int width;
	int height;
private:
	std::unique_ptr<Vec3[]> arr;
public:

	ColorArray() : width(0), height(0) {}

	ColorArray(int width, int height) :width(width), height(height)
	{
		arr = std::make_unique<Vec3[]>(width*height);
	}

	///frees the previous array and creates a new one with the specified width and height
	void reset(int width, int height)
	{
		arr.release();
		this->width = width;
		this->height = height;
		arr = std::make_unique<Vec3[]>(width*height);
	}

	///frees the old array and returns width and height to 0
	void free()
	{
		width = 0;
		height = 0;
		arr.release();
	}

	const Vec3& operator()(int i, int j) const
	{
		return arr[i + j*width];
	}

	const Vec3& operator()(int i) const
	{
		return arr[i];
	}

	Vec3& operator()(int i, int j)
	{
		return arr[i + j*width];
	}

	Vec3& operator()(int i)
	{
		return arr[i];
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
				file << static_cast<int>(255 * operator()(i, j).x) << " " << static_cast<int>(255 * operator()(i, j).y) << " " << static_cast<int>(255 * operator()(i, j).z) << "\n";
			}
		}

		file.close();
	}

	bool loadImageFromFile(const char* filename)
	{
		
		int imageWidth, imageHeight, numberOfComponents;

		unsigned char *imageData = nullptr;
		imageData = stbi_load(filename, &imageWidth, &imageHeight, &numberOfComponents, 3);
		if (imageData == nullptr)
			return false;
		reset(imageWidth, imageHeight);

		for (int j = 0; j < height; ++j)
		{
			for (int i = 0; i < width; ++i)
			{
				(*this)(i, j).set(static_cast<float>(imageData[3 * (i + j*imageWidth)]) / 255.0f, static_cast<float>(imageData[3 * (i + j*imageWidth)+1]) / 255.0f, static_cast<float>(imageData[3 * (i + j*imageWidth)+2]) / 255.0f);
			}
		}
		stbi_image_free(imageData);
		return true;
	}
};