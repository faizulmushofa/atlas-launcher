#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>

/**
 * Merender seluruh layout antarmuka pengguna (UI) Spotlight ke window.
 * 
 * @param renderer SDL_Renderer yang aktif.
 * @param window_w Lebar jendela (dalam piksel).
 * @param window_h Tinggi jendela (dalam piksel).
 */
void ui_render(SDL_Renderer* renderer, int window_w, int window_h);

/**
 * Membersihkan tekstur cache UI yang dibuat secara dinamis saat runtime.
 */
void ui_cleanup(void);

#endif // UI_H
