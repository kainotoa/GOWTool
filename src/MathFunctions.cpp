#include "pch.h"
#include "MathFunctions.h"

uint32_t BitHacks::RoundUpTo2(uint32_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

Vec3::Vec3(const float& x, const float& y, const float& z)
{
	X = x, Y = y, Z = z;
}
float Vec3::magnitude(void)
{
	return (sqrt(X * X + Y * Y + Z * Z));
}
void Vec3::normalize(void)
{
	float mag = magnitude();
	X = X / mag, Y = Y / mag, Z = Z / mag;
}
Vec2::Vec2(const float& x, const float& y)
{
	X = x, Y = y;
}
Vec4::Vec4()
{
	X = 0, Y = 0, Z = 0, W = 0;
}
Vec4::Vec4(const float& x, const float& y, const float& z, const float& w)
{
	X = x, Y = y, Z = z, W = w;
}
float& Vec4::operator[](size_t idx)
{
	if (idx > 3)
		throw std::out_of_range("idx was outside the bounds of the array, should be <= 3");
	return XYZW[idx];
}
float Vec4::magnitude(void)
{
	return (sqrt(X * X + Y * Y + Z * Z));
}
void Vec4::normalize(void)
{
	float mag = magnitude();
	X = X / mag, Y = Y / mag, Z = Z / mag;
}
Matrix4x4::Matrix4x4()
{
	rows[0] = Vec4();
	rows[0][0] = 1.f;
	rows[1] = Vec4();
	rows[1][1] = 1.f;
	rows[2] = Vec4();
	rows[2][2] = 1.f;
	rows[3] = Vec4();
	rows[3][3] = 1.f;
}
Vec4& Matrix4x4::operator[](size_t idx) {
	if (idx > 3)
		throw std::out_of_range("idx was outside the bounds of the array, should be <= 3");
	return rows[idx];
}
void Matrix4x4::operator =(Matrix4x4& RHS)
{
	for (size_t r = 0; r < 4; r++)
	{
		for (size_t c = 0; c < 4; c++)
		{
			rows[r][c] = RHS[r][c];
		}
	}
}
Matrix4x4 Matrix4x4::operator *(Matrix4x4& RHS)
{
	Matrix4x4 Result;
	for (size_t r = 0; r < 4; r++)
	{
		for (size_t c = 0; c < 4; c++)
		{
			for (size_t t = 0; t < 4; t++)
			{
				Result[r][c] += rows[r][t] * RHS[t][c];
			}
		}
	}
	return Result;
}
Matrix4x4 Matrix4x4::operator +(Matrix4x4& RHS)
{
	Matrix4x4 Result;
	for (size_t r = 0; r < 4; r++)
	{
		for (size_t c = 0; c < 4; c++)
		{
			Result[r][c] = rows[r][c] + RHS[r][c];
		}
	}
	return Result;
}
Matrix4x4 Matrix4x4::operator -(Matrix4x4& RHS)
{
	Matrix4x4 Result;
	for (size_t r = 0; r < 4; r++)
	{
		for (size_t c = 0; c < 4; c++)
		{
			Result[r][c] = rows[r][c] - RHS[r][c];
		}
	}
	return Result;
}
void Matrix4x4::Transpose(void)
{
	Matrix4x4 Result;
	for (size_t r = 0; r < 4; r++)
	{
		for (size_t c = 0; c < 4; c++)
		{
			Result[r][c] = rows[c][r];
		}
	}
	for (size_t r = 0; r < 4; r++)
	{
		for (size_t c = 0; c < 4; c++)
		{
			rows[r][c] = Result[r][c];
		}
	}
}
void Matrix4x4::Display(void)
{
	for (size_t r = 0; r < 4; r++)
	{
		for (size_t c = 0; c < 4; c++)
		{
			cout << rows[r][c] << " ";
		}
		cout << "\n";
	}
}
Quat::Quat(const float& x, const float& y, const float& z, const float& w)
{
	X = x, Y = y, Z = z, W = w;
}
float Quat::getreal(void)
{
	float m = (X * X + Y * Y + Z * Z);
	if (m > 1.0)
		return (sqrt(m - 1.f));
	else
		return (sqrt(1.f - m));
}
#pragma warning(disable:4244)
Vec4 TenBitShifted(const uint32_t& U32)
{
	float X = (U32 & 1023);
	float Y = ((U32 >> 10) & 1023);
	float Z = ((U32 >> 20) & 1023);
	float W = (U32 >> 30);

	Vec4 v(((X - 511) / 512.f), ((Y - 511) / 512.f), ((Z - 511) / 512.f), W / 3.f);
	return v;
}
Vec4 TenBitUnsigned(const uint32_t& U32)
{
	float X = (U32 & 1023);
	float Y = ((U32 >> 10) & 1023);
	float Z = ((U32 >> 20) & 1023);
	float W = (U32 >> 30);

	Vec4 v((X / 1023.f), (Y / 1023.f), (Z / 1023.f), W / 3.f);
	return v;
}
#pragma warning(default:4244)