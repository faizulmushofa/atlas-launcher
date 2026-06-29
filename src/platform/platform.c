#ifndef __APPLE__

#include "platform.h"
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>

bool platform_open_app(const char* path) {
    int wlen = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
    if (wlen <= 0) return false;
    wchar_t* wpath = (wchar_t*)malloc(wlen * sizeof(wchar_t));
    if (!wpath) return false;
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, wlen);
    
    HINSTANCE result = ShellExecuteW(NULL, L"open", wpath, NULL, NULL, SW_SHOWNORMAL);
    free(wpath);
    return (intptr_t)result > 32;
}

#elif defined(__linux__)
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

bool platform_open_app(const char* path) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: jalankan aplikasi di background
        char command[512];
        snprintf(command, sizeof(command), "%s &", path);
        int ret = system(command);
        exit(ret == 0 ? 0 : 1);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
    return false;
}

#else

bool platform_open_app(const char* path) {
    (void)path;
    return false;
}

#endif

#endif // !__APPLE__
