#pragma once
#include <cmath>
struct Vec3
{
	float X { 0 }, Y { 0 }, Z { 0 };
	Vec3() = default;
	Vec3(const float& x, const float& y, const float& z)
	{
		X = x, Y = y, Z = z;
	}
	float magnitude(void)
	{
		return (sqrt(X * X + Y * Y + Z * Z));
	}
	void normalize(void)
	{
		float mag = magnitude();
		X = X / mag, Y = Y / mag, Z = Z / mag;
	}
};
struct Vec2
{
	float X { 0 }, Y { 0 };
	Vec2() = default;
	Vec2(const float& x, const float& y)
	{
		X = x, Y = y;
	}
};
struct Vec4
{
	float X { 0 }, Y { 0 }, Z { 0 }, W { 0 };
	Vec4() = default;
	Vec4(const float& x, const float& y, const float& z, const float& w)
	{
		X = x, Y = y, Z = z, W = w;
	}
	float magnitude(void)
	{
		return (sqrt(X * X + Y * Y + Z * Z));
	}
	void normalize(void)
	{
		float mag = magnitude();
		X = X / mag, Y = Y / mag, Z = Z / mag;
	}
};

// for quaternion q = cos(theta/2) + sin(theta/2)*(x.i + y.j + z.k)
// where x.i + y.j + z.k is a normalized vector(or unit vector i.e. mag = 1) as the rotation axis
// theta = angle of rotation also q = W + X.i + Y.j + Z.k
// hence w = cos(theta/2) and X.i + Y.j + Z.k = sin(theta/2)*(x.i + y.j + z.k)
struct Quat 
{
	float X { 0 }, Y { 0 }, Z { 0 }, W { 1 };
	Quat() = default;
	Quat(const float& x, const float& y, const float& z, const float& w)
	{
		X = x, Y = y, Z = z, W = w;
	}
	float getreal(void)
	{
		float m = (X * X + Y * Y + Z * Z);
		if(m > 1.0)
			return (sqrt(m - 1.0));
		else
			return (sqrt(1.0 - m));
	}
};
inline Vec4 TenBitShifted(uint32_t U32)
{
	int16_t X = (int16_t)(U32 & 1023);
	int16_t Y = (int16_t)((U32 >> 10) & 1023);
	int16_t Z = (int16_t)((U32 >> 20) & 1023);
	int16_t W = (int16_t)((U32) >> 30);

	Vec4 v(((float)(X - 511) / (float)512), ((float)(Y - 511) / (float)512), ((float)(Z - 511) / (float)512), (float)W / (float)3);
	return v;
}
inline Vec4 TenBitUnsigned(uint32_t U32)
{
	int16_t X = (int16_t)(U32 & 1023);
	int16_t Y = (int16_t)((U32 >> 10) & 1023);
	int16_t Z = (int16_t)((U32 >> 20) & 1023);
	int16_t W = (int16_t)((U32) >> 30);

	Vec4 v(((float)X / (float)1023), ((float)Y / (float)1023), ((float)Z / (float)1023), (float)W / (float)3);
	return v;
}