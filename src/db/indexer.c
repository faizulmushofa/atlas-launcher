#include "indexer.h"
#include "sqlite.h"
#include "../platform/fs.h"
#include "../platform/detection.h"
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

// Helper check filter ektensi dokumen
static bool is_document_extension(const char* filename, const char** out_ext) {
    const char* dot = strrchr(filename, '.');
    if (!dot) return false;
    
    const char* allowed_extensions[] = {
        ".pdf", ".png", ".jpg", ".jpeg", ".docx", ".xlsx", ".pptx", ".txt", ".md", ".zip"
    };
    int count = sizeof(allowed_extensions) / sizeof(allowed_extensions[0]);
    
    for (int i = 0; i < count; i++) {
        if (SDL_strcasecmp(dot, allowed_extensions[i]) == 0) {
            *out_ext = allowed_extensions[i] + 1;
            return true;
        }
    }
    return false;
}

// Helper filter abaikan folder sistem/sampah/dev
static bool should_ignore_dir(const char* dir_name) {
    if (dir_name[0] == '.') return true;

    const char* ignore_list[] = {
        "node_modules", "Library", "AppData", "Application Data",
        "Local Settings", "Templates", "NetHood", "PrintHood",
        "Applications", "System", "Pictures", "Music", "Movies",
        "Public", "Creative Cloud Files", "bin", "obj", "build", "dist"
    };
    int count = sizeof(ignore_list) / sizeof(ignore_list[0]);
    for (int i = 0; i < count; i++) {
        if (SDL_strcasecmp(dir_name, ignore_list[i]) == 0) return true;
    }
    return false;
}

// Rekursi Dokumen
static void scan_user_documents_recursive(const char* dir_path, int depth);

static SDL_EnumerationResult SDLCALL scan_docs_callback(void *userdata, const char *dirname, const char *fname) {
    int depth = *(int*)userdata;

    if (should_ignore_dir(fname)) {
        return SDL_ENUM_CONTINUE;
    }

    char full_path[2048];
    snprintf(full_path, sizeof(full_path), "%s%s", dirname, fname);

    SDL_PathInfo info;
    if (SDL_GetPathInfo(full_path, &info)) {
        if (info.type == SDL_PATHTYPE_DIRECTORY) {
            scan_user_documents_recursive(full_path, depth + 1);
        } else if (info.type == SDL_PATHTYPE_FILE) {
            const char* ext = NULL;
            if (is_document_extension(fname, &ext)) {
                db_insert_item(fname, full_path, ext, platform_get_os_name());
            }
        }
    }
    return SDL_ENUM_CONTINUE;
}

static void scan_user_documents_recursive(const char* dir_path, int depth) {
    if (depth > 3) return; // Batasi kedalaman rekursi agar performa tetap cepat
    SDL_EnumerateDirectory(dir_path, scan_docs_callback, &depth);
}

// Rekursi Aplikasi
static void scan_apps_recursive(const char* dir_path, int depth);

typedef struct {
    int depth;
} AppScanContext;

static SDL_EnumerationResult SDLCALL scan_apps_callback(void *userdata, const char *dirname, const char *fname) {
    AppScanContext* ctx = (AppScanContext*)userdata;
    size_t fname_len = strlen(fname);
    if (fname_len == 0) return SDL_ENUM_CONTINUE;

    if (fname[0] == '.') return SDL_ENUM_CONTINUE;

    char full_path[2048];
    snprintf(full_path, sizeof(full_path), "%s%s", dirname, fname);

    SDL_PathInfo info;
    if (SDL_GetPathInfo(full_path, &info)) {
        if (info.type == SDL_PATHTYPE_DIRECTORY) {
#ifdef __APPLE__
            // Di macOS, jika folder berakhiran .app, anggap aplikasi dan jangan masuk ke dalam
            if (fname_len > 4 && strcmp(fname + fname_len - 4, ".app") == 0) {
                char name[128];
                size_t name_len = fname_len - 4;
                if (name_len >= sizeof(name)) name_len = sizeof(name) - 1;
                strncpy(name, fname, name_len);
                name[name_len] = '\0';
                db_insert_item(name, full_path, "app", "macOS");
                return SDL_ENUM_CONTINUE;
            }
#endif
            scan_apps_recursive(full_path, ctx->depth + 1);
        } else if (info.type == SDL_PATHTYPE_FILE) {
#ifdef _WIN32
            // Di Windows, cari file .lnk
            if (fname_len > 4 && SDL_strcasecmp(fname + fname_len - 4, ".lnk") == 0) {
                char name[128];
                size_t name_len = fname_len - 4;
                if (name_len >= sizeof(name)) name_len = sizeof(name) - 1;
                strncpy(name, fname, name_len);
                name[name_len] = '\0';
                db_insert_item(name, full_path, "app", "Windows");
            }
#elif defined(__linux__)
            // Di Linux, cari file .desktop
            if (fname_len > 8 && strcmp(fname + fname_len - 8, ".desktop") == 0) {
                FILE* f = fopen(full_path, "r");
                if (f) {
                    char line[256];
                    char name[128] = "";
                    char exec[256] = "";
                    while (fgets(line, sizeof(line), f)) {
                        if (strncmp(line, "Name=", 5) == 0 && strlen(name) == 0) {
                            strncpy(name, line + 5, sizeof(name) - 1);
                            name[strcspn(name, "\r\n")] = 0;
                        }
                        if (strncmp(line, "Exec=", 5) == 0 && strlen(exec) == 0) {
                            strncpy(exec, line + 5, sizeof(exec) - 1);
                            exec[strcspn(exec, "\r\n")] = 0;
                            char* space = strchr(exec, ' ');
                            if (space) *space = '\0';
                        }
                    }
                    fclose(f);
                    if (strlen(name) > 0 && strlen(exec) > 0) {
                        db_insert_item(name, exec, "app", "Linux");
                    }
                }
            }
#endif
        }
    }
    return SDL_ENUM_CONTINUE;
}

