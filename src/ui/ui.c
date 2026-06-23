#include "ui.h"
#include "../core/state.h"
#include "../render/draw2d.h"
#include <string.h>

/** Tekstur cache untuk query pencarian aktif */
static SDL_Texture* g_query_texture = NULL;
static char g_query_cache[256] = "";
static int g_query_w = 0;
static int g_query_h = 0;

/** Tekstur cache untuk placeholder pencarian */
static SDL_Texture* g_placeholder_texture = NULL;
static int g_placeholder_w = 0;
static int g_placeholder_h = 0;

void ui_cleanup(void) {
    if (g_query_texture) {
        SDL_DestroyTexture(g_query_texture);
        g_query_texture = NULL;
    }
    g_query_cache[0] = '\0';
    g_query_w = 0;
    g_query_h = 0;

    if (g_placeholder_texture) {
        SDL_DestroyTexture(g_placeholder_texture);
        g_placeholder_texture = NULL;
    }
    g_placeholder_w = 0;
    g_placeholder_h = 0;
}

void ui_render(SDL_Renderer* renderer, int window_w, int window_h) {
    (void)window_w;
    AppState* state = state_get();
    if (!state) return;

    // Warna
    SDL_Color color_active = { 240, 240, 245, 255 }; // Putih keperakan
    SDL_Color color_placeholder = { 90, 100, 120, 255 }; // Abu-abu gelap teredam
    SDL_Color color_cursor = { 64, 156, 255, 255 }; // Biru terang premium (Mac style)

    int start_x = 16;
    int text_h = 24; // Tinggi teks perkiraan (akan disesuaikan dinamis)
    int start_y = (window_h - text_h) / 2;

    // 1. Render Placeholder jika query kosong
    if (state->query_len == 0) {
        // Buat cache tekstur placeholder sekali saja di awal
        if (!g_placeholder_texture) {
            const char* placeholder = "Search files, folders or apps...";
            g_placeholder_texture = draw2d_create_text_texture_native(renderer, placeholder, color_placeholder, &g_placeholder_w, &g_placeholder_h);
        }
        
        if (g_placeholder_texture) {
            int draw_y = (window_h - g_placeholder_h) / 2;
            SDL_FRect dest_rect = { (float)start_x, (float)draw_y, (float)g_placeholder_w, (float)g_placeholder_h };
            SDL_RenderTexture(renderer, g_placeholder_texture, NULL, &dest_rect);
            text_h = g_placeholder_h;
            start_y = draw_y;
        }
    } else {
        // 2. Render Kueri teks yang diketik (gunakan cache jika teks tidak berubah)
        if (strcmp(g_query_cache, state->search_query) != 0 || !g_query_texture) {
            if (g_query_texture) {
                SDL_DestroyTexture(g_query_texture);
                g_query_texture = NULL;
            }
            
            strncpy(g_query_cache, state->search_query, sizeof(g_query_cache) - 1);
            g_query_texture = draw2d_create_text_texture_native(renderer, state->search_query, color_active, &g_query_w, &g_query_h);
        }

        if (g_query_texture) {
            int draw_y = (window_h - g_query_h) / 2;
            SDL_FRect dest_rect = { (float)start_x, (float)draw_y, (float)g_query_w, (float)g_query_h };
            SDL_RenderTexture(renderer, g_query_texture, NULL, &dest_rect);
            text_h = g_query_h;
            start_y = draw_y;
        }
    }

    // 3. Hitung pergeseran posisi kursor dinamis (karena font native memiliki lebar karakter proporsional)
    int cursor_offset_x = 0;
    if (state->cursor_position > 0 && state->query_len > 0) {
        char prefix[256];
        int prefix_len = state->cursor_position;
        if (prefix_len > state->query_len) prefix_len = state->query_len;
        strncpy(prefix, state->search_query, prefix_len);
        prefix[prefix_len] = '\0';

        // Buat tekstur sementara untuk substring sebelum kursor demi mengukur lebarnya
        int prefix_w = 0, prefix_h = 0;
        SDL_Texture* temp_tex = draw2d_create_text_texture_native(renderer, prefix, color_active, &prefix_w, &prefix_h);
        if (temp_tex) {
            cursor_offset_x = prefix_w;
            SDL_DestroyTexture(temp_tex);
        }
    }

    // 4. Gambar Kursor Berkedip (setiap 500ms bergantian hidup/mati)
    Uint64 ticks = SDL_GetTicks();
    if ((ticks / 500) % 2 == 0) {
        int cursor_x = start_x + cursor_offset_x;
        int cursor_h = text_h > 0 ? text_h : 24;
        draw2d_rect(renderer, cursor_x, start_y, 2, cursor_h, color_cursor);
    }
}
