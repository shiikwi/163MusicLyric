#include "MusicHook.h"
#include <Psapi.h>
#include <TlHelp32.h>
#include <thread>
#include "./MinHook/MinHook.h"

namespace MusicPlugin
{
	bool IsMainProcess()
	{
		wchar_t* cmdLine = GetCommandLineW();
		if (wcsstr(cmdLine, L"--type="))
		{
			return false;
		}
		return true;
	}

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
			//if (MH_CreateHook((LPVOID)pOnLoad, &HookedAudioplayer_onLoad, (LPVOID*)&TrueAudioplayer_onLoad) == MH_OK)
			//{
			//	MH_EnableHook((LPVOID)pOnLoad);
			//	Utils::Logger::Log("Hook audioplayer.onLoad Success");
			//}
			HardwareHook::Instance().Install(pOnLoad);
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

	LONG HardwareHook::VEH(PEXCEPTION_POINTERS pExcptInfo)
	{
		if (pExcptInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP)
		{
			if (pExcptInfo->ExceptionRecord->ExceptionAddress == (PVOID)Instance().m_targetAddress)
			{
				void* a1 = (void*)pExcptInfo->ContextRecord->Rcx;
				if (a1)
				{
					try
					{
						SSOAnalyzer* sa = (SSOAnalyzer*)a1;
						auto rawId = sa->GetString();
						LyricProc::Lyric::Instance().UpdateCurrentSong(rawId);
					}
					catch (...) {}
				}

				pExcptInfo->ContextRecord->EFlags |= 0x10000;
				return EXCEPTION_CONTINUE_EXECUTION;
			}
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}

	bool HardwareHook::SetHBP(HANDLE hThread, bool active)
	{
		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		if (!GetThreadContext(hThread, &ctx)) return false;
		if (active)
		{
			ctx.Dr0 = m_targetAddress;
			ctx.Dr7 = (1 << 0) | (1 << 8);
		}
		else
		{
			ctx.Dr0 = 0;
			ctx.Dr7 &= ~(1 << 0);
		}

		return SetThreadContext(hThread, &ctx);
	}

	void HardwareHook::ApplyThreads(bool active)
	{
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (hSnapshot != INVALID_HANDLE_VALUE)
		{
			THREADENTRY32 te;
			te.dwSize = sizeof(te);
			if (Thread32First(hSnapshot, &te))
			{
				do
				{
					if (te.th32OwnerProcessID == GetCurrentProcessId())
					{
						HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
						if (hThread)
						{
							SetHBP(hThread, active);
							CloseHandle(hThread);
						}
					}
				} while (Thread32Next(hSnapshot, &te));
			}
			CloseHandle(hSnapshot);
		}
	}

	bool HardwareHook::Install(uintptr_t address)
	{
		m_targetAddress = address;
		m_vehHandle = AddVectoredExceptionHandler(1, VEH);
		ApplyThreads(true);
		return true;
	}

	void HardwareHook::Uninstall()
	{
		ApplyThreads(false);
		if (m_vehHandle)
		{
			RemoveVectoredExceptionHandler(m_vehHandle);
			m_vehHandle = nullptr;
		}
	}

	void MusicHook::Initialize()
	{
		//if (MH_Initialize() != MH_OK) return;
		//HMODULE hKernelbase = GetModuleHandleW(L"kernelbase.dll");
		//if(!hKernelbase) Utils::Logger::Log("GetModuleHandleW kernelbase.dll failed");
		//MH_CreateHook(&LoadLibraryExW, &HookedLoadLibraryExW, (LPVOID*)&TrueLoadLibraryExW);
		//MH_EnableHook(MH_ALL_HOOKS);
		if (!IsMainProcess()) return;
		Utils::Logger::Log("Start Hook");

		HMODULE hCloudMusicAlr = GetModuleHandleW(L"cloudmusic.dll");
		if (hCloudMusicAlr)
			StartLyricHook(hCloudMusicAlr);
	}

	void MusicHook::Free()
	{
		//MH_DisableHook(MH_ALL_HOOKS);
		//MH_Uninitialize();
	}

}