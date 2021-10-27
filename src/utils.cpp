#include "pch.h"
#include "utils.h"

namespace Utils
{
	std::string FileDialogs::OpenFile(const char* filter, const char* title, HWND owner)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = owner;
		ofn.lpstrTitle = title;
		ofn.lpstrFilter = filter;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}
	void Logger::Error(const char* msg)
	{
		auto hndl = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hndl, &csbi);
		SetConsoleTextAttribute(hndl, 4);
		cout << msg;
		SetConsoleTextAttribute(hndl, csbi.wAttributes);
	}
	void Logger::Success(const char* msg)
	{
		auto hndl = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hndl, &csbi);
		SetConsoleTextAttribute(hndl, 2);
		cout << msg;
		SetConsoleTextAttribute(hndl, csbi.wAttributes);
	}
	void Logger::Warning(const char* msg)
	{
		auto hndl = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hndl, &csbi);
		SetConsoleTextAttribute(hndl, 14);
		cout << msg;
		SetConsoleTextAttribute(hndl, csbi.wAttributes);
	}
}