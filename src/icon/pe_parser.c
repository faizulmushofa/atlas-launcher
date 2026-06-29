#include "pe_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define READ_LE16(p) ((uint16_t)(((p)[1] << 8) | (p)[0]))
#define READ_LE32(p) ((uint32_t)(((p)[3] << 24) | ((p)[2] << 16) | ((p)[1] << 8) | (p)[0]))

static uint32_t rva_to_file_offset(uint32_t rva, uint32_t num_sections, const unsigned char* sec_table, uint32_t file_size) {
    for (uint16_t i = 0; i < num_sections; i++) {
        const unsigned char* sec = sec_table + i * 40;
        uint32_t vaddr = READ_LE32(sec + 12);
        uint32_t vsize = READ_LE32(sec + 8);
        uint32_t raw_ptr = READ_LE32(sec + 20);
        if (rva >= vaddr && rva < vaddr + vsize) {
            uint32_t offset = rva - vaddr + raw_ptr;
            if (offset < file_size) return offset;
        }
    }
    return 0;
}

static uint32_t find_resource_entry(const unsigned char* buf, uint32_t res_root, uint32_t dir_offset, uint32_t target_id, int is_type_dir, uint32_t file_size) {
    if (dir_offset + 16 > file_size) return 0;
    const unsigned char* dir = buf + res_root + dir_offset;
    uint16_t num_named = READ_LE16(dir + 12);
    uint16_t num_id = READ_LE16(dir + 14);
    uint16_t total = num_named + num_id;
    
    const unsigned char* entries = dir + 16;
    for (uint16_t i = 0; i < total; i++) {
        uint32_t entry_offset = dir_offset + 16 + i * 8;
        if (res_root + entry_offset + 8 > file_size) return 0;
        const unsigned char* entry = buf + res_root + entry_offset;
        uint32_t name_or_id = READ_LE32(entry);
        uint32_t offset_to_data = READ_LE32(entry + 4);
        
        int match = 0;
        if (is_type_dir) {
            if ((name_or_id & 0x80000000) == 0 && name_or_id == target_id) {
                match = 1;
            }
        } else {
            if (target_id == 0xFFFFFFFF) {
                match = 1;
            } else if ((name_or_id & 0x80000000) == 0 && name_or_id == target_id) {
                match = 1;
            }
        }
        
        if (match) {
            return offset_to_data;
        }
    }
    return 0;
}

