#pragma once
#include <cmath>

class BitHacks
{
public:
	static uint32_t RoundUpTo2(uint32_t v);
};


struct Vec3
{
	float X { 0 }, Y { 0 }, Z { 0 };
	Vec3() = default;
	Vec3(const float& x, const float& y, const float& z);
	float magnitude(void);
	void normalize(void);
};
struct Vec2
{
	float X { 0 }, Y { 0 };
	Vec2() = default;
	Vec2(const float& x, const float& y);
};
struct Vec4
{
	union
	{
		float XYZW[4];
		struct
		{
			float X, Y, Z, W;
		};
	};
	Vec4();
	Vec4(const float& x, const float& y, const float& z, const float& w);
	float& operator[](size_t idx);
	float magnitude(void);
	void normalize(void);
};
struct Matrix4x4 {
	union {
		Vec4 rows[4];
		struct {
			float	m11, m12, m13, m14;
			float 	m21, m22, m23, m24;
			float	m31, m32, m33, m34;
			float	m41, m42, m43, m44;
		};
	};
	// Init Identity Matrix
	Matrix4x4();
	Vec4& operator[](size_t idx);
	void operator=(Matrix4x4& RHS);
	Matrix4x4 operator *(Matrix4x4& RHS);
	Matrix4x4 operator+(Matrix4x4& RHS);
	Matrix4x4 operator-(Matrix4x4& RHS);
	void Transpose(void);
	void Display(void);
};
// for quaternion q = cos(theta/2) + sin(theta/2)*(x.i + y.j + z.k)
// where x.i + y.j + z.k is a normalized vector(or unit vector i.e. mag = 1) as the rotation axis
// theta = angle of rotation also q = W + X.i + Y.j + Z.k
// hence w = cos(theta/2) and X.i + Y.j + Z.k = sin(theta/2)*(x.i + y.j + z.k)
struct Quat 
{
	float X { 0 }, Y { 0 }, Z { 0 }, W { 1 };
	Quat() = default;
	Quat(const float& x, const float& y, const float& z, const float& w);
	float getreal(void);
};
Vec4 R10G10B10A2_SNORM_TO_VEC4(const uint32_t& U32);
Vec4 R10G10B10A2_UNORM_TO_VEC4(const uint32_t& U32);
uint32_t VEC4_TO_R10G10B10A2_SNORM(const Vec4& vec);
uint32_t VEC4_TO_R10G10B10A2_UNORM(const Vec4& vec);
Vec4 R10G11B11_UNORM_TO_VEC4(const uint32_t& U32);