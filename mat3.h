#pragma once
#include "vec3.h"
struct Mat3
{
public:
	Vec3 row[3];

	Mat3() : row{ Vec3(1.0f,0.0f,0.0f), Vec3(0.0f,1.0f,0.0f), Vec3(0.0f,0.0f,1.0f) } {}
	Mat3(float m11, float m12, float m13,
		float m21, float m22, float m23,
		float m31, float m32, float m33)
		: row{ Vec3(m11, m12, m13), Vec3(m21, m22, m23), Vec3(m31, m32, m33) }
	{

	}

	Mat3& fromColumnVectors(const Vec3& c1, const Vec3& c2, const Vec3& c3)
	{
		row[0].set(c1.x, c2.x, c3.x);
		row[1].set(c1.y, c2.y, c3.y);
		row[2].set(c1.z, c2.z, c3.z);
		return *this;
	}

	static Mat3 rotationMatrixXYZ(const Vec3& pitchYawRoll)
	{
		float c1 = cosf(pitchYawRoll.x);
		float c2 = cosf(pitchYawRoll.y);
		float c3 = cosf(pitchYawRoll.z);

		float s1 = sinf(pitchYawRoll.x);
		float s2 = sinf(pitchYawRoll.y);
		float s3 = sinf(pitchYawRoll.z);
		return Mat3(c2*c3, -c2*s3, s2,
			c1*s3 + c3*s1*s2, c1*c3-s1*s2*s3, -c2*s1,
			s1*s3-c1*c3*s2, c3*s1 + c1*s2*s3, c1*c2);
	}

	static Mat3 transpose(const Mat3& m)
	{
		return Mat3().fromColumnVectors(m.row[0], m.row[1], m.row[2]);
	}
};



Vec3 operator*(const Mat3& m, const Vec3& v)
{
	return Vec3(dotProduct(m.row[0], v), dotProduct(m.row[1], v), dotProduct(m.row[2], v));
}