static void scan_apps_recursive(const char* dir_path, int depth) {
    if (depth > 5) return;
    AppScanContext ctx = { depth };
    SDL_EnumerateDirectory(dir_path, scan_apps_callback, &ctx);
}

void indexer_run(void) {
    struct sqlite3* db = db_get_handle();
    if (!db) return;

    printf("[Indexer] Memulai pemindaian. Platform terdeteksi: %s\n", platform_get_os_name());

    // Bersihkan tabel items lama
    char* err_msg = NULL;
    int rc = sqlite3_exec((sqlite3*)db, "DELETE FROM items;", NULL, NULL, &err_msg);
    if (rc != 0) {
        fprintf(stderr, "[Indexer] Gagal membersihkan tabel items: %s\n", err_msg);
        if (err_msg) sqlite3_free(err_msg);
        return;
    }

    // Mulai transaksi SQLite untuk mempercepat penyimpanan data ke disk
    sqlite3_exec((sqlite3*)db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    // 1. Pemindaian Aplikasi
#ifdef __APPLE__
    printf("[Indexer] Memindai folder aplikasi macOS...\n");
    scan_apps_recursive("/Applications", 0);
    scan_apps_recursive("/System/Applications", 0);
#elif defined(_WIN32)
    char* program_data = getenv("ProgramData");
    if (program_data) {
        char path[512];
        snprintf(path, sizeof(path), "%s\\Microsoft\\Windows\\Start Menu\\Programs", program_data);
        printf("[Indexer] Memindai Start Menu (System): %s\n", path);
        scan_apps_recursive(path, 0);
    }
    char* app_data = getenv("APPDATA");
    if (app_data) {
        char path[512];
        snprintf(path, sizeof(path), "%s\\Microsoft\\Windows\\Start Menu\\Programs", app_data);
        printf("[Indexer] Memindai Start Menu (User): %s\n", path);
        scan_apps_recursive(path, 0);
    }
#elif defined(__linux__)
    printf("[Indexer] Memindai folder aplikasi Linux...\n");
    scan_apps_recursive("/usr/share/applications", 0);
    char* home = getenv("HOME");
    if (home) {
        char local_apps[512];
        snprintf(local_apps, sizeof(local_apps), "%s/.local/share/applications", home);
        scan_apps_recursive(local_apps, 0);
    }
#endif

    // 2. Pemindaian folder dokumen pengguna (Desktop, Documents, Downloads)
    const char* desktop = SDL_GetUserFolder(SDL_FOLDER_DESKTOP);
    if (desktop) {
        printf("[Indexer] Memindai Desktop: %s\n", desktop);
        scan_user_documents_recursive(desktop, 0);
    }
    
    const char* documents = SDL_GetUserFolder(SDL_FOLDER_DOCUMENTS);
    if (documents) {
        printf("[Indexer] Memindai Documents: %s\n", documents);
        scan_user_documents_recursive(documents, 0);
    }
    
    const char* downloads = SDL_GetUserFolder(SDL_FOLDER_DOWNLOADS);
    if (downloads) {
        printf("[Indexer] Memindai Downloads: %s\n", downloads);
        scan_user_documents_recursive(downloads, 0);
    }

    // Commit seluruh data terindeks ke basis data
    sqlite3_exec((sqlite3*)db, "COMMIT;", NULL, NULL, NULL);

    // Cari jumlah item terindeks (dipisah antara aplikasi dan dokumen)
    sqlite3_stmt* stmt = NULL;
    int total_apps = 0;
    int total_docs = 0;
    
    if (sqlite3_prepare_v2((sqlite3*)db, "SELECT COUNT(*) FROM items WHERE type = 'app';", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total_apps = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    if (sqlite3_prepare_v2((sqlite3*)db, "SELECT COUNT(*) FROM items WHERE type != 'app';", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total_docs = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    printf("[Indexer] Pemindaian selesai! Berhasil mengindeks %d aplikasi dan %d dokumen (Total: %d item).\n", 
           total_apps, total_docs, total_apps + total_docs);
}
