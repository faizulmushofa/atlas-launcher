#ifndef PLATFORM_FS_H
#define PLATFORM_FS_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Mendapatkan jalur folder home pengguna secara lintas platform.
 * @param out_path Buffer untuk menyimpan path home.
 * @param max_len Ukuran maksimum buffer.
 * @return true jika berhasil, false jika gagal.
 */
bool fs_get_user_home(char* out_path, size_t max_len);

#endif // PLATFORM_FS_H
