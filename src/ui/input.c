#include "input.h"
#include "../core/state.h"
#include "../search/search.h"
#include <string.h>
#include <stdio.h>

void input_init(SDL_Window* window) {
    // Memulai system text input SDL3 agar event SDL_EVENT_TEXT_INPUT dapat dipicu
    SDL_StartTextInput(window);
}

void input_handle_event(SDL_Event* event) {
    AppState* state = state_get();
    if (!state) return;

    // Simpan salinan query lama untuk mendeteksi perubahan
    char prev_query[256];
    strncpy(prev_query, state->search_query, sizeof(prev_query) - 1);
    prev_query[sizeof(prev_query) - 1] = '\0';

    if (event->type == SDL_EVENT_TEXT_INPUT) {
        const char* text = event->text.text;
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
    } 
    else if (event->type == SDL_EVENT_KEY_DOWN) {
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
                state->selected_index = (state->selected_index + 1) % state->result_count;
            }
        }
        // Navigasi Baris Pilihan ke Atas
        else if (key == SDLK_UP) {
            if (state->result_count > 0) {
                state->selected_index = (state->selected_index - 1 + state->result_count) % state->result_count;
            }
        }
        // Eksekusi Pilihan dengan Tombol Enter
        else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            if (state->result_count > 0 && state->selected_index >= 0 && state->selected_index < state->result_count) {
                const char* path = state->results[state->selected_index].path;
                printf("[Launcher] Mengeksekusi: %s (%s)\n", state->results[state->selected_index].name, path);
                SDL_OpenURL(path);
            }
        }
        // Clipboard Paste (Cmd+V di Mac, Ctrl+V di Windows/Linux)
#ifdef __APPLE__
        else if (key == SDLK_V && (mod & SDL_KMOD_GUI)) {
#else
        else if (key == SDLK_V && (mod & SDL_KMOD_CTRL)) {
#endif
            if (SDL_HasClipboardText()) {
                char* clipboard = SDL_GetClipboardText();
                if (clipboard) {
                    size_t text_len = strlen(clipboard);
                    // Pastikan tidak melebihi batas buffer dan abaikan karakter newline jika ada
                    if (state->query_len + text_len <= MAX_QUERY_LEN) {
                        memmove(&state->search_query[state->cursor_position + text_len], 
                                &state->search_query[state->cursor_position], 
                                state->query_len - state->cursor_position + 1);
                        
                        memcpy(&state->search_query[state->cursor_position], clipboard, text_len);
                        
                        state->cursor_position += text_len;
                        state->query_len += text_len;
                    }
                    SDL_free(clipboard);
                }
            }
        }
    }

    // Jika teks pencarian berubah, jalankan kueri pencarian baru
    if (strcmp(prev_query, state->search_query) != 0) {
        search_query(state->search_query);
    }
}
