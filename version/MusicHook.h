#pragma once
#include "Lyric.h"
namespace MusicPlugin
{
	typedef __int64(__fastcall* tOnLoad)(void* a1, void* a2);
	static inline tOnLoad TrueAudioplayer_onLoad = nullptr;

	inline decltype(&LoadLibraryExW) TrueLoadLibraryExW = (decltype(TrueLoadLibraryExW))GetProcAddress(GetModuleHandleW(L"kernelbase.dll"), "LoadLibraryExW");

	struct SSOAnalyzer
	{
		union
		{
			char buf[16];
			char* ptr;
		} data;
		size_t size;
		size_t capacity;
		std::string GetString()
		{
			if (capacity >= 16) return std::string(data.ptr, size);
			return std::string(data.buf, size);
		}
	};

	class MusicHook
	{
	public:
		static MusicHook& Instance()
		{
			static MusicHook instance;
			return instance;
		}

		void Initialize();
		void Free();

	private:
		MusicHook(const MusicHook&) = delete;
		MusicHook& operator=(const MusicHook&) = delete;
		MusicHook() = default;
		~MusicHook() = default;

		static void StartLyricHook(HMODULE hCloudMusic);
		static __int64 __fastcall HookedAudioplayer_onLoad(void* a1, void* a2);
		static HMODULE WINAPI HookedLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
	};


	class Scanner
	{
	public:
		static uintptr_t FindFunction(const wchar_t* dllName, const wchar_t* targetStr);
	private:
		static uintptr_t ScanPattern(uintptr_t base, uintptr_t size, BYTE* pattern, size_t patternSize);
	};
}