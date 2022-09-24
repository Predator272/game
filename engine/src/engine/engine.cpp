#include "engine/engine.hpp"
#include <codecvt>
#include <chrono>
#include <fstream>
#include <windows.h>

namespace Engine
{
	std::wstring Utf8ToUnicode(const std::string& String)
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(String);
	}

	std::string UnicodeToUtf8(const std::wstring& String)
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(String);
	}

	double GetTime()
	{
		return std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).time_since_epoch().count();
	}

	int GetSceenWidth()
	{
		return GetSystemMetrics(SM_CXSCREEN);
	}

	int GetSceenHeight()
	{
		return GetSystemMetrics(SM_CYSCREEN);
	}

	std::string GetProgramFileName()
	{
		constexpr size_t StringSize = 512;
		wchar_t String[StringSize] = {};
		GetModuleFileNameW(nullptr, String, StringSize);
		std::string FileName(UnicodeToUtf8(String));
		std::replace(FileName.begin(), FileName.end(), '\\', '/');
		return FileName;
	}

	std::string GetBasePath()
	{
		const std::string BasePath = GetProgramFileName();
		return BasePath.substr(0, BasePath.find_last_of('/'));
	}

	std::string ReadFile(const std::string& FileName)
	{
		std::ifstream File(FileName, std::ios::in | std::ios::binary);
		return { std::istreambuf_iterator<char>(File), std::istreambuf_iterator<char>() };
	}
}
