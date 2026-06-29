#ifndef PE_PARSER_H
#define PE_PARSER_H

#include <stddef.h>

/**
 * Membaca data ikon RT_ICON dari file EXE/DLL Windows secara biner murni.
 * @param filepath Path absolut ke file EXE/DLL.
 * @param out_size Menampung ukuran data biner ikon yang diekstrak.
 * @param is_png Output boolean: bernilai 1 jika data yang diekstrak berformat PNG, 0 jika DIB.
 * @return Buffer berisi data gambar ikon (PNG atau DIB), harus dibebaskan oleh pemanggil menggunakan free().
 */
unsigned char* pe_extract_icon_data(const char* filepath, size_t* out_size, int* is_png);

/**
 * Mendecode data DIB (Device Independent Bitmap) menjadi piksel mentah RGBA32.
 * @param dib_data Buffer data DIB.
 * @param dib_size Ukuran buffer DIB.
 * @param out_w Menampung lebar gambar hasil decode.
 * @param out_h Menampung tinggi gambar hasil decode.
 * @return Buffer piksel RGBA32 (lebar * tinggi * 4 bytes), harus dibebaskan menggunakan free().
 */
unsigned char* decode_dib_to_rgba(const unsigned char* dib_data, size_t dib_size, int* out_w, int* out_h);

#endif
