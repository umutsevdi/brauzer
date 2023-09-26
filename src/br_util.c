#include "br_util.h"

#ifdef _WIN32
#include <Windows.h>

void get_os(char* os, size_t os_s)
{
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&osvi)) {
        snprintf(os, os_s, "Windows NT %d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion);
    }
}

#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#include <sys/utsname.h>

void get_os(char* os, size_t os_s)
{
    struct utsname info;
    if (uname(&info) != -1) {
        snprintf(os, os_s, "X11;%s %s", info.sysname, info.machine);
    }
}

#elif __APPLE__
#include <sys/utsname.h>

void get_os(char* os, size_t os_s)
{
    struct utsname info;
    if (uname(&info) != -1) {
        snprintf(os, os_s, "Macintosh; Intel Mac OS X %s", info.release);
    }
}

#else
#error Unsupported platform
#endif
