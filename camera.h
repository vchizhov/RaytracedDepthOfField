#pragma once
#include "vec3.h"
#include "mat3.h"
#include "ray.h"
#include "util.h"


class Camera
{
private:
	///rotation matrix - local camera space->world space
	Mat3 rotation;
	///euler angles
	Vec3 pitchYawRoll;
	
	///check whether a ray starting from the 1st aperture hits the 2nd one
	bool empiricalOpticalVignetting(const Vec3& rayOrigin, const Vec3& rayDirection) const
	{
		///the second aperture is behind the first aperture - that's where the minus comes from
		Vec3 aperture2Intersection = rayOrigin - aperture2Distance / rayDirection.z*rayDirection;
		return aperture2Intersection.x*aperture2Intersection.x + aperture2Intersection.y*aperture2Intersection.y < aperture2RadiusSq;
	}

public:
	///the camera's position (the center of the first aperture)
	Vec3 position;

	bool enableVignetting;
	///aspectRatio = width/height, filmWidth = aspectRatio, filmHeight = 1.0f
	float aspectRatio;
	///the distance between the center of the 1st aperture and the center of the film
	float focalDistance;
	///radius of the 1st aperture - the bigger the radius, the stronger the dof effect
	float apertureRadius;
	///the 2nd aperture's squared radius - it's squared to save a sqrtf in the vignetting function
	float aperture2RadiusSq;
	///the distance between the 1st and 2nd aperture
	float aperture2Distance;
	///the number of tries before a ray gets vignetted out
	int maxTriesVignetting;

	Camera(float aspectRatio = 1.0f, float focalDistance = 1.0f, float apertureRadius = 1.0f, float aperture2RadiusSq = 1.0f, float aperture2Distance=0.3f, bool enableVignetting = false, int maxTriesVignetting = 10)
		: aspectRatio(aspectRatio), focalDistance(focalDistance), apertureRadius(apertureRadius), aperture2RadiusSq(aperture2RadiusSq), aperture2Distance(aperture2Distance), enableVignetting(enableVignetting), maxTriesVignetting(maxTriesVignetting)
	{

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

	///generates a ray from the film given screen coordinates(x,y) in [0,1]^2
	bool generateRay(float x, float y, Ray& ray) const
	{
		///everything below is in local camera space:

		///the point on the sensor
		Vec3 focusPoint((x - 0.5f)*aspectRatio, (y - 0.5f), focalDistance);

		///random point on the 1st aperture
		Vec3 thinLensPoint;
		uniformDiskSample(random(), random(), thinLensPoint.x, thinLensPoint.y);
		thinLensPoint.x *= apertureRadius;
		thinLensPoint.y *= apertureRadius;

		Vec3 rayDirection = normalize(focusPoint-thinLensPoint);

		if (enableVignetting)
		{
			///empirical vignetting
			int tries = 0;
			while (tries < maxTriesVignetting && !empiricalOpticalVignetting(thinLensPoint, rayDirection))
			{
				///the ray missed the 2nd aperture - generate a new ray
				uniformDiskSample(random(), random(), thinLensPoint.x, thinLensPoint.y);
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

		///our ray starts from the sensor
		///transform into world space:
		ray.origin = position+rotation*focusPoint;
		ray.direction = rotation*rayDirection;
		return true;

	}
};