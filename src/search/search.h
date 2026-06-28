#ifndef SEARCH_H
#define SEARCH_H

/**
 * Melakukan kueri pencarian ke SQLite berdasarkan kata kunci yang diketik.
 * Hasilnya akan disimpan langsung ke AppState global.
 * @param query_text Teks pencarian saat ini.
 */
void search_query(const char* query_text);

#endif // SEARCH_H
