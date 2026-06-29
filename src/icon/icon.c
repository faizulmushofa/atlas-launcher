#include "icon.h"
#include "pe_parser.h"
#include "icns_parser.h"
#include "generic_icon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void get_lowercase_extension(const char* path, char* out_ext, size_t max_len) {
    const char* dot = strrchr(path, '.');
    if (!dot || dot == path) {
        out_ext[0] = '\0';
        return;
    }
    size_t len = strlen(dot + 1);
    if (len >= max_len) len = max_len - 1;
    for (size_t i = 0; i < len; i++) {
        char c = dot[1 + i];
        if (c >= 'A' && c <= 'Z') c = c + ('a' - 'A');
        out_ext[i] = c;
    }
    out_ext[len] = '\0';
}

SDL_Texture* icon_get_native_texture(SDL_Renderer* renderer, const char* app_path) {
    SDL_PathInfo info;
    bool is_dir = false;
    
    if (SDL_GetPathInfo(app_path, &info)) {
        if (info.type == SDL_PATHTYPE_DIRECTORY) {
            is_dir = true;
        }
    }
    
    char ext[16];
    get_lowercase_extension(app_path, ext, sizeof(ext));
    
    SDL_Surface* surface = NULL;
    
    if (is_dir) {
        // Jika folder berkas di macOS (.app)
        if (strcmp(ext, "app") == 0) {
            size_t png_size = 0;
            unsigned char* png_data = macos_extract_bundle_icon(app_path, &png_size);
            if (png_data) {
                SDL_IOStream* io = SDL_IOFromConstMem(png_data, png_size);
                if (io) {
                    surface = SDL_LoadPNG_IO(io, true); // true = menutup io otomatis
                }
                free(png_data);
            }
            
            // Fallback jika ekstraksi .app gagal
            if (!surface) {
                surface = generic_icon_create_surface("app");
            }
        } else {
            // Folder biasa
            surface = generic_icon_create_surface("folder");
        }
    } else {
        // Jika file executable Windows (.exe atau .dll)
        if (strcmp(ext, "exe") == 0 || strcmp(ext, "dll") == 0) {
            size_t icon_size = 0;
            int is_png = 0;
            unsigned char* icon_data = pe_extract_icon_data(app_path, &icon_size, &is_png);
            if (icon_data) {
                if (is_png) {
                    SDL_IOStream* io = SDL_IOFromConstMem(icon_data, icon_size);
                    if (io) {
                        surface = SDL_LoadPNG_IO(io, true);
                    }
                } else {
                    int w = 0, h = 0;
                    unsigned char* rgba = decode_dib_to_rgba(icon_data, icon_size, &w, &h);
                    if (rgba) {
                        surface = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, rgba, w * 4);
                        if (surface) {
                            // Untuk SDL_CreateSurfaceFrom, data piksel harus tetap hidup sampai surface dihancurkan.
                            // Kita buat duplikat texture dari surface, lalu bebaskan rgba.
                            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                            SDL_DestroySurface(surface);
                            free(rgba);
                            free(icon_data);
                            return texture;
                        }
                        free(rgba);
                    }
                }
                free(icon_data);
            }
            
            // Fallback jika ekstraksi EXE gagal
            if (!surface) {
                surface = generic_icon_create_surface("app");
            }
        } else {
            // Dokumen biasa (txt, pdf, zip, gambar, dll)
            surface = generic_icon_create_surface(ext);
        }
    }
    
    if (!surface) {
        // Fallback mutlak jika tidak ada ikon yang terbuat
        surface = generic_icon_create_surface("unknown");
    }
    
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
        return texture;
    }
    
    return NULL;
}
