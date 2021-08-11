#if defined (_DISCORD_WINDOWS) && defined(_DISCORD_BUILD_DLL)
#include <windows.h>

// outsmart GCC's missing-declarations warning
BOOL WINAPI DllMain(HMODULE, DWORD, LPVOID);
BOOL WINAPI DllMain(HMODULE, DWORD, LPVOID)
{
    return TRUE;
}

#endif // _DISCORD_WINDOWS
