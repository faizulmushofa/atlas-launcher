#include "generic_icon.h"
#include <string.h>

static void set_pixel(SDL_Surface* surface, int x, int y, uint32_t color) {
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) return;
    uint32_t* pixels = (uint32_t*)surface->pixels;
    pixels[y * surface->w + x] = color;
}

static void draw_line(SDL_Surface* surface, int x, int y, int len, uint32_t color) {
    SDL_Rect rect = { x, y, len, 1 };
    SDL_FillSurfaceRect(surface, &rect, color);
}

static void draw_bordered_rect(SDL_Surface* surface, int x, int y, int w, int h, uint32_t border_color, uint32_t fill_color) {
    SDL_Rect outer = { x, y, w, h };
    SDL_FillSurfaceRect(surface, &outer, border_color);
    SDL_Rect inner = { x + 1, y + 1, w - 2, h - 2 };
    SDL_FillSurfaceRect(surface, &inner, fill_color);
}

SDL_Surface* generic_icon_create_surface(const char* ext_or_type) {
    // Buat surface 32x32 RGBA
    SDL_Surface* surface = SDL_CreateSurface(32, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) return NULL;

    // Bersihkan surface dengan warna transparan penuh
    SDL_FillSurfaceRect(surface, NULL, SDL_MapSurfaceRGBA(surface, 0, 0, 0, 0));

    // Petakan warna standar
    uint32_t c_transparent = SDL_MapSurfaceRGBA(surface, 0, 0, 0, 0);
    uint32_t c_white       = SDL_MapSurfaceRGBA(surface, 250, 250, 250, 255);
    uint32_t c_grey_border  = SDL_MapSurfaceRGBA(surface, 128, 134, 139, 255);
    uint32_t c_grey_line    = SDL_MapSurfaceRGBA(surface, 180, 185, 190, 255);

    if (strcmp(ext_or_type, "folder") == 0) {
        // Ikon Folder 3D (Kuning Premium)
        uint32_t c_folder_border = SDL_MapSurfaceRGBA(surface, 210, 130, 10, 255);
        uint32_t c_folder_back   = SDL_MapSurfaceRGBA(surface, 230, 150, 20, 255);
        uint32_t c_folder_front  = SDL_MapSurfaceRGBA(surface, 253, 195, 60, 255);

        // Tab folder (di belakang)
        draw_bordered_rect(surface, 4, 5, 10, 5, c_folder_border, c_folder_back);
        // Badan belakang folder
        draw_bordered_rect(surface, 4, 9, 24, 18, c_folder_border, c_folder_back);
        // Saku depan folder (untuk efek 3D)
        draw_bordered_rect(surface, 4, 12, 24, 15, c_folder_border, c_folder_front);
        
    } else if (strcmp(ext_or_type, "app") == 0) {
        // Ikon Aplikasi (Terminal Window Premium)
        uint32_t c_term_border = SDL_MapSurfaceRGBA(surface, 60, 64, 67, 255);
        uint32_t c_term_header = SDL_MapSurfaceRGBA(surface, 128, 134, 139, 255);
        uint32_t c_term_bg     = SDL_MapSurfaceRGBA(surface, 30, 30, 30, 255);
        uint32_t c_term_green  = SDL_MapSurfaceRGBA(surface, 52, 168, 83, 255);

        // Frame terminal
        draw_bordered_rect(surface, 4, 6, 24, 20, c_term_border, c_term_bg);
        // Bar judul terminal
        SDL_Rect header_rect = { 5, 7, 22, 4 };
        SDL_FillSurfaceRect(surface, &header_rect, c_term_header);

        // Karakter prompt '>' hijau
        set_pixel(surface, 7, 14, c_term_green);
        set_pixel(surface, 8, 15, c_term_green);
        set_pixel(surface, 7, 16, c_term_green);
        // Kursor '_' hijau berkelip
        draw_line(surface, 10, 16, 3, c_term_green);

    } else if (strcmp(ext_or_type, "pdf") == 0) {
        // Ikon PDF (Kertas Putih + Aksen Merah PDF)
        uint32_t c_pdf_border = SDL_MapSurfaceRGBA(surface, 219, 68, 85, 255);
        uint32_t c_pdf_red    = SDL_MapSurfaceRGBA(surface, 234, 67, 53, 255);

        // Gambar kertas dasar
        draw_bordered_rect(surface, 8, 5, 16, 22, c_grey_border, c_white);
        // Sudut terlipat (kanan atas)
        set_pixel(surface, 22, 5, c_transparent);
        set_pixel(surface, 23, 5, c_transparent);
        set_pixel(surface, 23, 6, c_transparent);
        // Lipatan kertas
        set_pixel(surface, 22, 6, c_grey_border);

        // Pita merah "PDF" di tengah kertas
        draw_bordered_rect(surface, 10, 11, 12, 6, c_pdf_border, c_pdf_red);
        // Garis putih menyerupai teks di dalam pita
        draw_line(surface, 12, 13, 8, c_white);
        draw_line(surface, 12, 15, 6, c_white);

    } else if (strcmp(ext_or_type, "png") == 0 || strcmp(ext_or_type, "jpg") == 0 || strcmp(ext_or_type, "jpeg") == 0 || strcmp(ext_or_type, "gif") == 0) {
        // Ikon Gambar (Kertas Putih + Landscape Gunung & Matahari)
        uint32_t c_img_border = SDL_MapSurfaceRGBA(surface, 15, 157, 88, 255);
        uint32_t c_sky        = SDL_MapSurfaceRGBA(surface, 200, 230, 255, 255);
        uint32_t c_mountain   = SDL_MapSurfaceRGBA(surface, 52, 168, 83, 255);
        uint32_t c_sun        = SDL_MapSurfaceRGBA(surface, 251, 188, 5, 255);

        draw_bordered_rect(surface, 8, 5, 16, 22, c_grey_border, c_white);
        // Sudut terlipat
        set_pixel(surface, 22, 5, c_transparent);
        set_pixel(surface, 23, 5, c_transparent);
        set_pixel(surface, 23, 6, c_transparent);
        set_pixel(surface, 22, 6, c_grey_border);

        // Bingkai gambar di dalam kertas
        draw_bordered_rect(surface, 10, 10, 12, 10, c_img_border, c_sky);
        // Matahari piksel
        set_pixel(surface, 18, 12, c_sun);
        // Gunung hijau piksel
        set_pixel(surface, 13, 16, c_mountain);
        draw_line(surface, 12, 17, 3, c_mountain);
        draw_line(surface, 11, 18, 8, c_mountain);

    } else if (strcmp(ext_or_type, "zip") == 0 || strcmp(ext_or_type, "rar") == 0 || strcmp(ext_or_type, "tar") == 0 || strcmp(ext_or_type, "gz") == 0) {
        // Ikon Arsip (Kertas Putih + Aksen Resleting Kuning)
        uint32_t c_zip_accent = SDL_MapSurfaceRGBA(surface, 244, 180, 0, 255);
        uint32_t c_zip_metal  = SDL_MapSurfaceRGBA(surface, 95, 99, 104, 255);

        draw_bordered_rect(surface, 8, 5, 16, 22, c_grey_border, c_white);
        // Sudut terlipat
        set_pixel(surface, 22, 5, c_transparent);
        set_pixel(surface, 23, 5, c_transparent);
        set_pixel(surface, 23, 6, c_transparent);
        set_pixel(surface, 22, 6, c_grey_border);

        // Gambar pola zipper (resleting) vertikal di tengah kertas
        for (int y = 9; y < 19; y += 2) {
            set_pixel(surface, 15, y, c_zip_accent);
            set_pixel(surface, 16, y + 1, c_zip_metal);
        }

    } else {
        // Ikon Dokumen Teks Fallback (Kertas Putih + Garis Teks Abu-abu)
        draw_bordered_rect(surface, 8, 5, 16, 22, c_grey_border, c_white);
        // Sudut terlipat
        set_pixel(surface, 22, 5, c_transparent);
        set_pixel(surface, 23, 5, c_transparent);
        set_pixel(surface, 23, 6, c_transparent);
        set_pixel(surface, 22, 6, c_grey_border);

        // Garis teks horizontal
        draw_line(surface, 11, 10, 10, c_grey_line);
        draw_line(surface, 11, 13, 8, c_grey_line);
        draw_line(surface, 11, 16, 10, c_grey_line);
        draw_line(surface, 11, 19, 6, c_grey_line);
    }

    return surface;
}
