#include "icon_cache.h"
#include "icon.h"
#include <string.h>
#include <stdlib.h>

#define MAX_CACHED_ICONS 64

typedef struct {
    char path[256];
    SDL_Texture* texture;
} CachedIcon;

static CachedIcon g_icon_cache[MAX_CACHED_ICONS];
static int g_cache_count = 0;
static int g_insert_idx = 0;

void icon_cache_init(void) {
    memset(g_icon_cache, 0, sizeof(g_icon_cache));
    g_cache_count = 0;
    g_insert_idx = 0;
}

SDL_Texture* icon_cache_get(SDL_Renderer* renderer, const char* app_path) {
    if (!app_path || strlen(app_path) == 0) return NULL;

    // 1. Cari di cache
    for (int i = 0; i < g_cache_count; i++) {
        if (strcmp(g_icon_cache[i].path, app_path) == 0) {
            return g_icon_cache[i].texture;
        }
    }

    // 2. Jika tidak ada, load secara native
    SDL_Texture* texture = icon_get_native_texture(renderer, app_path);
    if (!texture) return NULL;

    // 3. Masukkan ke dalam cache
    if (g_cache_count < MAX_CACHED_ICONS) {
        // Masukkan di slot baru
        strncpy(g_icon_cache[g_cache_count].path, app_path, sizeof(g_icon_cache[g_cache_count].path) - 1);
        g_icon_cache[g_cache_count].path[sizeof(g_icon_cache[g_cache_count].path) - 1] = '\0';
        g_icon_cache[g_cache_count].texture = texture;
        g_cache_count++;
    } else {
        // Cache penuh: Timpa data paling tua (FIFO rolling index)
        int replace_idx = g_insert_idx;
        if (g_icon_cache[replace_idx].texture) {
            SDL_DestroyTexture(g_icon_cache[replace_idx].texture);
        }
        strncpy(g_icon_cache[replace_idx].path, app_path, sizeof(g_icon_cache[replace_idx].path) - 1);
        g_icon_cache[replace_idx].path[sizeof(g_icon_cache[replace_idx].path) - 1] = '\0';
        g_icon_cache[replace_idx].texture = texture;
        g_insert_idx = (g_insert_idx + 1) % MAX_CACHED_ICONS;
    }

    return texture;
}

void icon_cache_cleanup(void) {
    for (int i = 0; i < g_cache_count; i++) {
        if (g_icon_cache[i].texture) {
            SDL_DestroyTexture(g_icon_cache[i].texture);
            g_icon_cache[i].texture = NULL;
        }
        g_icon_cache[i].path[0] = '\0';
    }
    g_cache_count = 0;
    g_insert_idx = 0;
}
