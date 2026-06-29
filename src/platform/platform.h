#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>

/**
 * Membuka aplikasi atau berkas pada path yang diberikan.
 * @param path Path absolut menuju aplikasi atau berkas (misal /Applications/Safari.app atau C:\Program Files\...)
 * @return true jika berhasil membuka, false jika gagal.
 */
bool platform_open_app(const char* path);

#endif // PLATFORM_H
