#include "pch.h"
#include "utils.h"
#include <iomanip>

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
	CommandLine::CommandLine(const string& title, const string& usage)
	{
		this->_usage = usage;
		this->_title = title;
	}
	void CommandLine::AddCommand(const string& name, const string& helpmsg)
	{
		Command cmmd;
		cmmd.desc = helpmsg;
		_commands[name] = cmmd;
	}
	void CommandLine::AddOption(const string& commandName, const string& optionName, const string& desc, const OptionType& optionType, const vector<string>& valids)
	{
		if (!_commands.contains(commandName))
			return;
		Command::Option opn;
		opn.optionType = optionType;
		opn.desc = desc;
		
		if (opn.optionType != OptionType::Bool)
			opn.valids = valids;

		_commands[commandName]._options[optionName] = opn;
	}

	bool CommandLine::Parse(int argc, char* argv[])
	{
		if (argc < 2)
		{
			Utils::Logger::Error("\nRequired argument was not provided.\n");
			return false;
		}
		int test = 0;
		for (int i = 1; i < argc; i++)
		{
			if (_commands.contains(argv[i]))
			{
				_commands[argv[i]].active = true;
				test++;
			}
			if (test > 1)
			{
				Utils::Logger::Error("\nMultiple Commands are not allowed at once.\n");
				return false;
			}
		}
		if (test == 0)
		{
			Utils::Logger::Error("\nNo Command was provided.\n");
			return false;
		}
		for (auto itr = _commands.begin(); itr != _commands.end(); itr++)
		{
			if (itr->second.active)
			{
				auto &cmmd = itr->second;
				const string& cmmdName = itr->first;
				for (int i = 1; i < argc; i++)
				{
					if (argv[i] == itr->first)
					{
						continue;
					}
					if (cmmd._options.contains(argv[i]))
					{
						auto& opn = cmmd._options[argv[i]];
						string opnName = argv[i];
						opn.active = true;
						if (opn.optionType == OptionType::MultiArgs || opn.optionType == OptionType::SingleArg)
						{
							i++;
							for (; i < argc && itr->first != argv[i]; i++)
							{
								if (cmmd._options.contains(argv[i]))
								{
									i--;
									break;
								}
								if (opn.valids.size() > 0)
								{
									if (std::find(opn.valids.begin(), opn.valids.end(), argv[i]) == opn.valids.end())
									{
										string msg = string("Invalid Arg: ") + argv[i] + " for the provided option: " + opnName + ".\n";
										Utils::Logger::Error(msg.c_str());
										return false;
									}
								}
								opn.args.push_back(argv[i]);
							}
							if (opn.args.size() > 1 && opn.optionType == OptionType::SingleArg)
							{
								string msg = "Multiple Args not allowed for the provided option: " + opnName + ".\n";
								Utils::Logger::Error(msg.c_str());
								return false;
							}
							if (opn.args.size() < 1)
							{
								string msg = "No Args provided for the option: " + opnName + ".\n";
								Utils::Logger::Error(msg.c_str());
								return false;
							}
						}
					}
					else
					{
						string msg = string("\nInvalid Option: ") + argv[i] + " for the provided command: " + cmmdName + ".\n";
						Utils::Logger::Error(msg.c_str());
						return false;
					}
				}
				return true;
			}
		}
		return false;
	}
	void CommandLine::PrintHelp(bool forceGlobal)
	{
		if (!forceGlobal)
		{
			for (auto itr = _commands.begin(); itr != _commands.end(); itr++)
			{
				if (itr->second.active)
				{
					cout << itr->first +"\n";
					cout << _usage + " " + itr->first + " [options]\n";
					cout << "Options:\n";
					for (auto itr1 = itr->second._options.begin(); itr1 != itr->second._options.end(); itr1++)
					{
						cout << "  ";
						cout.width(0x10);
						cout << std::left << itr1->first << itr1->second.desc + "\n";
					}
					return;
				}
			}
		}
		cout << _title + "\n";
		cout << _usage + " [command] [options]\n";
		cout << "Commands:\n";
		for (auto itr = _commands.begin(); itr != _commands.end(); itr++)
		{
			cout << "  ";
			cout.width(0x10);
			cout << std::left << itr->first << itr->second.desc + "\n";
		}
	}
}