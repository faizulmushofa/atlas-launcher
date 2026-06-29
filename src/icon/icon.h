#ifndef ICON_H
#define ICON_H

#include <SDL3/SDL.h>

/**
 * Mengambil tekstur icon aplikasi secara native dari OS.
 * @param renderer SDL_Renderer yang aktif.
 * @param app_path Jalur absolut ke berkas/aplikasi.
 * @return SDL_Texture* berisi icon 32x32, atau NULL jika gagal.
 */
SDL_Texture* icon_get_native_texture(SDL_Renderer* renderer, const char* app_path);

#endif // ICON_H
