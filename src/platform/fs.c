#include "fs.h"
#include <SDL3/SDL.h>
#include <string.h>

bool fs_get_user_home(char* out_path, size_t max_len) {
    if (!out_path || max_len == 0) return false;

    const char* home = SDL_GetUserFolder(SDL_FOLDER_HOME);
    if (home) {
        size_t len = strlen(home);
        // Hapus trailing path separator jika ada agar konsisten dengan format folder home sebelumnya
        if (len > 0 && (home[len - 1] == '/' || home[len - 1] == '\\')) {
            len--;
        }
        if (len >= max_len) len = max_len - 1;
        strncpy(out_path, home, len);
        out_path[len] = '\0';
        return true;
    }

    return false;
}
