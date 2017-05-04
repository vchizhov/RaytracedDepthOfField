#pragma once
#include "vec3.h"
#include "mat3.h"
#include "ray.h"
#include "util.h"
#include <algorithm>
#include <iostream>

#define random() uniformDistribution(randomEngine)

class Camera
{
private:
	///rotation matrix - local camera space->world space
	Mat3 rotation;
	///euler angles
	Vec3 pitchYawRoll;
	
	///the radius of the 2nd aperture
	float aperture2Radius;
	///the 2nd aperture's squared radius - it's squared to save a sqrtf in the vignetting function
	float aperture2RadiusSq;

	///image for the bokeh
	ColorArray bokehImage;
	///rows pdf and cdf
	float* bokehImageRowPDF;
	float* bokehImageRowCDF;
		
	///check whether a ray starting from the 1st aperture hits the 2nd one
	bool empiricalOpticalVignetting(const Vec3& rayOrigin, const Vec3& rayDirection) const
	{
		///the second aperture is behind the first aperture - that's where the minus comes from
		Vec3 aperture2Intersection = rayOrigin - aperture2Distance / rayDirection.z*rayDirection;
		return aperture2Intersection.x*aperture2Intersection.x + aperture2Intersection.y*aperture2Intersection.y < aperture2RadiusSq;
	}

	//Unneeded - std::upper_bound will work fine even with a non strictly monotne cdf:
	///places all values==value at the end
	/*void placeValueAtEnd(float* valueArray, int* indexArray, int count, float value, int& outEnd)
	{
		int end = count-1;
		for (int i = 0; i < end; ++i)
		{
			while ( valueArray[indexArray[i]] == value)
			{
				std::swap(indexArray[i], indexArray[end]);
				--end;
				if (end == i)
				{
					if (indexArray[end] == value)
					{
						outEnd = end;
					}
					else
					{
						outEnd = end+1;
					}
					return;
				}
			}
		}
		outEnd = end+1;
	}*/

	///my own implementation of upper bound - returns the index of the first element that has a value greater than value, 
	///if the value is bigger than all elements - returns the last
	int myUpperBound(const float* arr, int count, float value) const
	{
		int left = 0;
		int right = count;
		int mid;
		while (right - left>2)
		{
			mid = (left + right) / 2;
			if (arr[mid] < value)
			{
				left = mid;
			}
			else if (arr[mid] >= value)
			{
				right = mid + 1;
			}
		}
		if (right - left == 2)
		{
			if (arr[left] >= value)
				return left;

			return left + 1;
		}
		else
		{
			return left;
		}
	}

	//a slightly modified version to work with the green component
	int myUpperBound(const Vec3* arr, int count, float value) const
	{
		int left = 0;
		int right = count;
		int mid;
		while (right - left>2)
		{
			mid = (left + right) / 2;
			if (arr[mid].y < value)
			{
				left = mid;
			}
			else if (arr[mid].y >= value)
			{
				right = mid + 1;
			}
		}
		if (right - left == 2)
		{
			if (arr[left].y >= value)
				return left;

			return left + 1;
		}
		else
		{
			return left;
		}
	}

