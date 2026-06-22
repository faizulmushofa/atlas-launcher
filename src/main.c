#include "core/app.h"

/**
 * Titik masuk utama program (entry point).
 * Fungsi ini menginisialisasi modul aplikasi, memulai loop utama program,
 * dan memastikan cleanup sumber daya dilakukan saat keluar.
 * 
 * @param argc Jumlah argumen baris perintah.
 * @param argv Larik string dari argumen baris perintah.
 * @return Nilai status keluar program (0 jika sukses, 1 jika terjadi kegagalan inisialisasi).
 */
int main(int argc, char* argv[]) {
    if (!app_init()) {
        return 1;
    }

    app_run();

    app_cleanup();

    return 0;
}