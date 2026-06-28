#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#define MAX_QUERY_LEN 255

typedef struct {
    int id;
    char name[128];
    char path[256];
    char type[32];
    char platform[32];
} SearchResult;

typedef struct {
    char search_query[MAX_QUERY_LEN + 1];
    int query_len;
    int cursor_position;
    
    // Hasil pencarian aktif
    SearchResult results[5];
    int result_count;
    int selected_index;
} AppState;

/**
 * Mendapatkan instance global dari state aplikasi.
 * 
 * @return Pointer ke AppState global.
 */
AppState* state_get(void);

/**
 * Menginisialisasi state aplikasi dengan nilai default.
 */
void state_init(void);

#endif // STATE_H
