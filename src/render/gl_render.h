#ifndef GL_RENDER_H
#define GL_RENDER_H

#include <SDL3/SDL.h>
#include <stdbool.h>

/**
 * Menginisialisasi perangkat SDL_GPU dan menautkannya ke window.
 * Fungsi ini membuat device GPU abstrak (SDL_GPUDevice) dan mengklaim window
 * utama agar dapat digunakan untuk proses rendering GPU.
 * 
 * @param window Pointer ke SDL_Window yang aktif.
 * @return true jika inisialisasi GPU berhasil, false jika gagal.
 */
bool gl_render_init(SDL_Window* window);

/**
 * Membersihkan seluruh sumber daya renderer GPU.
 * Fungsi ini melepas tautan window dari perangkat GPU dan menghancurkan
 * objek SDL_GPUDevice yang aktif.
 * 
 * @param window Pointer ke SDL_Window tempat frame digambar.
 */
void gl_render_cleanup(SDL_Window* window);

/**
 * Merender satu frame visual menggunakan SDL_GPU.
 * Fungsi ini membuat command buffer, mengambil tekstur swapchain window,
 * memulai render pass dengan warna latar gelap solid, lalu men-submit
 * perintah ke kartu grafis.
 * 
 * @param window Pointer ke SDL_Window tempat frame akan digambar.
 */
void gl_render_frame(SDL_Window* window);

#endif // GL_RENDER_H
