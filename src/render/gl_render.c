#include "gl_render.h"
#include "draw2d.h"
#include "../ui/ui.h"
#include "../icon/icon_cache.h"
#include <stdio.h>

/** Pointer global ke SDL_Renderer yang aktif. */
static SDL_Renderer* g_renderer = NULL;

bool gl_render_init(SDL_Window* window) {
    // Membuat renderer 2D hardware-accelerated bawaan SDL3
    g_renderer = SDL_CreateRenderer(window, NULL);
    if (!g_renderer) {
        printf("Gagal membuat SDL_Renderer: %s\n", SDL_GetError());
        return false;
    }

    // Aktifkan alpha blending global pada renderer agar elemen UI transparan digambar dengan benar
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);

    // Menginisialisasi modul renderer 2D (load font atlas)
    if (!draw2d_init(g_renderer)) {
        printf("Gagal menginisialisasi draw2d font atlas\n");
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
        return false;
    }

    // Inisialisasi cache ikon GPU
    icon_cache_init();

    printf("Renderer SDL3 berhasil diinisialisasi!\n");
    return true;
}

void gl_render_cleanup(SDL_Window* window) {
    (void)window;
    // Bersihkan font atlas dan cache UI
    draw2d_cleanup();
    ui_cleanup();
    icon_cache_cleanup();

    if (g_renderer) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
    }
}

void gl_render_frame(SDL_Window* window) {
    if (!g_renderer) return;

    int w = 800, h = 50;
    SDL_GetWindowSize(window, &w, &h);

    // 1. Clear screen dengan warna transparan penuh
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 0);
    SDL_RenderClear(g_renderer);

    // 2. Gambar Border Putih dan Latar Belakang Gelap dengan sudut melengkung
    SDL_Color border_color = { 255, 255, 255, 255 };
    SDL_Color bg_color = { 20, 24, 30, 255 };

    // Border luar (radius 12, menutup seluruh ukuran window)
    draw2d_fill_rounded_rect(g_renderer, 0, 0, w, h, 12, border_color);

    // Latar belakang dalam (radius 11, inset 1px untuk menyisakan border)
    draw2d_fill_rounded_rect(g_renderer, 1, 1, w - 2, h - 2, 11, bg_color);

    // 3. Gambar Seluruh Elemen UI (search query, placeholder, kursor)
    ui_render(g_renderer, w, h);

    // 4. Tampilkan (swap buffer) frame ke layar
    SDL_RenderPresent(g_renderer);
}
