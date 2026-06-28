#ifndef ICON_CACHE_H
#define ICON_CACHE_H

#include <SDL3/SDL.h>

/**
 * Menginisialisasi cache ikon di memori.
 */
void icon_cache_init(void);

/**
 * Mendapatkan tekstur ikon aplikasi, memuat secara native dari OS jika belum ada di cache.
 * 
 * @param renderer SDL_Renderer yang aktif.
 * @param app_path Jalur absolut ke berkas/aplikasi.
 * @return SDL_Texture* berisi ikon 32x32.
 */
SDL_Texture* icon_cache_get(SDL_Renderer* renderer, const char* app_path);

/**
 * Membersihkan seluruh tekstur ikon yang dicache dan membebaskan memori GPU.
 */
void icon_cache_cleanup(void);

#endif // ICON_CACHE_H
