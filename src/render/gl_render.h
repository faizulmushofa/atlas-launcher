#ifndef GL_RENDER_H
#define GL_RENDER_H

#include <SDL3/SDL.h>
#include <stdbool.h>

/**
 * Menginisialisasi SDL_Renderer dan menautkannya ke window.
 * Fungsi ini memilih backend GPU terbaik (Metal di Mac, Direct3D di Windows, dst).
 * 
 * @param window Pointer ke SDL_Window yang aktif.
 * @return true jika inisialisasi berhasil, false jika gagal.
 */
bool gl_render_init(SDL_Window* window);

/**
 * Membersihkan seluruh sumber daya renderer.
 * 
 * @param window Pointer ke SDL_Window yang aktif.
 */
void gl_render_cleanup(SDL_Window* window);

/**
 * Merender satu frame visual lengkap (border, background, dan elemen UI).
 * 
 * @param window Pointer ke SDL_Window tempat frame digambar.
 */
void gl_render_frame(SDL_Window* window);

#endif // GL_RENDER_H
