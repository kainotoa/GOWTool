#include <cstdint>
#include <math.h>

struct Vec3
{
	float X, Y, Z;
	Vec3(void) { X = 0, Y = 0, Z =  0; }
	Vec3(const float x, const float y, const float z)
	{
		X = x, Y = y, Z = z;
	}
	float magnitude(void)
	{
		double a = X, b = Y, c = Z;
		return ((float)sqrt(a * a + b * b + c * c));
	}
	void normalize(void)
	{
		float mag = magnitude();
		X = X / mag, Y = Y / mag, Z = Z / mag;
	}
};
struct Vec2
{
	float X, Y;
	Vec2(void) { X = 0, Y = 0; }
	Vec2(const float x, const float y)
	{
		X = x, Y = y;
	}
};
struct Vec4
{
	float X, Y, Z, W;
	Vec4(void) { X = 0, Y = 0, Z = 0, W = 0; }
	Vec4(const float x, const float y, const float z, const float w)
	{
		X = x, Y = y, Z = z, W = w;
	}
	float magnitude(void)
	{
		double a = X, b = Y, c = Z;
		return ((float)sqrt(a * a + b * b + c * c));
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
	float X, Y, Z, W;
	Quat(void) { X = 0, Y = 0, Z = 0, W = 0; }
	Quat(const float x, const float y, const float z, const float w)
	{
		X = x, Y = y, Z = z, W = w;
	}
	float getreal(void)
	{
		double a = X, b = Y, c = Z;
		double d = (a * a + b * b + c * c);
		if(d > 1.0)
			return ((float)sqrt(d - 1.0));
		else
			return ((float)sqrt(1.0 - d));
	}
};