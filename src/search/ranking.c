#include "ranking.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

// Helper untuk konversi karakter ke lowercase
static char to_lower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

// Cek prefix case-insensitive
static bool starts_with_case(const char* str, const char* prefix) {
    while (*prefix) {
        if (to_lower(*str) != to_lower(*prefix)) {
            return false;
        }
        str++;
        prefix++;
    }
    return true;
}

// Case-insensitive strstr
static const char* stristr(const char* haystack, const char* needle) {
    if (!*needle) return haystack;
    for (; *haystack; haystack++) {
        if (to_lower(*haystack) == to_lower(*needle)) {
            const char* h = haystack;
            const char* n = needle;
            while (*h && *n && to_lower(*h) == to_lower(*n)) {
                h++;
                n++;
            }
            if (!*n) return haystack;
        }
    }
    return NULL;
}

// Hitung skor kecocokan
static int calculate_score(const char* name, const char* query) {
    if (!name || !query) return 0;
    
    // 1. Cocok Sempurna (Exact Match) - abaikan kapitalisasi
    int name_len = strlen(name);
    int query_len = strlen(query);
    if (name_len == query_len && starts_with_case(name, query)) {
        return 1000;
    }

    // 2. Cocok Awalan (Prefix Match) - diawali oleh kueri
    if (starts_with_case(name, query)) {
        // Makin pendek nama, skor makin tinggi (Safari dibanding SafariLauncher)
        return 500 - name_len;
    }

    // 3. Cocok Awal Kata (Word Boundary Prefix Match) - misal kueri "store" cocok dengan "Store" di "App Store"
    for (int i = 0; i < name_len; i++) {
        // Awal kata didefinisikan jika karakter sebelumnya adalah spasi atau karakter khusus
        if (i > 0 && (name[i - 1] == ' ' || name[i - 1] == '-' || name[i - 1] == '_')) {
            if (starts_with_case(&name[i], query)) {
                return 300 - i; // Makin di depan letak kata pencocokan, skor makin tinggi
            }
        }
    }

    // 4. Cocok Substring
    const char* match = stristr(name, query);
    if (match) {
        int pos = (int)(match - name);
        return 100 - pos; // Makin di depan posisinya, skor makin tinggi
    }

    return 0;
}

// Struktur perantara untuk sorting dengan qsort
typedef struct {
    SearchResult item;
    int score;
} RankedItem;

static int compare_ranked_items(const void* a, const void* b) {
    const RankedItem* itemA = (const RankedItem*)a;
    const RankedItem* itemB = (const RankedItem*)b;
    // Urutkan menurun (descending score)
    return itemB->score - itemA->score;
}

int ranking_sort_results(SearchResult* results, int count, const char* query) {
    if (count <= 0 || !query || strlen(query) == 0) {
        return count > 5 ? 5 : count; // Batasi maksimal 5 jika tidak disortir
    }

    RankedItem* ranked = (RankedItem*)malloc(sizeof(RankedItem) * count);
    if (!ranked) return count > 5 ? 5 : count;

    // Hitung skor untuk masing-masing hasil
    for (int i = 0; i < count; i++) {
        ranked[i].item = results[i];
        ranked[i].score = calculate_score(results[i].name, query);
    }

    // Urutkan menggunakan qsort standar C
    qsort(ranked, count, sizeof(RankedItem), compare_ranked_items);

    // Salin kembali hasil yang sudah diurutkan (maksimal 5 item terbaik)
    int final_count = count > 5 ? 5 : count;
    for (int i = 0; i < final_count; i++) {
        results[i] = ranked[i].item;
    }

    free(ranked);
    return final_count;
}
