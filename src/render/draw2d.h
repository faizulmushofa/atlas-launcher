#ifndef DRAW2D_H
#define DRAW2D_H

#include <SDL3/SDL.h>
#include <stdbool.h>

/**
 * Menginisialisasi modul renderer 2D, termasuk memuat tekstur solid 1x1 
 * dan memuat/membangun font atlas di memori GPU.
 * 
 * @param renderer SDL_Renderer yang aktif.
 * @return true jika berhasil, false jika terjadi kegagalan.
 */
bool draw2d_init(SDL_Renderer* renderer);

/**
 * Membersihkan seluruh sumber daya (tekstur, buffer) milik modul renderer 2D.
 */
void draw2d_cleanup(void);

/**
 * Menggambar bentuk persegi panjang (solid rectangle) berwarna tertentu.
 * 
 * @param renderer SDL_Renderer yang aktif.
 * @param x Posisi koordinat X tujuan (dalam piksel).
 * @param y Posisi koordinat Y tujuan (dalam piksel).
 * @param w Lebar persegi panjang (dalam piksel).
 * @param h Tinggi persegi panjang (dalam piksel).
 * @param color Warna persegi panjang (RGBA).
 */
void draw2d_rect(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color);

/**
 * Menggambar bentuk persegi panjang berongga (outline) berwarna tertentu.
 * 
 * @param renderer SDL_Renderer yang aktif.
 * @param x Posisi koordinat X tujuan.
 * @param y Posisi koordinat Y tujuan.
 * @param w Lebar.
 * @param h Tinggi.
 * @param color Warna outline (RGBA).
 * @param thickness Ketebalan garis outline (dalam piksel).
 */
void draw2d_rect_outline(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color, float thickness);

/**
 * Menggambar bentuk persegi panjang dengan sudut melengkung (filled rounded rectangle) berwarna tertentu.
 * 
 * @param renderer SDL_Renderer yang aktif.
 * @param x Posisi koordinat X tujuan.
 * @param y Posisi koordinat Y tujuan.
 * @param w Lebar.
 * @param h Tinggi.
 * @param r Radius sudut (dalam piksel).
 * @param color Warna (RGBA).
 */
void draw2d_fill_rounded_rect(SDL_Renderer* renderer, int x, int y, int w, int h, int r, SDL_Color color);



/**
 * Membuat tekstur teks baru menggunakan font bawaan sistem operasi.
 * 
 * @param renderer SDL_Renderer yang aktif.
 * @param text String teks yang akan dirender.
 * @param color Warna teks (RGBA).
 * @param out_w Pointer untuk mendapatkan lebar tekstur hasil (opsional).
 * @param out_h Pointer untuk mendapatkan tinggi tekstur hasil (opsional).
 * @return SDL_Texture yang berisi render teks native, atau NULL jika gagal.
 */
SDL_Texture* draw2d_create_text_texture_native(SDL_Renderer* renderer, const char* text, SDL_Color color, int* out_w, int* out_h);

#endif // DRAW2D_H
