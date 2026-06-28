#include "input.h"
#include "../core/app.h"
#include "../core/state.h"
#include "../platform/platform.h"
#include "../search/search.h"
#include "../db/sqlite.h"
#include "../platform/detection.h"
#include "ui.h"
#include <SDL3/SDL_dialog.h>
#include <stdio.h>
#include <string.h>

static void SDLCALL on_file_selected(void* userdata, const char* const* filelist, int filter) {
    (void)userdata;
    (void)filter;
    if (filelist && filelist[0]) {
        const char* path = filelist[0];
        
        // Dapatkan nama file/aplikasi dari path
        const char* filename = path;
        const char* slash1 = strrchr(path, '/');
        const char* slash2 = strrchr(path, '\\');
        const char* slash = slash1 > slash2 ? slash1 : slash2;
        if (slash) {
            filename = slash + 1;
        }

        // Jika berakhiran .app di macOS, hilangkan ekstensinya
        char clean_name[128];
        strncpy(clean_name, filename, sizeof(clean_name) - 1);
        clean_name[sizeof(clean_name) - 1] = '\0';
        size_t len = strlen(clean_name);
        if (len > 4 && strcmp(clean_name + len - 4, ".app") == 0) {
            clean_name[len - 4] = '\0';
        }

        // Tentukan tipe (app atau file)
        const char* type = "file";
        if (len > 4 && strcmp(filename + len - 4, ".app") == 0) {
            type = "app";
        }
#ifdef _WIN32
        else if (len > 4 && _stricmp(filename + len - 4, ".lnk") == 0) {
            type = "app";
            clean_name[len - 4] = '\0';
        }
        else if (len > 4 && _stricmp(filename + len - 4, ".exe") == 0) {
            type = "app";
            clean_name[len - 4] = '\0';
        }
#endif

        // Tambah ke database shortcuts
        db_insert_shortcut(clean_name, path, type, platform_get_os_name());
        
        // Paksa render ulang UI
        ui_invalidate_cache();
        
        // Reload shortcut di list (kueri kosong)
        AppState* state = state_get();
        if (state && state->query_len == 0) {
            search_query("");
        }
    }
}

void input_init(SDL_Window *window) {
  // Memulai system text input SDL3 agar event SDL_EVENT_TEXT_INPUT dapat dipicu
  SDL_StartTextInput(window);
}

