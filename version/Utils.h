#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include <format>
#include <vector>

namespace Utils
{
	class Logger
	{
	public:
		template<typename... Args>
		static void Log(std::format_string<Args...> fmt, Args&&... args)
		{
			std::cout << std::format("[*TaskbarLyr] {}\n", std::format(fmt, std::forward<Args>(args)...));
		}
		template<typename... Args>
		static void Error(std::format_string<Args...> fmt, Args&&... args)
		{
			std::cerr << std::format("[TaskbarLyr Error] {}\n", std::format(fmt, std::forward<Args>(args)...));
		}
		static std::string WStringToUTF8(const std::wstring& wstr) {
			if (wstr.empty()) return {};
			int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
			std::string result(size - 1, '\0');
			WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, result.data(), size, nullptr, nullptr);
			return result;
		}
	};

	class Console
	{
	public:
		static void AttachConsole()
		{
			::AllocConsole();
			FILE* f;
			freopen_s(&f, "CONOUT$", "w", stdout);
			freopen_s(&f, "CONOUT$", "w", stderr);
			SetConsoleTitleW(L"TaskbarLyr Debug Console");
		}
	};
}
