#include "fs.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

bool fs_get_user_home(char* out_path, size_t max_len) {
    if (!out_path || max_len == 0) return false;

#ifdef _WIN32
    // Windows: Dapatkan folder profil pengguna
    char* userprofile = getenv("USERPROFILE");
    if (userprofile) {
        strncpy(out_path, userprofile, max_len - 1);
        out_path[max_len - 1] = '\0';
        return true;
    }
    // Fallback Windows menggunakan SHGetFolderPathW
    WCHAR wpath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, wpath))) {
        WideCharToMultiByte(CP_UTF8, 0, wpath, -1, out_path, (int)max_len - 1, NULL, NULL);
        out_path[max_len - 1] = '\0';
        return true;
    }
#else
    // macOS / Linux: Menggunakan variabel lingkungan HOME
    char* home = getenv("HOME");
    if (home) {
        strncpy(out_path, home, max_len - 1);
        out_path[max_len - 1] = '\0';
        return true;
    }
#endif

    return false;
}
