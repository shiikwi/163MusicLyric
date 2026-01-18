#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include <format>

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
			SetConsoleOutputCP(CP_UTF8);
			FILE* f;
			freopen_s(&f, "CONOUT$", "w", stdout);
			freopen_s(&f, "CONOUT$", "w", stderr);
			SetConsoleTitleW(L"TaskbarLyr Debug Console");
		}
	};

	static std::wstring UTF82WString(const std::string& str) {
		if (str.empty()) return std::wstring();
		int size = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
		std::wstring result(size, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &result[0], size);
		return result;
	}
}
