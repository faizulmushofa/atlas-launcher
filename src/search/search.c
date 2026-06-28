#include "search.h"
#include "../core/state.h"
#include "../db/sqlite.h"
#include "../platform/detection.h"
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

void search_query(const char* query_text) {
    AppState* state = state_get();
    if (!state) return;

    sqlite3* db = db_get_handle();
    if (!db) {
        state->result_count = 0;
        state->selected_index = 0;
        return;
    }

    // Jika teks pencarian kosong, muat daftar shortcut/recent items
    if (!query_text || strlen(query_text) == 0) {
        const char* sql = "SELECT id, name, path, type, platform FROM shortcuts LIMIT 5;";
        sqlite3_stmt* stmt = NULL;
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            state->result_count = 0;
            state->selected_index = 0;
            return;
        }

        int count = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW && count < 5) {
            state->results[count].id = sqlite3_column_int(stmt, 0);
            
            const char* name = (const char*)sqlite3_column_text(stmt, 1);
            const char* path = (const char*)sqlite3_column_text(stmt, 2);
            const char* type = (const char*)sqlite3_column_text(stmt, 3);
            const char* platform = (const char*)sqlite3_column_text(stmt, 4);

            strncpy(state->results[count].name, name ? name : "", sizeof(state->results[count].name) - 1);
            state->results[count].name[sizeof(state->results[count].name) - 1] = '\0';

            strncpy(state->results[count].path, path ? path : "", sizeof(state->results[count].path) - 1);
            state->results[count].path[sizeof(state->results[count].path) - 1] = '\0';

            strncpy(state->results[count].type, type ? type : "", sizeof(state->results[count].type) - 1);
            state->results[count].type[sizeof(state->results[count].type) - 1] = '\0';

            strncpy(state->results[count].platform, platform ? platform : "", sizeof(state->results[count].platform) - 1);
            state->results[count].platform[sizeof(state->results[count].platform) - 1] = '\0';

            count++;
        }
        sqlite3_finalize(stmt);
        state->result_count = count;
        state->selected_index = 0;
        return;
    }

    // Persiapkan pola pencarian (e.g. %query%)
    char pattern[258];
    snprintf(pattern, sizeof(pattern), "%%%s%%", query_text);

    const char* sql = "SELECT id, name, path, type, platform FROM items "
                      "WHERE name LIKE ? AND (platform = ? OR platform = 'all') "
                      "LIMIT 5;";

    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[Search] Gagal menyiapkan query: %s\n", sqlite3_errmsg(db));
        state->result_count = 0;
        state->selected_index = 0;
        return;
    }

    // Bind parameter 1: pola pencarian
    sqlite3_bind_text(stmt, 1, pattern, -1, SQLITE_TRANSIENT);
    
    // Bind parameter 2: nama platform saat ini (e.g. macOS, Windows, Linux)
    sqlite3_bind_text(stmt, 2, platform_get_os_name(), -1, SQLITE_TRANSIENT);

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < 5) {
        state->results[count].id = sqlite3_column_int(stmt, 0);
        
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        const char* path = (const char*)sqlite3_column_text(stmt, 2);
        const char* type = (const char*)sqlite3_column_text(stmt, 3);
        const char* platform = (const char*)sqlite3_column_text(stmt, 4);

        strncpy(state->results[count].name, name ? name : "", sizeof(state->results[count].name) - 1);
        state->results[count].name[sizeof(state->results[count].name) - 1] = '\0';

        strncpy(state->results[count].path, path ? path : "", sizeof(state->results[count].path) - 1);
        state->results[count].path[sizeof(state->results[count].path) - 1] = '\0';

        strncpy(state->results[count].type, type ? type : "", sizeof(state->results[count].type) - 1);
        state->results[count].type[sizeof(state->results[count].type) - 1] = '\0';

        strncpy(state->results[count].platform, platform ? platform : "", sizeof(state->results[count].platform) - 1);
        state->results[count].platform[sizeof(state->results[count].platform) - 1] = '\0';

        count++;
    }

    sqlite3_finalize(stmt);

    state->result_count = count;
    state->selected_index = 0;
}
