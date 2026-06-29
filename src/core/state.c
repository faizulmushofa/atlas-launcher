#include "state.h"
#include <string.h>

/** Global static instance of AppState */
static AppState g_app_state;

AppState* state_get(void) {
    return &g_app_state;
}

void state_init(void) {
    memset(&g_app_state, 0, sizeof(AppState));
    g_app_state.cursor_position = 0;
    g_app_state.query_len = 0;
    g_app_state.search_query[0] = '\0';
}
