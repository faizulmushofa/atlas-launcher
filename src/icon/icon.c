#include "icon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>

static unsigned char* win32_get_app_icon_rgba(const char* app_path, int* out_width, int* out_height) {
    WCHAR wpath[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, app_path, -1, wpath, MAX_PATH);

    SHFILEINFOW sfi;
    if (!SHGetFileInfoW(wpath, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON)) {
        return NULL;
    }

    HICON hIcon = sfi.hIcon;
    if (!hIcon) return NULL;

    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo)) {
        DestroyIcon(hIcon);
        return NULL;
    }

    int w = 32;
    int h = 32;
    *out_width = w;
    *out_height = h;

    unsigned char* pixels = (unsigned char*)malloc(w * h * 4);
    if (!pixels) {
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        DestroyIcon(hIcon);
        return NULL;
    }

    HDC hdc = GetDC(NULL);
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h; 
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    GetDIBits(hdc, iconInfo.hbmColor, 0, h, pixels, &bmi, DIB_RGB_COLORS);
    ReleaseDC(NULL, hdc);

    // Tukar BGRA ke RGBA (Windows menggunakan BGRA bawaan)
    for (int i = 0; i < w * h * 4; i += 4) {
        unsigned char b = pixels[i];
        unsigned char r = pixels[i + 2];
        pixels[i] = r;
        pixels[i + 2] = b;
        
        // Memperbaiki alpha channel dari HICON (jika transparan tapi ada warna, set alpha = 255)
        if (pixels[i + 3] == 0) {
            if (pixels[i] != 0 || pixels[i+1] != 0 || pixels[i+2] != 0) {
                pixels[i + 3] = 255;
            }
        }
    }

    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
    DestroyIcon(hIcon);

    return pixels;
}
#endif

#ifdef __APPLE__
// Fungsi wrapper yang diimplementasikan di icon_os_macos.m
extern unsigned char* macos_get_app_icon_rgba(const char* app_path, int* out_width, int* out_height);
#endif

static SDL_Texture* create_fallback_texture(SDL_Renderer* renderer) {
    SDL_Surface* surface = SDL_CreateSurface(32, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) return NULL;
    
    // Gambar icon default berbentuk kotak berwarna abu-abu biru premium
    SDL_FillSurfaceRect(surface, NULL, SDL_MapSurfaceRGBA(surface, 100, 115, 135, 255));
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return texture;
}

SDL_Texture* icon_get_native_texture(SDL_Renderer* renderer, const char* app_path) {
    unsigned char* pixel_data = NULL;
    int w = 0, h = 0;

#if defined(__APPLE__)
    pixel_data = macos_get_app_icon_rgba(app_path, &w, &h);
#elif defined(_WIN32)
    pixel_data = win32_get_app_icon_rgba(app_path, &w, &h);
#endif

    if (!pixel_data) {
        // Gunakan fallback jika OS lain atau gagal memuat
        return create_fallback_texture(renderer);
    }

    // Buat surface dari data pixel mentah RGBA
    SDL_Surface* surface = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pixel_data, w * 4);
    if (!surface) {
        free(pixel_data);
        return create_fallback_texture(renderer);
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    free(pixel_data);

    if (!texture) {
        return create_fallback_texture(renderer);
    }

    return texture;
}
