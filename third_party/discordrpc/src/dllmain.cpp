#ifdef _DISCORD_WINDOWS
#include <windows.h>
#endif // _DISCORD_WINDOWS

// outsmart GCC's missing-declarations warning
BOOL WINAPI DllMain(HMODULE, DWORD, LPVOID);
BOOL WINAPI DllMain(HMODULE, DWORD, LPVOID)
{
    return TRUE;
}
