#include "ui.h"
#include "../core/state.h"
#include "../render/draw2d.h"
#include "../icon/icon.h"
#include "../icon/icon_cache.h"
#include "../db/sqlite.h"
#include <string.h>
#include <stdio.h>

/** Tekstur cache untuk query pencarian aktif */
static SDL_Texture* g_query_texture = NULL;
static char g_query_cache[256] = "";
static int g_query_w = 0;
static int g_query_h = 0;

/** Tekstur cache untuk placeholder pencarian */
static SDL_Texture* g_placeholder_texture = NULL;
static int g_placeholder_w = 0;
static int g_placeholder_h = 0;

/** Tekstur cache untuk baris hasil pencarian */
static char g_result_query_cache[256] = "";
static SDL_Texture* g_result_name_textures[5] = {NULL};
static int g_result_name_w[5] = {0};
static int g_result_name_h[5] = {0};

static SDL_Texture* g_result_type_textures[5] = {NULL};
static int g_result_type_w[5] = {0};
static int g_result_type_h[5] = {0};

static SDL_Texture* g_result_icon_textures[5] = {NULL};

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

    for (int i = 0; i < 5; i++) {
        if (g_result_name_textures[i]) {
            SDL_DestroyTexture(g_result_name_textures[i]);
            g_result_name_textures[i] = NULL;
        }
        if (g_result_type_textures[i]) {
            SDL_DestroyTexture(g_result_type_textures[i]);
            g_result_type_textures[i] = NULL;
        }
        g_result_icon_textures[i] = NULL;
        g_result_name_w[i] = 0;
        g_result_name_h[i] = 0;
        g_result_type_w[i] = 0;
        g_result_type_h[i] = 0;
    }
    g_result_query_cache[0] = '\0';
}

