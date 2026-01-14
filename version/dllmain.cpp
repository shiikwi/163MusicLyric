#pragma comment(lib, "./MinHook/libMinHook.lib")
#include "MusicHook.h"
#include "./Hijack/version.h";

extern "C" __declspec(dllexport) void Lyrics() {}

bool IsMainProcess()
{
    wchar_t* cmdLine = GetCommandLineW();
    if (wcsstr(cmdLine, L"--type="))
    {
        return false;
    }
    return true;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);
        Utils::Console::AttachConsole();
        if (IsMainProcess())
        {
            MusicPlugin::MusicHook::Instance().Initialize();
            //CreateThread(NULL, 0, [](LPVOID lpParam) -> DWORD {
            //    MusicPlugin::MusicHook::Instance().Initialize();
            //    return 0;
            //    }, NULL, 0, NULL);
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