unsigned char* pe_extract_icon_data(const char* filepath, size_t* out_size, int* is_png) {
#ifdef _WIN32
    // Konversi path UTF-8 ke UTF-16 wide-character agar aman di Windows
    int wlen = MultiByteToWideChar(CP_UTF8, 0, filepath, -1, NULL, 0);
    FILE* f = NULL;
    if (wlen > 0) {
        wchar_t* wpath = (wchar_t*)malloc(wlen * sizeof(wchar_t));
        if (wpath) {
            MultiByteToWideChar(CP_UTF8, 0, filepath, -1, wpath, wlen);
            f = _wfopen(wpath, L"rb");
            free(wpath);
        }
    }
#else
    FILE* f = fopen(filepath, "rb");
#endif
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Batasan ukuran file executable (maksimal 100MB untuk mencegah konsumsi RAM berlebih)
    if (file_size <= 64 || file_size > 100 * 1024 * 1024) {
        fclose(f);
        return NULL;
    }
    
    unsigned char* buf = (unsigned char*)malloc(file_size);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    
    if (fread(buf, 1, file_size, f) != (size_t)file_size) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    
    // 1. Validasi DOS MZ header
    if (READ_LE16(buf) != 0x5A4D) {
        free(buf);
        return NULL;
    }
    
    uint32_t pe_offset = READ_LE32(buf + 60);
    if (pe_offset + 24 > (uint32_t)file_size) {
        free(buf);
        return NULL;
    }
    
    // 2. Validasi PE signature
    if (READ_LE32(buf + pe_offset) != 0x00004550) {
        free(buf);
        return NULL;
    }
    
    uint16_t num_sections = READ_LE16(buf + pe_offset + 4 + 2);
    uint16_t size_opt_hdr = READ_LE16(buf + pe_offset + 4 + 16);
    uint16_t opt_magic = READ_LE16(buf + pe_offset + 24);
    
    uint32_t data_dir_offset = 0;
    if (opt_magic == 0x10B) { // PE32 (32-bit)
        data_dir_offset = pe_offset + 24 + 96;
    } else if (opt_magic == 0x20B) { // PE32+ (64-bit)
        data_dir_offset = pe_offset + 24 + 112;
    } else {
        free(buf);
        return NULL;
    }
    
    if (data_dir_offset + 24 > (uint32_t)file_size) {
        free(buf);
        return NULL;
    }
    
    // Ambil Resource Directory RVA (indeks 2)
    uint32_t res_rva = READ_LE32(buf + data_dir_offset + 16);
    uint32_t res_size = READ_LE32(buf + data_dir_offset + 20);
    if (res_rva == 0 || res_size == 0) {
        free(buf);
        return NULL;
    }
    
    // Peta Section Table
    uint32_t sec_table_offset = pe_offset + 24 + size_opt_hdr;
    if (sec_table_offset + num_sections * 40 > (uint32_t)file_size) {
        free(buf);
        return NULL;
    }
    
    const unsigned char* sec_table = buf + sec_table_offset;
    uint32_t res_root = rva_to_file_offset(res_rva, num_sections, sec_table, file_size);
    if (res_root == 0) {
        free(buf);
        return NULL;
    }
    
    // 3. Cari RT_GROUP_ICON (Type ID: 14)
    uint32_t group_dir_offset = find_resource_entry(buf, res_root, 0, 14, 1, file_size);
    if ((group_dir_offset & 0x80000000) == 0) {
        free(buf);
        return NULL;
    }
    group_dir_offset &= 0x7FFFFFFF;
    
    // Masuk ke Name ID subdirectory (RT_GROUP_ICON) -> pilih yang pertama
    uint32_t group_lang_offset = find_resource_entry(buf, res_root, group_dir_offset, 0xFFFFFFFF, 0, file_size);
    if ((group_lang_offset & 0x80000000) == 0) {
        free(buf);
        return NULL;
    }
    group_lang_offset &= 0x7FFFFFFF;
    
    // Masuk ke Language subdirectory -> dapatkan data entry offset
    uint32_t group_data_entry_offset = find_resource_entry(buf, res_root, group_lang_offset, 0xFFFFFFFF, 0, file_size);
    if (group_data_entry_offset & 0x80000000) {
        free(buf);
        return NULL;
    }
    
    uint32_t group_entry_file = res_root + group_data_entry_offset;
    if (group_entry_file + 16 > (uint32_t)file_size) {
        free(buf);
        return NULL;
    }
    
    uint32_t group_rva = READ_LE32(buf + group_entry_file);
    uint32_t group_size = READ_LE32(buf + group_entry_file + 4);
    uint32_t group_file_offset = rva_to_file_offset(group_rva, num_sections, sec_table, file_size);
    if (group_file_offset == 0 || group_file_offset + group_size > (uint32_t)file_size) {
        free(buf);
        return NULL;
    }
    
    const unsigned char* group_data = buf + group_file_offset;
    if (group_size < 6) {
        free(buf);
        return NULL;
    }
    
    uint16_t count = READ_LE16(group_data + 4);
    if (6 + count * 14 > group_size) {
        free(buf);
        return NULL;
    }
    
    // 4. Pilih ikon dengan resolusi dan bit-depth terbaik (scoring)
    int best_index = -1;
    int best_score = -1;
    for (int i = 0; i < count; i++) {
        const unsigned char* entry = group_data + 6 + i * 14;
        int w = entry[0];
        int h = entry[1];
        int bit_count = READ_LE16(entry + 6);
        
        if (w == 0) w = 256;
        if (h == 0) h = 256;
        
        int score = 0;
        if (w == 32) score += 100;
        else if (w == 48) score += 90;
        else if (w == 64) score += 80;
        else if (w == 256) score += 70;
        else score += w;
        
        if (bit_count == 32) score += 50;
        else if (bit_count == 24) score += 30;
        else if (bit_count == 8) score += 10;
        
        if (score > best_score) {
            best_score = score;
            best_index = i;
        }
    }
    
    if (best_index == -1) {
        free(buf);
        return NULL;
    }
    
    const unsigned char* best_entry = group_data + 6 + best_index * 14;
    uint16_t icon_id = READ_LE16(best_entry + 12);
    uint32_t icon_size = READ_LE32(best_entry + 8);
    
    // 5. Cari RT_ICON (Type ID: 3) dengan ID yang dipilih
    uint32_t icon_dir_offset = find_resource_entry(buf, res_root, 0, 3, 1, file_size);
    if ((icon_dir_offset & 0x80000000) == 0) {
        free(buf);
        return NULL;
    }
    icon_dir_offset &= 0x7FFFFFFF;
    
    // Masuk ke Name ID subdirectory (RT_ICON) -> cari yang ID-nya cocok
    uint32_t icon_lang_offset = find_resource_entry(buf, res_root, icon_dir_offset, icon_id, 0, file_size);
    if ((icon_lang_offset & 0x80000000) == 0) {
        free(buf);
        return NULL;
    }
    icon_lang_offset &= 0x7FFFFFFF;
    
    // Masuk ke Language subdirectory -> dapatkan data entry offset
    uint32_t icon_data_entry_offset = find_resource_entry(buf, res_root, icon_lang_offset, 0xFFFFFFFF, 0, file_size);
    if (icon_data_entry_offset & 0x80000000) {
        free(buf);
        return NULL;
    }
    
    uint32_t icon_entry_file = res_root + icon_data_entry_offset;
    if (icon_entry_file + 16 > (uint32_t)file_size) {
        free(buf);
        return NULL;
    }
    
    uint32_t icon_rva = READ_LE32(buf + icon_entry_file);
    uint32_t icon_size_real = READ_LE32(buf + icon_entry_file + 4);
    uint32_t icon_file_offset = rva_to_file_offset(icon_rva, num_sections, sec_table, file_size);
    
    if (icon_file_offset == 0 || icon_file_offset + icon_size > (uint32_t)file_size) {
        free(buf);
        return NULL;
    }
    
    // Alokasikan memori untuk data biner ikon saja
    unsigned char* icon_data = (unsigned char*)malloc(icon_size);
    if (!icon_data) {
        free(buf);
        return NULL;
    }
    
    memcpy(icon_data, buf + icon_file_offset, icon_size);
    *out_size = icon_size;
    
    // Deteksi jika biner merupakan berkas PNG utuh
    if (icon_size >= 8 && memcmp(icon_data, "\x89PNG\r\n\x1a\n", 8) == 0) {
        *is_png = 1;
    } else {
        *is_png = 0;
    }
    
    free(buf);
    return icon_data;
}

