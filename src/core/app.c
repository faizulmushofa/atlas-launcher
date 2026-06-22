#include "app.h"
#include "../render/gl_render.h"
#include <SDL3/SDL.h>
#include <stdio.h>

/** Pointer statik ke window utama aplikasi. */
static SDL_Window* app_window = NULL;

/** Status penanda apakah loop aplikasi sedang berjalan. */
static bool is_running = false;

static SDL_HitTestResult SDLCALL app_hit_test(SDL_Window* window, const SDL_Point* pt, void* data) {
    (void)window;
    (void)pt;
    (void)data;
    return SDL_HITTEST_DRAGGABLE;
}

/**
 * Menginisialisasi seluruh subsistem aplikasi.
 * Fungsi ini melakukan setup untuk subsistem video SDL3, membuat window aplikasi
 * utama, serta menginisialisasi modul renderer SDL_GPU.
 * 
 * @return true jika seluruh proses inisialisasi berhasil, false jika terjadi kegagalan.
 */
bool app_init(void) {
    // Menginisialisasi subsistem video SDL3 (mengembalikan true jika berhasil)
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Gagal menginisialisasi SDL: %s\n", SDL_GetError());
        return false;
    }

    // Membuat window SDL3 (dimensi 800x100, flag SDL_WINDOW_BORDERLESS agar window tidak memiliki bingkai/frame)
    app_window = SDL_CreateWindow(
        "Spotlight Search",
        800,
        100,
        SDL_WINDOW_BORDERLESS
    );

    if (!app_window) {
        printf("Gagal membuat SDL window: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // Daftarkan callback hit test agar window borderless bisa di-drag/tarik
    if (!SDL_SetWindowHitTest(app_window, app_hit_test, NULL)) {
        printf("Gagal mendaftarkan hit test window: %s\n", SDL_GetError());
    }

    // Menginisialisasi renderer SDL_GPU eksternal
    if (!gl_render_init(app_window)) {
        SDL_DestroyWindow(app_window);
        SDL_Quit();
        return false;
    }

    is_running = true;
    return true;
}

/**
 * Menjalankan loop utama (event loop) aplikasi.
 * Fungsi ini mengontrol jalannya aplikasi dengan terus-menerus mendengarkan input/event
 * dari sistem (seperti tombol ESC atau tombol tutup window) dan memicu penggambaran frame
 * menggunakan renderer SDL_GPU selama aplikasi dalam status berjalan.
 */
void app_run(void) {
    SDL_Event event;

    // Loop utama aplikasi
    while (is_running) {
        // Melakukan polling event SDL yang masuk
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                is_running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                // Menutup window/aplikasi jika tombol ESC ditekan (event.key.key pada SDL3)
                if (event.key.key == SDLK_ESCAPE) {
                    is_running = false;
                }
            }
        }

        // Memanggil fungsi menggambar frame dari modul renderer GPU
        gl_render_frame(app_window);
    }
}

/**
 * Melakukan cleanup dan membebaskan seluruh sumber daya aplikasi.
 * Fungsi ini dipanggil sebelum aplikasi berakhir untuk melepaskan window dari GPU,
 * menghancurkan perangkat GPU, menghancurkan window SDL3, dan menutup subsistem SDL3.
 */
void app_cleanup(void) {
    // Membersihkan renderer SDL_GPU
    gl_render_cleanup(app_window);

    // Hancurkan window SDL yang aktif
    if (app_window) {
        SDL_DestroyWindow(app_window);
        app_window = NULL;
    }

    // Menghentikan subsistem SDL
    SDL_Quit();
    printf("Aplikasi berhasil ditutup dengan bersih.\n");
}
