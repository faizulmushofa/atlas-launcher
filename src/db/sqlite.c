#include "sqlite.h"
#include <sqlite3.h>
#include <stdio.h>

static sqlite3* db = NULL;

bool db_init(void) {
    int rc = sqlite3_open("spotlight.db", &db);
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
    return true;
}

void db_close(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
        printf("[SQLite] Koneksi database ditutup.\n");
    }
}
