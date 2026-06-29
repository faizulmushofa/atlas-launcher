#ifndef ICNS_PARSER_H
#define ICNS_PARSER_H

#include <stddef.h>

/**
 * Mencari berkas .icns di subfolder Contents/Resources/ dari app_bundle_path
 * dan mengekstrak blok ikon PNG dengan resolusi tinggi.
 * @param app_bundle_path Jalur absolut ke folder .app.
 * @param out_size Menampung ukuran data biner PNG yang diekstrak.
 * @return Buffer berisi data gambar PNG, harus dibebaskan oleh pemanggil menggunakan free().
 */
unsigned char* macos_extract_bundle_icon(const char* app_bundle_path, size_t* out_size);

#endif
