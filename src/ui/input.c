#include "input.h"
#include "../core/state.h"
#include <string.h>

void input_init(SDL_Window* window) {
    // Memulai system text input SDL3 agar event SDL_EVENT_TEXT_INPUT dapat dipicu
    SDL_StartTextInput(window);
}

void input_handle_event(SDL_Event* event) {
    AppState* state = state_get();
    if (!state) return;

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
}
