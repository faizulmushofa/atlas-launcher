#ifndef SQLITE_H
#define SQLITE_H

#include <stdbool.h>

/**
 * Inisialisasi koneksi database SQLite.
 * Membuka koneksi ke database/spotlight.db dan memastikan skema tabel items terbentuk.
 * @return true jika berhasil, false jika gagal.
 */
bool db_init(void);

/**
 * Menutup koneksi database SQLite yang terbuka.
 */
void db_close(void);

/**
 * Mendapatkan pointer ke handle database SQLite yang aktif.
 * @return Pointer ke struct sqlite3 atau NULL.
 */
struct sqlite3;
struct sqlite3* db_get_handle(void);

/**
 * Memasukkan data pencarian aplikasi baru ke dalam tabel items.
 * @return true jika sukses, false jika gagal.
 */
bool db_insert_item(const char* name, const char* path, const char* type, const char* platform);

#endif // SQLITE_H
