#include "MusicHook.h"
#include <Psapi.h>
#include <thread>
#include "./MinHook/MinHook.h"

namespace MusicPlugin
{
	uintptr_t Scanner::ScanPattern(uintptr_t base, uintptr_t size, BYTE* pattern, size_t patternSize)
	{
		for (uintptr_t i = 0; i < size - patternSize; i++)
		{
			bool found = true;
			for (uintptr_t j = 0; j < patternSize; j++)
			{
				if (*(BYTE*)(base + i + j) != pattern[j])
				{
					found = false;
					break;
				}
			}
			if (found) return (base + i);
		}
		return 0;
	}


	uintptr_t Scanner::FindFunction(const wchar_t* dllName, const wchar_t* targetStr)
	{
		uintptr_t moduleBase = (uintptr_t)GetModuleHandleW(dllName);
		if (!moduleBase) return 0;

		MODULEINFO mi;
		GetModuleInformation(GetCurrentProcess(), (HMODULE)moduleBase, &mi, sizeof(mi));
		uintptr_t moduleSize = (uintptr_t)mi.SizeOfImage;

		size_t strLen = wcslen(targetStr) * 2;
		uintptr_t strAddr = Scanner::ScanPattern(moduleBase, moduleSize, (BYTE*)targetStr, strLen);
		if (!strAddr)
		{
			Utils::Logger::Error("Find {} StrAddress failed", Utils::Logger::WStringToUTF8(targetStr));
			return 0;
		}

		//lea rdx [strAddr]
		uintptr_t leaOpAddr = 0;
		for (uintptr_t i = moduleBase; i < moduleBase + moduleSize;i++)
		{
			if (*(BYTE*)i == 0x48 && *(BYTE*)(i + 1) == 0x8D && *(BYTE*)(i + 2) == 0x15)
			{
				int32_t rva = *(int32_t*)(i + 3);
				uintptr_t target = i + 7 + rva;
				if (target == strAddr)
				{
					leaOpAddr = i;
					break;
				}
			}
		}
		if (!leaOpAddr)
		{
			Utils::Logger::Error("Find LeaOpAddress failed");
			return 0;
		}

		uintptr_t funcHead = 0;
		for (uintptr_t i = leaOpAddr; i > moduleBase; --i)
		{
			if (*(BYTE*)i == 0xCC)
			{
				if (*(BYTE*)(i + 1) != 0xCC)
				{
					funcHead = i + 1;
					break;
				}
			}
		}

		Utils::Logger::Log("{} func found at: 0x{:x}", Utils::Logger::WStringToUTF8(targetStr), funcHead);
		return funcHead;
	}

	__int64 __fastcall MusicHook::HookedAudioplayer_onLoad(void* a1, void* a2)
	{
		if (a1)
		{
			SSOAnalyzer* sa = (SSOAnalyzer*)a1;
			auto rawId = sa->GetString();

			size_t pos = rawId.find('_');
			auto SongId = (pos != std::string::npos) ? rawId.substr(0, pos) : rawId;
			LyricProc::Lyric::Instance().UpdateCurrentSong(SongId);
		}

		return TrueAudioplayer_onLoad(a1, a2);
	}

	void MusicHook::StartLyricHook(HMODULE hCloudMusic)
	{
		Utils::Logger::Log("cloudmusic.dll loaded at: {}", (void*)hCloudMusic);

		auto pOnLoad = Scanner::FindFunction(L"cloudmusic.dll", L"audioplayer.onLoad");
		if (pOnLoad)
		{
			if (MH_CreateHook((LPVOID)pOnLoad, &HookedAudioplayer_onLoad, (LPVOID*)&TrueAudioplayer_onLoad) == MH_OK)
			{
				MH_EnableHook((LPVOID)pOnLoad);
				Utils::Logger::Log("Hook audioplayer.onLoad Success");
			}
		}
	}


	HMODULE WINAPI MusicHook::HookedLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
	{
		Utils::Logger::Log("LoadLibraryExW Hooked");
		HMODULE hMod = TrueLoadLibraryExW(lpLibFileName, hFile, dwFlags);
		if (hMod && lpLibFileName)
		{
			std::wstring libName(lpLibFileName);
			Utils::Logger::Log("LoadLibraryExW: {}", Utils::Logger::WStringToUTF8(libName));
			if (libName.find(L"cloudmusic.dll") != std::wstring::npos)
				StartLyricHook(hMod);
		}
		return hMod;
	}

	void MusicHook::Initialize()
	{
		if (MH_Initialize() != MH_OK) return;
		//HMODULE hKernelbase = GetModuleHandleW(L"kernelbase.dll");
		//if(!hKernebase) Utils::Logger::Log("GetModuleHandleW kernelbase.dll failed");
		//MH_CreateHook(&LoadLibraryExW, &HookedLoadLibraryExW, (LPVOID*)&TrueLoadLibraryExW);
		//MH_EnableHook(MH_ALL_HOOKS);
		Utils::Logger::Log("Start Hook");

		HMODULE hCloudMusicAlr = GetModuleHandleW(L"cloudmusic.dll");
		if (hCloudMusicAlr)
			StartLyricHook(hCloudMusicAlr);
	}

	void MusicHook::Free()
	{
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();
	}

}