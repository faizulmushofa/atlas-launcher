#ifndef GENERIC_ICON_H
#define GENERIC_ICON_H

#include <SDL3/SDL.h>

/**
 * Membuat SDL_Surface untuk ikon generik berdasarkan ekstensi file atau tipe "folder".
 * @param ext_or_type Ekstensi file (misal "txt", "pdf", "zip") atau tipe "folder", "app".
 * @return SDL_Surface* berisi gambar ikon, harus dibebaskan oleh pemanggil menggunakan SDL_DestroySurface.
 */
SDL_Surface* generic_icon_create_surface(const char* ext_or_type);

#endif