unsigned char* decode_dib_to_rgba(const unsigned char* dib_data, size_t dib_size, int* out_w, int* out_h) {
    if (dib_size < 40) return NULL;
    
    uint32_t biSize = READ_LE32(dib_data);
    int32_t biWidth = (int32_t)READ_LE32(dib_data + 4);
    int32_t biHeight = (int32_t)READ_LE32(dib_data + 8);
    uint16_t biBitCount = READ_LE16(dib_data + 14);
    uint32_t biCompression = READ_LE32(dib_data + 16);
    
    // Di format ICO, biHeight berisi tinggi XOR mask + tinggi AND mask (2x tinggi asli)
    int w = biWidth;
    int h = biHeight / 2;
    *out_w = w;
    *out_h = h;
    
    unsigned char* rgba = (unsigned char*)malloc(w * h * 4);
    if (!rgba) return NULL;
    memset(rgba, 0, w * h * 4);
    
    const unsigned char* xor_pixels = dib_data + biSize;
    const unsigned char* color_table = NULL;
    int color_table_size = 0;
    
    if (biBitCount <= 8) {
        color_table = dib_data + biSize;
        color_table_size = (1 << biBitCount);
        xor_pixels += color_table_size * 4;
    }
    
    if (biCompression != 0 && biCompression != 3) { // 0 = BI_RGB, 3 = BI_BITFIELDS
        free(rgba);
        return NULL;
    }
    
    // Baris data XOR mask dibulatkan ke kelipatan 4 bytes
    int xor_row_size = ((w * biBitCount + 31) / 32) * 4;
    // Baris data AND mask (1 bit per piksel) dibulatkan ke kelipatan 4 bytes
    int and_row_size = ((w + 31) / 32) * 4;
    
    const unsigned char* and_pixels = xor_pixels + h * xor_row_size;
    
    // Pastikan kita tidak membaca melebihi buffer DIB
    if ((size_t)(and_pixels + h * and_row_size - dib_data) > dib_size) {
        and_pixels = NULL; // Jangan gunakan AND mask jika corrupt/kurang
    }
    
    for (int y = 0; y < h; y++) {
        // DIB disimpan bottom-up
        int target_y = h - 1 - y;
        const unsigned char* xor_row = xor_pixels + y * xor_row_size;
        const unsigned char* and_row = and_pixels ? (and_pixels + y * and_row_size) : NULL;
        
        for (int x = 0; x < w; x++) {
            unsigned char r = 0, g = 0, b = 0, a = 255;
            
            if (biBitCount == 32) {
                b = xor_row[x * 4 + 0];
                g = xor_row[x * 4 + 1];
                r = xor_row[x * 4 + 2];
                a = xor_row[x * 4 + 3];
            } else if (biBitCount == 24) {
                b = xor_row[x * 3 + 0];
                g = xor_row[x * 3 + 1];
                r = xor_row[x * 3 + 2];
                a = 255;
            } else if (biBitCount == 8) {
                unsigned char idx = xor_row[x];
                if (idx < color_table_size) {
                    b = color_table[idx * 4 + 0];
                    g = color_table[idx * 4 + 1];
                    r = color_table[idx * 4 + 2];
                }
                a = 255;
            } else if (biBitCount == 4) {
                unsigned char byte = xor_row[x / 2];
                unsigned char idx = (x % 2 == 0) ? (byte >> 4) : (byte & 0x0F);
                if (idx < color_table_size) {
                    b = color_table[idx * 4 + 0];
                    g = color_table[idx * 4 + 1];
                    r = color_table[idx * 4 + 2];
                }
                a = 255;
            } else if (biBitCount == 1) {
                unsigned char byte = xor_row[x / 8];
                unsigned char bit = (byte >> (7 - (x % 8))) & 1;
                if (bit < color_table_size) {
                    b = color_table[bit * 4 + 0];
                    g = color_table[bit * 4 + 1];
                    r = color_table[bit * 4 + 2];
                }
                a = 255;
            }
            
            // Terapkan transparansi AND mask 1-bit
            if (and_row) {
                unsigned char and_byte = and_row[x / 8];
                unsigned char and_bit = (and_byte >> (7 - (x % 8))) & 1;
                if (and_bit) {
                    a = 0; // Transparan penuh
                }
            }
            
            int idx = (target_y * w + x) * 4;
            rgba[idx + 0] = r;
            rgba[idx + 1] = g;
            rgba[idx + 2] = b;
            rgba[idx + 3] = a;
        }
    }
    
    return rgba;
}
