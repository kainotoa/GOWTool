#pragma once
#define NOMINMAX
#include "pch.h"
#include <Windows.h>

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <utility>

using std::string;
using std::map;
using std::vector;
using std::pair;

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
	std::string inline str_tolower(std::string s) {
		std::transform(s.begin(), s.end(), s.begin(),
			[](unsigned char c) { return std::tolower(c); }
		);
		return s;
	}
	std::string inline str_toupper(std::string s) {
		std::transform(s.begin(), s.end(), s.begin(),
			[](unsigned char c) { return std::toupper(c); }
		);
		return s;
	}
	class CommandLine
	{
	public:
		enum class OptionType
		{
			Bool = 0,
			SingleArg = 1,
			MultiArgs = 2,
		};

		CommandLine(const string& title, const string& usage);
		void AddCommand(const string& commandName, const string& Desc);
		void AddOption(const string& commandName, const string& optionName,const string& desc, const OptionType& optionType = OptionType::Bool, const vector<string>& valids = { });
		void PrintHelp(bool forceGlobal = false);
		bool Parse(int argc, char* argv[]);

		class Command
		{
		public:

			class Option
			{
			public:
				vector<string> args;
				bool active = false;
				vector<string> valids;
				OptionType optionType;
				string desc;
			};

			bool active = false;
			map<string, Option> _options;

			string desc;
		};

		map<string, Command> _commands;

		string _usage;
		string _title;
	};
}