#ifndef RANKING_H
#define RANKING_H

#include "../core/state.h"

/**
 * Mengurutkan hasil pencarian berdasarkan tingkat relevansinya dengan kueri.
 * @param results Array SearchResult hasil mentah dari database.
 * @param count Jumlah hasil mentah.
 * @param query Kueri pencarian yang dimasukkan pengguna.
 * @return Jumlah hasil setelah diurutkan (maksimal 5).
 */
int ranking_sort_results(SearchResult* results, int count, const char* query);

#endif // RANKING_H
