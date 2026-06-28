#include "sqlite.h"
#include "indexer.h"
#include <sqlite3.h>
#include <stdio.h>

static sqlite3* db = NULL;

bool db_init(void) {
    int rc = sqlite3_open("db/spotlight.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[SQLite] Gagal membuka database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        db = NULL;
        return false;
    }

    printf("[SQLite] Koneksi database berhasil! Versi: %s\n", sqlite3_libversion());

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
        fprintf(stderr, "[SQLite] Gagal membuat skema tabel: %s\n", err_msg);
        sqlite3_free(err_msg);
        db_close();
        return false;
    }

    printf("[SQLite] Skema database berhasil diverifikasi/dibuat.\n");

    // Jalankan pemindaian direktori aplikasi setiap startup
    indexer_run();

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
    if (!db) return false;

    const char* sql = "INSERT INTO items (name, path, type, platform) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, platform, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}
