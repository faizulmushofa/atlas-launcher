#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#define MAX_QUERY_LEN 255

typedef struct {
    char search_query[MAX_QUERY_LEN + 1];
    int query_len;
    int cursor_position;
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
