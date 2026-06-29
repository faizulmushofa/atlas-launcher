#include "icns_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>

#define READ_BE32(p) ((uint32_t)(((p)[0] << 24) | ((p)[1] << 16) | ((p)[2] << 8) | (p)[3]))

static unsigned char* extract_png_from_icns(const char* icns_path, size_t* out_size) {
    FILE* f = fopen(icns_path, "rb");
    if (!f) return NULL;
    
    unsigned char header[8];
    if (fread(header, 1, 8, f) != 8) {
        fclose(f);
        return NULL;
    }
    
    if (memcmp(header, "icns", 4) != 0) {
        fclose(f);
        return NULL;
    }
    
    uint32_t total_size = READ_BE32(header + 4);
    
    unsigned char* best_png_data = NULL;
    size_t best_png_size = 0;
    int best_score = -1;
    
    uint32_t offset = 8;
    while (offset < total_size) {
        unsigned char entry_header[8];
        if (fread(entry_header, 1, 8, f) != 8) break;
        
        char type[4];
        memcpy(type, entry_header, 4);
        uint32_t entry_size = READ_BE32(entry_header + 4);
        
        if (entry_size < 8 || offset + entry_size > total_size) break;
        uint32_t data_size = entry_size - 8;
        
        int score = -1;
        if (memcmp(type, "ic08", 4) == 0) score = 100;      // 256x256
        else if (memcmp(type, "ic13", 4) == 0) score = 95; // 256x256@2x
        else if (memcmp(type, "ic07", 4) == 0) score = 90; // 128x128
        else if (memcmp(type, "ic09", 4) == 0) score = 85; // 512x512
        else if (memcmp(type, "ic14", 4) == 0) score = 80; // 512x512@2x
        else if (memcmp(type, "ic10", 4) == 0) score = 70; // 1024x1024
        else if (memcmp(type, "ic12", 4) == 0) score = 60; // 32x32@2x
        else if (memcmp(type, "ic11", 4) == 0) score = 50; // 16x16@2x
        
        if (score > best_score) {
            unsigned char* data = (unsigned char*)malloc(data_size);
            if (data && fread(data, 1, data_size, f) == data_size) {
                if (data_size >= 8 && memcmp(data, "\x89PNG\r\n\x1a\n", 8) == 0) {
                    if (best_png_data) free(best_png_data);
                    best_png_data = data;
                    best_png_size = data_size;
                    best_score = score;
                } else {
                    free(data);
                }
            } else {
                free(data);
            }
        } else {
            fseek(f, data_size, SEEK_CUR);
        }
        
        offset += entry_size;
    }
    
    fclose(f);
    
    if (best_png_data) {
        *out_size = best_png_size;
        return best_png_data;
    }
    return NULL;
}

unsigned char* macos_extract_bundle_icon(const char* app_bundle_path, size_t* out_size) {
    char res_path[512];
    snprintf(res_path, sizeof(res_path), "%s/Contents/Resources", app_bundle_path);
    
    DIR* dir = opendir(res_path);
    if (!dir) return NULL;
    
    char first_icns_path[512] = {0};
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        size_t len = strlen(entry->d_name);
        if (len > 5 && strcmp(entry->d_name + len - 5, ".icns") == 0) {
            snprintf(first_icns_path, sizeof(first_icns_path), "%s/%s", res_path, entry->d_name);
            break;
        }
    }
    closedir(dir);
    
    if (first_icns_path[0] != '\0') {
        return extract_png_from_icns(first_icns_path, out_size);
    }
    return NULL;
}
