#include "Utils.h"
#include "MusicHook.h"
#include "Taskbar.h"
#include "./Hijack/version.h";

extern "C" __declspec(dllexport) void Lyrics() {}

bool IsMainProcess()
{
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	std::wstring wsPath(path);
	for (auto& c : wsPath) c = towlower(c);

	bool isMain = (wcsstr(GetCommandLineW(), L"--type=") == nullptr);
	bool isCloudMusic = (wsPath.find(L"cloudmusic.exe") != std::wstring::npos);

	return isMain && isCloudMusic;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hModule);
		if (IsMainProcess)
		{
			Utils::Console::AttachConsole();
			MusicPlugin::MusicHook::Instance().Initialize();
			CreateThread(NULL, 0, [](LPVOID lpParam) -> DWORD {
				Taskbar::TaskbarWindow::Instance().Create();
				MSG msg;
				while (GetMessage(&msg, NULL, 0, 0)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				return 0;
				}, NULL, 0, NULL);
		}
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
	{
		MusicPlugin::MusicHook::Instance().Free();
		break;
	}
	}
	return TRUE;
}

