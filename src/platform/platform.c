#ifndef __APPLE__

#include "platform.h"
#include <SDL3/SDL.h>

bool platform_open_app(const char* path) {
    if (!path) return false;
    return SDL_OpenURL(path);
}

#endif // !__APPLE__