	bool buildBokehImageProbability()
	{
		///for every pixel calculate the luminance with the following formula: pixelLuminance = 0.3*pixel.r + 0.59*pixel.g + 0.11*pixel.b
		Vec3 magicalLuminanceCoefficients(0.3f, 0.59f, 0.11f);
		float totalLuminance = 0.0f;
		for (int i = 0; i < bokehImage.width*bokehImage.height; ++i)
		{
			///store the luminance in the red value (we won't need the image beyond this anyways)
			bokehImage(i).x = dotProduct(magicalLuminanceCoefficients, bokehImage(i));
			///accumulate the luminance
			totalLuminance += bokehImage(i).x;
		}
		///normalize the luminance value of each pixel, so that it is effectively the probability for that pixel being picked - aka the pdf
		totalLuminance = 1.0f / totalLuminance;
		for (int i = 0; i < bokehImage.width*bokehImage.height; ++i)
		{
			///note: totalLuminance is the inverse total luminance here so this is actually pixelLuminance/totalLuminance
			bokehImage(i).x *= totalLuminance;
		}

		///calculate the probability for each row (marginal pdf)
		bokehImageRowPDF = new float[bokehImage.height];

		for (int j = 0; j < bokehImage.height; ++j)
		{
			bokehImageRowPDF[j] = 0.0f;
			for (int i = 0; i < bokehImage.width; ++i)
			{
				bokehImageRowPDF[j] += bokehImage(i, j).x;
			}
		}

		///calculate the rows cdf and record it in the rows probability
		//precalculate the first
		bokehImageRowCDF = new float[bokehImage.height];
		bokehImageRowCDF[0] = bokehImageRowPDF[0];
		for (int j = 1; j < bokehImage.height; ++j)
		{
			bokehImageRowCDF[j] = bokehImageRowCDF[j-1] + bokehImageRowPDF[j];
		}

		///if every row has probability 0 - we have a useless image - abort
		if (bokehImageRowCDF[bokehImage.height-1] == 0.0f)
		{
			return false;
		}		

		///for each pixel in each row - normalize it, so that its pdf is relative to the row and calculate the cdf
		for (int j = 0; j < bokehImage.height; ++j)
		{
			///no use claculating the row pixels' cdf relative to the row, if the row is never going to get picked anyways
			if (bokehImageRowPDF[j] != 0.0f)
			{
				//precalculate the first
				bokehImage(0, j).y = bokehImage(0, j).x / bokehImageRowPDF[j];
				for (int i = 1; i < bokehImage.width; ++i)
				{
					///record the pixel's cdf relative to the row in the green component
					bokehImage(i, j).y = bokehImage(i-1, j).y + bokehImage(i, j).x / bokehImageRowPDF[j];
				}
			}
			

		}

		return true;
	}

	///(x,y) in [-1,1]^2
	void sampleBokehImage(float r1, float r2, float r3, float r4, float& x, float& y) const
	{

		int row = myUpperBound(bokehImageRowCDF, bokehImage.height, r1);
		y = 2.0 - 2.0*(r3 + row) / float(bokehImage.height - 1);
		int column = myUpperBound(&bokehImage(0, row), bokehImage.width, r2);
		x = 2.0* (r4+column)/ float(bokehImage.width - 1) - 1.0;
	}

public:
	///the camera's position (the center of the first aperture)
	Vec3 position;

	///aspectRatio = width/height, filmWidth = aspectRatio, filmHeight = 1.0f
	float aspectRatio;
	///the distance between the center of the 1st aperture and the center of the film
	float focalDistance;
	///radius of the 1st aperture - the bigger the radius, the stronger the dof effect
	float apertureRadius;


	bool enableVignetting;
	///the distance between the 1st and 2nd aperture
	float aperture2Distance;
	///the number of tries before a ray gets vignetted out
	int maxTriesVignetting;


	bool enableImageBasedBokeh;
	
	

	Camera(float aspectRatio = 1.0f, float focalDistance = 1.0f, float apertureRadius = 1.0f, float aperture2Radius= 1.0f, float aperture2Distance=0.3f, 
		bool enableVignetting = false, int maxTriesVignetting = 10, bool enableImageBasedBokeh = false)
		: aspectRatio(aspectRatio), focalDistance(focalDistance), apertureRadius(apertureRadius), aperture2Radius(aperture2Radius), aperture2RadiusSq(aperture2Radius*aperture2Radius), aperture2Distance(aperture2Distance), 
		enableVignetting(enableVignetting), maxTriesVignetting(maxTriesVignetting), enableImageBasedBokeh(enableImageBasedBokeh)
	{

	}

	bool loadBokehImage(const char* filename)
	{	
		if (!bokehImage.loadImageFromFile(filename))
			return false;
		delete[] bokehImageRowCDF;
		delete[] bokehImageRowPDF;
		if (!buildBokehImageProbability())
			return false;

		return true;
	}

	void rotate(float pitch, float yaw, float roll)
	{
		pitchYawRoll.set(pitch, yaw, roll);
		rotation = Mat3::transpose(Mat3::rotationMatrixXYZ(pitchYawRoll));
	}

	///actually Tait-Brayn since it's XYZ
	const Vec3& getEulerAngles() const
	{
		return pitchYawRoll;
	}

	void setAperture2Radius(float radius)
	{
		aperture2Radius = radius;
		aperture2RadiusSq = radius*radius;
	}

	float getAperture2Radius() const
	{
		return aperture2Radius;
	}