void ui_render(SDL_Renderer* renderer, int window_w, int window_h) {
    (void)window_h;
    AppState* state = state_get();
    if (!state) return;

    // Warna
    SDL_Color color_active = { 240, 240, 245, 255 }; // Putih keperakan
    SDL_Color color_placeholder = { 90, 100, 120, 255 }; // Abu-abu gelap teredam
    SDL_Color color_cursor = { 64, 156, 255, 255 }; // Biru terang premium (Mac style)
    SDL_Color color_divider = { 50, 55, 65, 255 }; // Garis pemisah abu-abu gelap
    SDL_Color color_highlight = { 64, 156, 255, 120 }; // Biru semi-transparan premium

    int start_x = 16;
    int text_h = 24; // Tinggi teks perkiraan (akan disesuaikan dinamis)
    int start_y = (50 - text_h) / 2; // Bar pencarian selalu memiliki tinggi tetap 50px di bagian atas

    // 1. Render Placeholder jika query kosong
    if (state->query_len == 0) {
        if (!g_placeholder_texture) {
            const char* placeholder = "Search files, folders or apps...";
            g_placeholder_texture = draw2d_create_text_texture_native(renderer, placeholder, color_placeholder, &g_placeholder_w, &g_placeholder_h);
        }
        
        if (g_placeholder_texture) {
            int draw_y = (50 - g_placeholder_h) / 2;
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
            
            snprintf(g_query_cache, sizeof(g_query_cache), "%s", state->search_query);
            g_query_texture = draw2d_create_text_texture_native(renderer, state->search_query, color_active, &g_query_w, &g_query_h);
        }

        if (g_query_texture) {
            int draw_y = (50 - g_query_h) / 2;
            SDL_FRect dest_rect = { (float)start_x, (float)draw_y, (float)g_query_w, (float)g_query_h };
            SDL_RenderTexture(renderer, g_query_texture, NULL, &dest_rect);
            text_h = g_query_h;
            start_y = draw_y;
        }
    }

    // 3. Hitung pergeseran posisi kursor dinamis
    int cursor_offset_x = 0;
    if (state->cursor_position > 0 && state->query_len > 0) {
        char prefix[256];
        int prefix_len = state->cursor_position;
        if (prefix_len > state->query_len) prefix_len = state->query_len;
        strncpy(prefix, state->search_query, prefix_len);
        prefix[prefix_len] = '\0';

        int prefix_w = 0, prefix_h = 0;
        SDL_Texture* temp_tex = draw2d_create_text_texture_native(renderer, prefix, color_active, &prefix_w, &prefix_h);
        if (temp_tex) {
            cursor_offset_x = prefix_w;
            SDL_DestroyTexture(temp_tex);
        }
    }

    // 4. Gambar Kursor Berkedip
    Uint64 ticks = SDL_GetTicks();
    if ((ticks / 500) % 2 == 0) {
        int cursor_x = start_x + cursor_offset_x;
        int cursor_h = text_h > 0 ? text_h : 24;
        draw2d_rect(renderer, cursor_x, start_y, 2, cursor_h, color_cursor);
    }

    // 4b. Gambar Tombol Hijau (+) di kanan Search Bar untuk membuka file dialog
    int plus_x = window_w - 40;
    int plus_y = 15;
    SDL_Color green_color = { 46, 204, 113, 255 }; // Hijau premium
    draw2d_fill_rounded_rect(renderer, plus_x, plus_y, 20, 20, 10, green_color);
    
    // Tanda plus (+) putih di dalam lingkaran
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderLine(renderer, (float)(plus_x + 5), (float)(plus_y + 10), (float)(plus_x + 15), (float)(plus_y + 10));
    SDL_RenderLine(renderer, (float)(plus_x + 10), (float)(plus_y + 5), (float)(plus_x + 10), (float)(plus_y + 15));

    // 5. Render Dropdown Hasil Pencarian
    if (state->result_count > 0) {
        // Hancurkan cache tekstur baris lama jika query berubah ATAU query kosong (untuk daftar shortcuts dinamis)
        if (state->query_len == 0 || strcmp(g_result_query_cache, state->search_query) != 0) {
            for (int i = 0; i < 5; i++) {
                if (g_result_name_textures[i]) {
                    SDL_DestroyTexture(g_result_name_textures[i]);
                    g_result_name_textures[i] = NULL;
                }
                if (g_result_type_textures[i]) {
                    SDL_DestroyTexture(g_result_type_textures[i]);
                    g_result_type_textures[i] = NULL;
                }
                g_result_icon_textures[i] = NULL;
            }
            snprintf(g_result_query_cache, sizeof(g_result_query_cache), "%s", state->search_query);

            // Buat cache tekstur untuk hasil pencarian aktif
            for (int i = 0; i < state->result_count; i++) {
                g_result_name_textures[i] = draw2d_create_text_texture_native(renderer, state->results[i].name, color_active, &g_result_name_w[i], &g_result_name_h[i]);
                
                // Tambahkan keterangan tipe
                char type_label[64];
                snprintf(type_label, sizeof(type_label), "(%s)", state->results[i].type);
                g_result_type_textures[i] = draw2d_create_text_texture_native(renderer, type_label, color_placeholder, &g_result_type_w[i], &g_result_type_h[i]);

                // Ambil icon aplikasi dari cache GPU
                g_result_icon_textures[i] = icon_cache_get(renderer, state->results[i].path);
            }
        }

        // Gambar Garis Pemisah (Divider)
        draw2d_rect(renderer, 0, 50, window_w, 1, color_divider);

        // Gambar daftar baris hasil pencarian
        for (int i = 0; i < state->result_count; i++) {
            int row_y = 50 + i * 40;

            // Highlight baris yang sedang dipilih
            if (i == state->selected_index) {
                // Menggambar kotak rounded highlight modern bergaya macOS
                draw2d_fill_rounded_rect(renderer, 8, row_y + 4, window_w - 16, 32, 6, color_highlight);
            }

            // Gambar Icon (Sisi Kiri, x = 24, y = row_y + 4, w = 32, h = 32)
            if (g_result_icon_textures[i]) {
                SDL_FRect icon_rect = { 24.0f, (float)(row_y + 4), 32.0f, 32.0f };
                SDL_RenderTexture(renderer, g_result_icon_textures[i], NULL, &icon_rect);
            }

            // Gambar Nama Item (Sisi Kiri, digeser ke x = 68 untuk memberi ruang bagi icon)
            if (g_result_name_textures[i]) {
                int draw_y = row_y + (40 - g_result_name_h[i]) / 2;
                SDL_FRect dest_rect = { 68.0f, (float)draw_y, (float)g_result_name_w[i], (float)g_result_name_h[i] };
                SDL_RenderTexture(renderer, g_result_name_textures[i], NULL, &dest_rect);
            }

            // Gambar Tipe/Keterangan Item (Sisi Kanan)
            if (g_result_type_textures[i]) {
                int draw_y = row_y + (40 - g_result_type_h[i]) / 2;
                float draw_x;
                if (state->query_len == 0) {
                    draw_x = (float)(window_w - 55 - g_result_type_w[i]);
                } else {
                    draw_x = (float)(window_w - 24 - g_result_type_w[i]);
                }
                SDL_FRect dest_rect = { draw_x, (float)draw_y, (float)g_result_type_w[i], (float)g_result_type_h[i] };
                SDL_RenderTexture(renderer, g_result_type_textures[i], NULL, &dest_rect);
            }

            // Gambar tombol merah (-) hanya pada tampilan daftar pintasan (kueri kosong)
            if (state->query_len == 0) {
                int btn_x = window_w - 40;
                int btn_y = row_y + 10;
                SDL_Color red_color = { 231, 76, 60, 255 };
                draw2d_fill_rounded_rect(renderer, btn_x, btn_y, 20, 20, 10, red_color);
                
                // Tanda minus (-) putih
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderLine(renderer, (float)(btn_x + 5), (float)(btn_y + 10), (float)(btn_x + 15), (float)(btn_y + 10));
            }
        }
    }
}

void ui_invalidate_cache(void) {
    g_result_query_cache[0] = '\0';
}

