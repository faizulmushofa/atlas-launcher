#include "sqlite.h"
#include "indexer.h"
#include <sqlite3.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include <string.h>

static sqlite3* db = NULL;
static char g_db_path[512] = "";

const char* db_get_path(void) {
    if (g_db_path[0] == '\0') {
        char* pref_path = SDL_GetPrefPath("Atlas", "SpotlightSearch");
        if (pref_path) {
            snprintf(g_db_path, sizeof(g_db_path), "%sspotlight.db", pref_path);
            SDL_free(pref_path);
        } else {
            // Fallback
            strncpy(g_db_path, "db/spotlight.db", sizeof(g_db_path) - 1);
            g_db_path[sizeof(g_db_path) - 1] = '\0';
        }
    }
    return g_db_path;
}
static int SDLCALL indexer_thread_func(void* data) {
    (void)data;
    indexer_run();
    return 0;
}

bool db_init(void) {
    const char* db_path = db_get_path();
    printf("[SQLite] Membuka database di: %s\n", db_path);

    int rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[SQLite] Gagal membuka database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        db = NULL;
        return false;
    }

    printf("[SQLite] Koneksi database berhasil! Versi: %s\n", sqlite3_libversion());

    // Aktifkan WAL (Write-Ahead Logging) mode untuk konkurensi multi-thread
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);

    // Buat tabel schema jika belum ada
    const char* sql = "CREATE TABLE IF NOT EXISTS items ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "name TEXT NOT NULL,"
                      "path TEXT NOT NULL,"
                      "type TEXT NOT NULL,"
                      "icon_handle TEXT,"
                      "platform TEXT NOT NULL"
                      ");";
    
    char* err_msg = NULL;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[SQLite] Gagal membuat skema tabel items: %s\n", err_msg);
        sqlite3_free(err_msg);
        db_close();
        return false;
    }

    // Buat tabel shortcuts jika belum ada
    const char* sql_shortcuts = "CREATE TABLE IF NOT EXISTS shortcuts ("
                                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                "name TEXT NOT NULL,"
                                "path TEXT NOT NULL,"
                                "type TEXT NOT NULL,"
                                "platform TEXT NOT NULL"
                                ");";
    rc = sqlite3_exec(db, sql_shortcuts, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[SQLite] Gagal membuat skema tabel shortcuts: %s\n", err_msg);
        sqlite3_free(err_msg);
        db_close();
        return false;
    }

    printf("[SQLite] Skema database berhasil diverifikasi/dibuat.\n");

    // Jalankan pemindaian direktori aplikasi di background thread setiap startup agar GUI tetap responsive
    SDL_Thread* thread = SDL_CreateThread(indexer_thread_func, "IndexerThread", NULL);
    if (thread) {
        SDL_DetachThread(thread);
    } else {
        // Fallback jika pembuatan thread gagal
        indexer_run();
    }

    return true;
}

void db_close(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
        printf("[SQLite] Koneksi database ditutup.\n");
    }
}

struct sqlite3* db_get_handle(void) {
    return db;
}

bool db_insert_item(const char* name, const char* path, const char* type, const char* platform) {
    if (!db) {
        fprintf(stderr, "[SQLite] Error: Mencoba insert item tetapi database belum aktif.\n");
        return false;
    }

    const char* sql = "INSERT INTO items (name, path, type, platform) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[SQLite] Gagal menyiapkan statement insert item: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, platform, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "[SQLite] Gagal menjalankan insert item: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool db_insert_shortcut(const char* name, const char* path, const char* type, const char* platform) {
    if (!db) {
        fprintf(stderr, "[SQLite] Error: Mencoba insert shortcut tetapi database belum aktif.\n");
        return false;
    }

    // Pastikan tidak menduplikasi shortcut yang sama
    if (db_is_shortcut(path)) return true;

    const char* sql = "INSERT INTO shortcuts (name, path, type, platform) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[SQLite] Gagal menyiapkan statement insert shortcut: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, platform, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "[SQLite] Gagal menjalankan insert shortcut: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool db_delete_shortcut(const char* path) {
    if (!db) {
        fprintf(stderr, "[SQLite] Error: Mencoba delete shortcut tetapi database belum aktif.\n");
        return false;
    }

    const char* sql = "DELETE FROM shortcuts WHERE path = ?;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[SQLite] Gagal menyiapkan statement delete shortcut: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "[SQLite] Gagal menjalankan delete shortcut: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool db_is_shortcut(const char* path) {
    if (!db) return false;

    const char* sql = "SELECT COUNT(*) FROM shortcuts WHERE path = ?;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[SQLite] Gagal menyiapkan statement check shortcut: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_TRANSIENT);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    return count > 0;
}
