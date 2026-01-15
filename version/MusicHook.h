#pragma once
#include "Lyric.h"
#include <Psapi.h>
#include <TlHelp32.h>
#include <thread>
#include <atomic>
#include <array>
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

	class HardwareHook
	{
	public:
		static HardwareHook& Instance()
		{
			static HardwareHook instance;
			return instance;
		}

		bool Install(uintptr_t address);
		void Uninstall();

	private:
		HardwareHook(const HardwareHook&) = delete;
		HardwareHook& operator=(const HardwareHook&) = delete;
		HardwareHook() = default;
		~HardwareHook() { Uninstall(); }

		uintptr_t m_targetAddress = 0;
		PVOID m_vehHandle = nullptr;

		static LONG CALLBACK VEH(PEXCEPTION_POINTERS pExcptInfo);
		bool SetHBP(HANDLE hThread, bool active);
		void ApplyThreads(bool active);
	};

	class  TickMonitor
	{
	public:
		static TickMonitor& Instance()
		{
			static TickMonitor instance;
			return instance;
		}

		bool Initialize(uintptr_t POnPlayProgress);
		void Stop();

		double GetCurrentTick() const { return m_pCurrentTick ? *m_pCurrentTick : 0.0; }
		double GetTotalTick() const { return m_pTotalTick ? *m_pTotalTick : 0.0; }

	private:
		TickMonitor(const TickMonitor&) = delete;
		TickMonitor& operator=(const TickMonitor&) = delete;
		TickMonitor() = default;
		~TickMonitor() = default;

		double* m_pCurrentTick = nullptr;
		double* m_pTotalTick = nullptr;
		std::atomic<bool> m_running{ false };
		std::thread m_worker;

		void WorkerLoop();
	};

	class Scanner
	{
	public:
		static uintptr_t FindFunction(const wchar_t* dllName, const wchar_t* targetStr);
		static uintptr_t ScanPattern(uintptr_t base, uintptr_t size, BYTE* pattern, size_t patternSize);
		static uintptr_t ResolveRip(uintptr_t address, uintptr_t offsetIdx, uint32_t instSize);
	};
}