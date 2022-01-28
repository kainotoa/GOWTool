#pragma once
#include "pch.h"

size_t ConvertGnfToDDS(const byte* gnfsrc, const size_t& gnfsize, byte*& ddsout);
size_t ConvertDDSToGnf(const byte* ddssrc, const size_t& ddssize, byte*& gnfout);