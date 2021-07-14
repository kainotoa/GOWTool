#pragma once
#include "pch.h"
#include "MathFunctions.h"
class Rig
{
public:
	uint16_t boneCount{0};
	int16_t* boneParents;
	string* boneNames;
	Matrix4x4* matrix;
	Matrix4x4* IBMs;
public:
	Rig(string filenam);
	Rig() = default;
};