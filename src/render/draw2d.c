#include "draw2d.h"
#include "../platform/detection.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

bool draw2d_init(SDL_Renderer* renderer) {
    (void)renderer;
    return true;
}

void draw2d_cleanup(void) {
    // Tidak ada yang perlu dibersihkan
}

void draw2d_rect(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color) {
    SDL_FRect rect = { (float)x, (float)y, (float)w, (float)h };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void draw2d_rect_outline(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color, float thickness) {
    SDL_FRect rect = { (float)x, (float)y, (float)w, (float)h };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Jika ketebalan garis standar (1px), gunakan SDL_RenderRect bawaan SDL3
    if (thickness <= 1.0f) {
        SDL_RenderRect(renderer, &rect);
    } else {
        // Jika butuh ketebalan lebih, gambar menggunakan 4 persegi tipis
        // Sisi atas
        draw2d_rect(renderer, x, y, w, (int)thickness, color);
        // Sisi bawah
        draw2d_rect(renderer, x, y + h - (int)thickness, w, (int)thickness, color);
        // Sisi kiri
        draw2d_rect(renderer, x, y, (int)thickness, h, color);
        // Sisi kanan
        draw2d_rect(renderer, x + w - (int)thickness, y, (int)thickness, h, color);
    }
}

void draw2d_fill_rounded_rect(SDL_Renderer* renderer, int x, int y, int w, int h, int r, SDL_Color color) {
    if (r <= 0) {
        draw2d_rect(renderer, x, y, w, h, color);
        return;
    }
    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;

    // 1. Draw central rectangular body (cross shape)
    // Horizontal central body (covers full width minus corners, full height)
    SDL_FRect center_rect = { (float)(x + r), (float)y, (float)(w - 2 * r), (float)h };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &center_rect);

    // Left vertical strip (covers left edge, height from y+r to y+h-r)
    SDL_FRect left_rect = { (float)x, (float)(y + r), (float)r, (float)(h - 2 * r) };
    SDL_RenderFillRect(renderer, &left_rect);

    // Right vertical strip (covers right edge, height from y+r to y+h-r)
    SDL_FRect right_rect = { (float)(x + w - r), (float)(y + r), (float)r, (float)(h - 2 * r) };
    SDL_RenderFillRect(renderer, &right_rect);

    // 2. Draw the 4 corner circles (quarter circles)
    for (int dy = 0; dy <= r; dy++) {
        int dx = (int)(sqrtf((float)(r * r - dy * dy)) + 0.5f);
        
        // Top-left corner: line from (x + r - dx) to (x + r) at (y + r - dy)
        SDL_RenderLine(renderer, (float)(x + r - dx), (float)(y + r - dy), (float)(x + r), (float)(y + r - dy));
        
        // Top-right corner: line from (x + w - r) to (x + w - r + dx) at (y + r - dy)
        SDL_RenderLine(renderer, (float)(x + w - r), (float)(y + r - dy), (float)(x + w - r + dx), (float)(y + r - dy));
        
        // Bottom-left corner: line from (x + r - dx) to (x + r) at (y + h - r + dy)
        SDL_RenderLine(renderer, (float)(x + r - dx), (float)(y + h - r + dy), (float)(x + r), (float)(y + h - r + dy));
        
        // Bottom-right corner: line from (x + w - r) to (x + w - r + dx) at (y + h - r + dy)
        SDL_RenderLine(renderer, (float)(x + w - r), (float)(y + h - r + dy), (float)(x + w - r + dx), (float)(y + h - r + dy));
    }
}

#if defined(PLATFORM_OS_MACOS)
extern SDL_Texture* platform_create_text_texture_macos(SDL_Renderer* renderer, const char* text, SDL_Color color, int* out_w, int* out_h);
#elif defined(PLATFORM_OS_WINDOWS)
extern SDL_Texture* platform_create_text_texture_windows(SDL_Renderer* renderer, const char* text, SDL_Color color, int* out_w, int* out_h);
#endif

SDL_Texture* draw2d_create_text_texture_native(SDL_Renderer* renderer, const char* text, SDL_Color color, int* out_w, int* out_h) {
    if (!text || strlen(text) == 0) return NULL;

#if defined(PLATFORM_OS_MACOS)
    return platform_create_text_texture_macos(renderer, text, color, out_w, out_h);
#elif defined(PLATFORM_OS_WINDOWS)
    return platform_create_text_texture_windows(renderer, text, color, out_w, out_h);
#else
    // Fallback untuk platform lain yang belum didukung
    (void)renderer;
    (void)color;
    (void)out_w;
    (void)out_h;
    return NULL;
#endif
}