void input_handle_event(SDL_Event *event) {
  AppState *state = state_get();
  if (!state)
    return;

  // Simpan salinan query lama untuk mendeteksi perubahan
  char prev_query[256];
  strncpy(prev_query, state->search_query, sizeof(prev_query) - 1);
  prev_query[sizeof(prev_query) - 1] = '\0';

  if (event->type == SDL_EVENT_TEXT_INPUT) {
    const char *text = event->text.text;
    size_t text_len = strlen(text);

    // Pastikan tidak melebihi batas buffer
    if (state->query_len + text_len <= MAX_QUERY_LEN) {
      // Geser karakter di kanan kursor ke kanan
      memmove(&state->search_query[state->cursor_position + text_len],
              &state->search_query[state->cursor_position],
              state->query_len - state->cursor_position + 1);

      // Masukkan teks baru pada posisi kursor
      memcpy(&state->search_query[state->cursor_position], text, text_len);

      state->cursor_position += text_len;
      state->query_len += text_len;
    }
  } else if (event->type == SDL_EVENT_KEY_DOWN) {
    SDL_Keycode key = event->key.key;
    Uint16 mod = event->key.mod;

    // Penanganan tombol Backspace
    if (key == SDLK_BACKSPACE) {
      if (state->cursor_position > 0) {
        // Geser karakter dari posisi kursor ke kiri
        memmove(&state->search_query[state->cursor_position - 1],
                &state->search_query[state->cursor_position],
                state->query_len - state->cursor_position + 1);

        state->cursor_position--;
        state->query_len--;
      }
    }
    // Penanganan tombol Delete
    else if (key == SDLK_DELETE) {
      if (state->cursor_position < state->query_len) {
        memmove(&state->search_query[state->cursor_position],
                &state->search_query[state->cursor_position + 1],
                state->query_len - state->cursor_position);

        state->query_len--;
      }
    }
    // Navigasi Kursor ke Kiri
    else if (key == SDLK_LEFT) {
      if (state->cursor_position > 0) {
        state->cursor_position--;
      }
    }
    // Navigasi Kursor ke Kanan
    else if (key == SDLK_RIGHT) {
      if (state->cursor_position < state->query_len) {
        state->cursor_position++;
      }
    }
    // Navigasi Baris Pilihan ke Bawah
    else if (key == SDLK_DOWN) {
      if (state->result_count > 0) {
        state->selected_index =
            (state->selected_index + 1) % state->result_count;
      }
    }
    // Navigasi Baris Pilihan ke Atas
    else if (key == SDLK_UP) {
      if (state->result_count > 0) {
        state->selected_index =
            (state->selected_index - 1 + state->result_count) %
            state->result_count;
      }
    }
    // Eksekusi Pilihan dengan Tombol Enter
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
      if (state->result_count > 0 && state->selected_index >= 0 &&
          state->selected_index < state->result_count) {
        const char *path = state->results[state->selected_index].path;
        printf("[Launcher] Mengeksekusi: %s (%s)\n",
               state->results[state->selected_index].name, path);
        if (platform_open_app(path)) {
          app_request_exit();
        } else {
          printf("[Launcher] Gagal membuka aplikasi: %s\n", path);
        }
      }
    }
    // Clipboard Paste (Cmd+V di Mac, Ctrl+V di Windows/Linux)
#ifdef __APPLE__
    else if (key == SDLK_V && (mod & SDL_KMOD_GUI)) {
#else
    else if (key == SDLK_V && (mod & SDL_KMOD_CTRL)) {
#endif
      if (SDL_HasClipboardText()) {
        char *clipboard = SDL_GetClipboardText();
        if (clipboard) {
          size_t text_len = strlen(clipboard);
          // Pastikan tidak melebihi batas buffer dan abaikan karakter newline
          // jika ada
          if (state->query_len + text_len <= MAX_QUERY_LEN) {
            memmove(&state->search_query[state->cursor_position + text_len],
                    &state->search_query[state->cursor_position],
                    state->query_len - state->cursor_position + 1);

            memcpy(&state->search_query[state->cursor_position], clipboard,
                   text_len);

            state->cursor_position += text_len;
            state->query_len += text_len;
          }
          SDL_free(clipboard);
        }
      }
    }
  } else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    float cx = event->button.x;
    float cy = event->button.y;
    printf("[Launcher] Klik mouse terdeteksi di: x=%.1f, y=%.1f\n", cx, cy);

    if (cy < 50.0f) {
      // Klik di area search bar
      // Tombol hijau (+) berada di x = [750, 790], y = [5, 45] px (Click target 40x40 px)
      if (cx >= 750.0f && cx <= 790.0f && cy >= 5.0f && cy <= 45.0f) {
        printf("[Launcher] Tombol hijau + diklik! Membuka dialog pemilih file...\n");
        SDL_Window* win = app_get_window();
        SDL_ShowOpenFileDialog(on_file_selected, NULL, win, NULL, 0, NULL, false);
      }
    } else {
      // Klik di area dropdown hasil / pintasan (Y >= 50)
      if (state->result_count > 0) {
        for (int i = 0; i < state->result_count; i++) {
          int row_y = 50 + i * 40;

          // Cek apakah klik berada di area baris i
          if (cy >= (float)row_y && cy < (float)(row_y + 40)) {
            if (state->query_len == 0) {
              // Cek apakah klik berada di tombol merah (-)
              if (cx >= 750.0f && cx <= 790.0f) {
                const SearchResult* item = &state->results[i];
                db_delete_shortcut(item->path);
                ui_invalidate_cache();
                search_query(""); // Reload pintasan
              } else {
                // Klik pada baris pintasan: eksekusi aplikasi
                const char* path = state->results[i].path;
                printf("[Launcher] Mengeksekusi dari klik: %s (%s)\n", state->results[i].name, path);
                if (platform_open_app(path)) {
                  app_request_exit();
                } else {
                  printf("[Launcher] Gagal membuka aplikasi: %s\n", path);
                }
              }
            } else {
              // Di list hasil pencarian normal (tidak ada tombol hijau di kanan baris): klik langsung mengeksekusi
              const char* path = state->results[i].path;
              printf("[Launcher] Mengeksekusi dari klik: %s (%s)\n", state->results[i].name, path);
              if (platform_open_app(path)) {
                app_request_exit();
              } else {
                printf("[Launcher] Gagal membuka aplikasi: %s\n", path);
              }
            }
            break;
          }
        }
      }
    }
  }

  // Jika teks pencarian berubah, jalankan kueri pencarian baru
  if (strcmp(prev_query, state->search_query) != 0) {
    search_query(state->search_query);
  }
}
