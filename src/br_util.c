#include "br_util.h"
#include "../include/br_util.h"

bool is_null_terminated(const char* str)
{
    size_t i = 0;
    while (i++ < MAX_URI_LENGTH) {
        if (str[i] == 0)
            return true;
    }
    return false;
}

bool is_ip(const char* input)
{
    char* input_d = strdup(input);
    int count = 0;
    char* token = strtok(input_d, ".");
    while (token != NULL) {
        int num = atoi(token);
        if (num < 0 || num > 255) {
            free(input_d);
            return false;
        }
        token = strtok(NULL, ".");
        count++;
    }
    free(input_d);
    return count == 4;
}

void uri_strip(char* str)
{
    char* pos = strchr(str, ':');
    if (pos != NULL)
        *pos = 0;
}

const char* to_abs_path(const char* uri, const char* request_path)
{
    static char r[MAX_URI_LENGTH];
    if (!(is_null_terminated(uri) && is_null_terminated(request_path)))
        return NULL;
    if (strstr(request_path, "://") == NULL)
        return request_path;
    const char* idx = request_path[0] != '/' ? request_path : request_path + 1;
    size_t uri_s = strlen(uri);
    size_t idx_s = strlen(idx);
    memcpy(r, uri, uri_s);
    memcpy(r + uri_s, idx, idx_s);
    return r;
}

int parse_port(const char* URI)
{
    char* port_str;// Check if the given Ip Address
    if ((port_str = strstr(URI, ":")), port_str != NULL) {
        char* endptr = NULL;
        int p = strtol(&port_str[1], &endptr, 10);
        if (*endptr == 0) {
            port_str[0] = 0;
            return p;
        }
    }
    return -1;
}

BR_PROTOCOL capture_protocol(const char* uri, int* start_addr)
{
    if (!strncmp("gemini", uri, 6)) {
        // gemini://
        *start_addr = 9;
        return BR_PROTOCOL_GEMINI;
    } else if (!strncmp("https", uri, 5)) {
        // https://
        *start_addr = 8;
        return BR_PROTOCOL_HTTPS;
    } else if (!strncmp("http", uri, 4)) {
        *start_addr = 7;
        return BR_PROTOCOL_HTTP;
    } else if (!strncmp("gopher", uri, 6)) {
        *start_addr = 9;
        return BR_PROTOCOL_GOPHER;
    }
    return BR_PROTOCOL_UNSUPPORTED;
}

#ifndef _WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>

char* uri_from(const char* ip)
{
    struct in_addr addr;
    inet_pton(AF_INET, ip, &addr);
    struct hostent* host_entry = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    if (host_entry == NULL) {
        return NULL;
    }
    return host_entry->h_name;
}

char* ip_from(const char* hostname)
{
    struct hostent* host_entry = gethostbyname(hostname);
    if (host_entry == NULL) {
        return NULL;
    }
    return inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
}

#endif

#ifdef _WIN32
#include <Windows.h>

void get_os(char* os, size_t os_s)
{
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&osvi)) {
        snprintf(os, os_s, "Windows NT %d.%d", osvi.dwMajorVersion,
                 osvi.dwMinorVersion);
    }
}

#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__)        \
    || defined(__OpenBSD__) || defined(__DragonFly__)
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
