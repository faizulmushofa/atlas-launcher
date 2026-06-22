#ifndef APP_H
#define APP_H

#include <stdbool.h>

/**
 * Menginisialisasi seluruh subsistem aplikasi.
 * Fungsi ini melakukan setup untuk subsistem video SDL2, menetapkan konfigurasi
 * atribut OpenGL, membuat window aplikasi utama, serta menginisialisasi modul renderer OpenGL.
 * 
 * @return true jika seluruh proses inisialisasi berhasil, false jika terjadi kegagalan.
 */
bool app_init(void);

/**
 * Menjalankan loop utama (event loop) aplikasi.
 * Fungsi ini mengontrol jalannya aplikasi dengan terus-menerus mendengarkan input/event
 * dari sistem (seperti tombol ESC atau tombol tutup window) dan memicu penggambaran frame
 * menggunakan renderer OpenGL selama aplikasi dalam status berjalan.
 */
void app_run(void);

/**
 * Melakukan cleanup dan membebaskan seluruh sumber daya aplikasi.
 * Fungsi ini dipanggil sebelum aplikasi berakhir untuk menghancurkan OpenGL context,
 * menghancurkan window SDL2 yang aktif, dan menutup subsistem SDL2 secara bersih.
 */
void app_cleanup(void);

#endif // APP_H
