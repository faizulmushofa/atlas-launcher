#include "app.h"
#include "state.h"
#include "../ui/input.h"
#include "../render/gl_render.h"
#include "../db/sqlite.h"
#include "../search/search.h"
#include <SDL3/SDL.h>
#include <stdio.h>

/** Pointer statik ke window utama aplikasi. */
static SDL_Window* app_window = NULL;

/** Status penanda apakah loop aplikasi sedang berjalan. */
static bool is_running = false;

static SDL_HitTestResult SDLCALL app_hit_test(SDL_Window* window, const SDL_Point* pt, void* data) {
    (void)window;
    (void)data;

    // Bagian dropdown hasil/pintasan (y >= 50) tidak boleh draggable agar bisa diklik
    if (pt->y >= 50) {
        return SDL_HITTEST_NORMAL;
    }

    // Bagian tombol hijau + di kanan search bar (x >= 740) tidak boleh draggable agar bisa diklik
    if (pt->x >= 740) {
        return SDL_HITTEST_NORMAL;
    }

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
    // Inisialisasi state aplikasi global
    state_init();

    // Menginisialisasi subsistem video SDL3 (mengembalikan true jika berhasil)
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Gagal menginisialisasi SDL: %s\n", SDL_GetError());
        return false;
    }

    // Membuat window SDL3 (dimensi 800x50, flag SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT agar window transparan tanpa bingkai)
    app_window = SDL_CreateWindow(
        "Spotlight Search",
        800,
        50,
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT
    );

    if (!app_window) {
        printf("Gagal membuat SDL window: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // Mengaktifkan mode text input di SDL3 untuk pengetikan pada window
    input_init(app_window);

    // Daftarkan callback hit test agar window borderless bisa di-drag/tarik
    if (!SDL_SetWindowHitTest(app_window, app_hit_test, NULL)) {
        printf("Gagal mendaftarkan hit test window: %s\n", SDL_GetError());
    }

    // Menginisialisasi renderer eksternal
    if (!gl_render_init(app_window)) {
        SDL_DestroyWindow(app_window);
        SDL_Quit();
        return false;
    }

    // Menginisialisasi koneksi database SQLite
    if (!db_init()) {
        gl_render_cleanup(app_window);
        SDL_DestroyWindow(app_window);
        SDL_Quit();
        return false;
    }

    // Muat daftar pintasan default saat startup
    search_query("");

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
            // Teruskan seluruh event input ke sistem pemrosesan input
            input_handle_event(&event);

            if (event.type == SDL_EVENT_QUIT) {
                is_running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                // Menutup window/aplikasi jika tombol ESC ditekan (event.key.key pada SDL3)
                if (event.key.key == SDLK_ESCAPE) {
                    is_running = false;
                }
            }
        }

        // Hitung tinggi target window secara dinamis berdasarkan hasil pencarian atau daftar pintasan
        int target_h = 50;
        AppState* state = state_get();
        if (state && state->result_count > 0) {
            target_h = 50 + (state->result_count * 40) + 10;
        }

        int curr_w = 800, curr_h = 50;
        SDL_GetWindowSize(app_window, &curr_w, &curr_h);
        if (curr_h != target_h) {
            SDL_SetWindowSize(app_window, 800, target_h);
        }

        // Memanggil fungsi menggambar frame dari modul renderer
        gl_render_frame(app_window);
    }
}

/**
 * Melakukan cleanup dan membebaskan seluruh sumber daya aplikasi.
 * Fungsi ini dipanggil sebelum aplikasi berakhir untuk melepaskan window dari GPU,
 * menghancurkan perangkat GPU, menghancurkan window SDL3, dan menutup subsistem SDL3.
 */
void app_cleanup(void) {
    // Menutup koneksi SQLite
    db_close();

    // Membersihkan renderer SDL_GPU
    gl_render_cleanup(app_window);
    printf("Membersihkan windows\n");

    // Hancurkan window SDL yang aktif
    if (app_window) {
        printf("Menghancurkan SDL aktif\n");
        SDL_DestroyWindow(app_window);
        app_window = NULL;
    }

    // Menghentikan subsistem SDL
    SDL_Quit();
    printf("Aplikasi berhasil ditutup dengan bersih.\n");
}

void app_request_exit(void) {
    is_running = false;
}

SDL_Window* app_get_window(void) {
    return app_window;
}


