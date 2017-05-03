#pragma once
#include <algorithm>

struct Vec3
{
	float x, y, z;

	Vec3(float scalar=0.0f) : x(scalar), y(scalar), z(scalar) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	Vec3(const Vec3& other) : x(other.x), y(other.y), z(other.z) {}

	Vec3& operator=(const Vec3& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	void set(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	float length() const
	{
		return sqrtf(x*x + y*y + z*z);
	}

	Vec3& operator+=(const Vec3& other)
	{
		x += other.x; y += other.y; z += other.z;
		return *this;
	}

	Vec3& operator*=(float s)
	{
		x *= s; y *= s; z *= s;
		return *this;
	}
};

Vec3 operator+(const Vec3& v1, const Vec3& v2)
{
	return Vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

Vec3 operator-(const Vec3& v1, const Vec3& v2)
{
	return Vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

Vec3 operator*(float s, const Vec3& v)
{
	return Vec3(s*v.x, s*v.y, s*v.z);
}

Vec3 crossProduct(const Vec3& v1, const Vec3& v2)
{
	return Vec3(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

float dotProduct(const Vec3& v1, const Vec3& v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

Vec3 normalize(const Vec3& v)
{
	return 1 / v.length()*v;
}

float clamp(float x, float a, float b)
{
	return std::min(std::max(x, a), b);
}

Vec3 clamp(const Vec3& v, const Vec3& a, const Vec3& b)
{
	return Vec3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z));
}