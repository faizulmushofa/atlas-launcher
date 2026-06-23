#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL.h>

/**
 * Mengaktifkan input teks SDL3 untuk window yang bersangkutan.
 * 
 * @param window Pointer ke SDL_Window utama.
 */
void input_init(SDL_Window* window);

/**
 * Memproses event input (key down dan text input) untuk memperbarui search query.
 * 
 * @param event Pointer ke SDL_Event yang tertangkap.
 */
void input_handle_event(SDL_Event* event);

#endif // INPUT_H
