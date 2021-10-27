#pragma once
#include "pch.h"
#include <Windows.h>

namespace Utils
{
	class FileDialogs
	{
	public:
		static std::string OpenFile(const char* filter = "All Files (*.*)\0*.*\0", const char* title = "Open", HWND owner = NULL);
	};
	class Logger
	{
	public:
		static void Error(const char* msg);
		static void Warning(const char* msg);
		static void Success(const char* msg);
	};
}