	///generates a ray from the film given screen coordinates(x,y) in [0,1]^2
	bool generateRay(std::mt19937& randomEngine, std::uniform_real_distribution<float>& uniformDistribution, float x, float y, Ray& ray) const
	{
		///everything below is in local camera space:

		///the point on the sensor
		Vec3 focusPoint((x - 0.5f)*aspectRatio, (y - 0.5f), focalDistance);

		///random point on the 1st aperture
		Vec3 thinLensPoint;

		Vec3 rayDirection;

		if (enableVignetting)
		{
			//was supposed to be an optimization - didn't work out well:
			/*///(1-lambda)*aperture2Center+lambda*focusPoint = aperture2Center projected on the plane z = 0.0f
			float lambdaProjectAperture2 = aperture2Distance / (aperture2Distance + focusPoint.z);
			///the projected center and radius of the 2nd aperture onto the plane of the first
			Vec3 aperture2ProjectedCenter(focusPoint.x*lambdaProjectAperture2, focusPoint.y*lambdaProjectAperture2, 0.0f);
			float aperture2ProjectedRadius = (1.0f - lambdaProjectAperture2)*aperture2Radius;

			///find the intersection rectangle
			///first values are the coordinates of the bottom left corner of the projection of the 2nd vaperture
			float xmin = std::max(aperture2ProjectedCenter.x - aperture2ProjectedRadius, -apertureRadius);
			float xmax = std::min(aperture2ProjectedCenter.x + aperture2ProjectedRadius, apertureRadius);
			if (xmin > xmax)
				return false;
			float ymin = std::max(aperture2ProjectedCenter.y - aperture2ProjectedRadius, -apertureRadius);
			float ymax = std::min(aperture2ProjectedCenter.y + aperture2ProjectedRadius, apertureRadius);
			if (ymin > ymax)
				return false;
			float dx = xmax - xmin;
			float dy = ymax - ymin;
			float tempRadiusSquared;

			int tries = -1;
			while (++tries, tries < maxTriesVignetting)
			{
				///if the ray missed the 2nd aperture - generate a new ray
				thinLensPoint.x = xmin + dx*random();
				thinLensPoint.y = ymin + dy*random();

				///does the ray not hit the first aperture?
				tempRadiusSquared = thinLensPoint.x*thinLensPoint.x + thinLensPoint.y*thinLensPoint.y;
				if (tempRadiusSquared > apertureRadius*apertureRadius)
					continue;

				///does the ray not hit the 2nd aperture?
				tempRadiusSquared = (thinLensPoint.x - aperture2ProjectedCenter.x)*(thinLensPoint.x-aperture2ProjectedCenter.x)
					+ (thinLensPoint.y-aperture2ProjectedCenter.y)*(thinLensPoint.y - aperture2ProjectedCenter.y);
				if (tempRadiusSquared > aperture2RadiusSq)
					continue;

				rayDirection = normalize(focusPoint - thinLensPoint);
				break;
			}
			if (tries >= maxTriesVignetting)
			{
				return false;
			}*/

			///take samples
			enableImageBasedBokeh ? sampleBokehImage(random(), random(), random(), random(), thinLensPoint.x, thinLensPoint.y) : uniformDiskSample(random(), random(), thinLensPoint.x, thinLensPoint.y);
			
			thinLensPoint.x *= apertureRadius;
			thinLensPoint.y *= apertureRadius;

			rayDirection = normalize(focusPoint - thinLensPoint);

			///empirical vignetting
			int tries = 0;
			while (tries < maxTriesVignetting && !empiricalOpticalVignetting(thinLensPoint, rayDirection))
			{
				///the ray missed the 2nd aperture - generate a new ray
				enableImageBasedBokeh ? sampleBokehImage(random(), random(), random(), random(), thinLensPoint.x, thinLensPoint.y) : uniformDiskSample(random(), random(), thinLensPoint.x, thinLensPoint.y);
				thinLensPoint.x *= apertureRadius;
				thinLensPoint.y *= apertureRadius;
				rayDirection = normalize(focusPoint - thinLensPoint);

				++tries;
			}
			if (tries == maxTriesVignetting)
			{
				return false;
			}
		}
		else
		{
			///take samples
			enableImageBasedBokeh ? sampleBokehImage(random(), random(), random(), random(), thinLensPoint.x, thinLensPoint.y) : uniformDiskSample(random(), random(), thinLensPoint.x, thinLensPoint.y);

			thinLensPoint.x *= apertureRadius;
			thinLensPoint.y *= apertureRadius;

			rayDirection = normalize(focusPoint - thinLensPoint);
		}

		///our ray starts from the sensor
		///transform into world space:
		ray.origin = position+rotation*focusPoint;
		ray.direction = rotation*rayDirection;
		return true;

	}
};