#include "core/app.h"
#include "platform/detection.h"
#include <stdio.h>

/**
 * Titik masuk utama program (entry point).
 * Fungsi ini menginisialisasi modul aplikasi, memulai loop utama program,
 * dan memastikan cleanup sumber daya dilakukan saat keluar.
 * @param argc Jumlah argumen baris perintah.
 * @param argv Larik string dari argumen baris perintah.
 * @return Nilai status keluar program (0 jika sukses, 1 jika terjadi kegagalan inisialisasi).
 */
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    
    if (!app_init()) {
        return 1;
    }

    printf("Menjalankan Spotlight Search di OS: %s\n", platform_get_os_name());

    app_run();

    app_cleanup();

    return 0;
}