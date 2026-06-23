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

#endif // SQLITE_H
