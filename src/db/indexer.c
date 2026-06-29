#include "indexer.h"
#include "sqlite.h"
#include "../platform/fs.h"
#include "../platform/detection.h"
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(__APPLE__) || defined(__linux__)
#include <dirent.h>
#include <sys/stat.h>
#include <strings.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

// Helper check filter ektensi dokumen
static bool is_document_extension(const char* filename, const char** out_ext) {
    const char* dot = strrchr(filename, '.');
    if (!dot) return false;
    
    const char* allowed_extensions[] = {
        ".pdf", ".png", ".jpg", ".jpeg", ".docx", ".xlsx", ".pptx", ".txt", ".md", ".zip"
    };
    int count = sizeof(allowed_extensions) / sizeof(allowed_extensions[0]);
    
    for (int i = 0; i < count; i++) {
#ifdef _WIN32
        if (_stricmp(dot, allowed_extensions[i]) == 0) {
#else
        if (strcasecmp(dot, allowed_extensions[i]) == 0) {
#endif
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
#ifdef _WIN32
        if (_stricmp(dir_name, ignore_list[i]) == 0) return true;
#else
        if (strcasecmp(dir_name, ignore_list[i]) == 0) return true;
#endif
    }
    return false;
}

#ifdef _WIN32
static bool should_ignore_dir_win(const WCHAR* dir_name) {
    if (dir_name[0] == L'.') return true;

    char name_utf8[128];
    WideCharToMultiByte(CP_UTF8, 0, dir_name, -1, name_utf8, sizeof(name_utf8) - 1, NULL, NULL);
    name_utf8[sizeof(name_utf8) - 1] = '\0';

    return should_ignore_dir(name_utf8);
}
#endif

// macOS App Scanning
#ifdef __APPLE__
static void scan_mac_apps(const char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        size_t len = strlen(entry->d_name);
        if (len > 4 && strcmp(entry->d_name + len - 4, ".app") == 0) {
            char name[128];
            strncpy(name, entry->d_name, len - 4);
            name[len - 4] = '\0';

            char path[512];
            snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

            db_insert_item(name, path, "app", "macOS");
        }
    }
    closedir(dir);
}

static void scan_user_documents_recursive(const char* dir_path, int depth) {
    if (depth > 3) return; // Batasi kedalaman rekursi agar performa tetap cepat

    DIR* dir = opendir(dir_path);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (should_ignore_dir(entry->d_name)) {
            continue;
        }

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                scan_user_documents_recursive(full_path, depth + 1);
            } else if (S_ISREG(st.st_mode)) {
                const char* ext = NULL;
                if (is_document_extension(entry->d_name, &ext)) {
                    db_insert_item(entry->d_name, full_path, ext, platform_get_os_name());
                }
            }
        }
    }
    closedir(dir);
}
#endif

// Windows App Scanning & Recursive Document Scanning
#ifdef _WIN32
static void scan_win_apps_recursive(const WCHAR* dir_path) {
    WCHAR search_path[MAX_PATH];
    swprintf(search_path, MAX_PATH, L"%s\\*", dir_path);

    WIN32_FIND_DATAW find_data;
    HANDLE find_handle = FindFirstFileW(search_path, &find_data);
    if (find_handle == INVALID_HANDLE_VALUE) return;

    do {
        if (wcscmp(find_data.cFileName, L".") == 0 || wcscmp(find_data.cFileName, L"..") == 0) {
            continue;
        }

        WCHAR full_path[MAX_PATH];
        swprintf(full_path, MAX_PATH, L"%s\\%s", dir_path, find_data.cFileName);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            scan_win_apps_recursive(full_path);
        } else {
            size_t len = wcslen(find_data.cFileName);
            if (len > 4 && _wcsicmp(find_data.cFileName + len - 4, L".lnk") == 0) {
                char name[128];
                WideCharToMultiByte(CP_UTF8, 0, find_data.cFileName, (int)len - 4, name, sizeof(name) - 1, NULL, NULL);
                name[len - 4] = '\0';

                char path[512];
                WideCharToMultiByte(CP_UTF8, 0, full_path, -1, path, sizeof(path) - 1, NULL, NULL);

                db_insert_item(name, path, "app", "Windows");
            }
        }
    } while (FindNextFileW(find_handle, &find_data));

    FindClose(find_handle);
}

static void scan_user_documents_recursive_win(const WCHAR* dir_path, int depth) {
    if (depth > 3) return;

    WCHAR search_path[MAX_PATH];
    swprintf(search_path, MAX_PATH, L"%s\\*", dir_path);

    WIN32_FIND_DATAW find_data;
    HANDLE find_handle = FindFirstFileW(search_path, &find_data);
    if (find_handle == INVALID_HANDLE_VALUE) return;

    do {
        if (should_ignore_dir_win(find_data.cFileName)) {
            continue;
        }

        WCHAR full_path[MAX_PATH];
        swprintf(full_path, MAX_PATH, L"%s\\%s", dir_path, find_data.cFileName);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            scan_user_documents_recursive_win(full_path, depth + 1);
        } else {
            char filename_utf8[128];
            WideCharToMultiByte(CP_UTF8, 0, find_data.cFileName, -1, filename_utf8, sizeof(filename_utf8) - 1, NULL, NULL);
            filename_utf8[sizeof(filename_utf8) - 1] = '\0';

            const char* ext = NULL;
            if (is_document_extension(filename_utf8, &ext)) {
                char path_utf8[512];
                WideCharToMultiByte(CP_UTF8, 0, full_path, -1, path_utf8, sizeof(path_utf8) - 1, NULL, NULL);
                path_utf8[sizeof(path_utf8) - 1] = '\0';

                db_insert_item(filename_utf8, path_utf8, ext, "Windows");
            }
        }
    } while (FindNextFileW(find_handle, &find_data));

    FindClose(find_handle);
}
#endif

// Linux App Scanning & Recursive Document Scanning
#ifdef __linux__
static void scan_linux_apps(const char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        size_t len = strlen(entry->d_name);
        if (len > 8 && strcmp(entry->d_name + len - 8, ".desktop") == 0) {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

            FILE* f = fopen(path, "r");
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
    }
    closedir(dir);
}

static void scan_user_documents_recursive(const char* dir_path, int depth) {
    if (depth > 3) return;

    DIR* dir = opendir(dir_path);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (should_ignore_dir(entry->d_name)) {
            continue;
        }

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                scan_user_documents_recursive(full_path, depth + 1);
            } else if (S_ISREG(st.st_mode)) {
                const char* ext = NULL;
                if (is_document_extension(entry->d_name, &ext)) {
                    db_insert_item(entry->d_name, full_path, ext, platform_get_os_name());
                }
            }
        }
    }
    closedir(dir);
}
#endif

void indexer_run(void) {
    struct sqlite3* db = db_get_handle();
    if (!db) return;

    printf("[Indexer] Memulai pemindaian aplikasi sistem...\n");

    // Bersihkan tabel items lama
    char* err_msg = NULL;
    int rc = sqlite3_exec((sqlite3*)db, "DELETE FROM items;", NULL, NULL, &err_msg);
    if (rc != 0) {
        fprintf(stderr, "[Indexer] Gagal membersihkan tabel items: %s\n", err_msg);
        if (err_msg) sqlite3_free(err_msg);
        return;
    }

#ifdef __APPLE__
    scan_mac_apps("/Applications");
    scan_mac_apps("/System/Applications");
#elif defined(_WIN32)
    WCHAR program_data[MAX_PATH];
    ExpandEnvironmentStringsW(L"%ProgramData%\\Microsoft\\Windows\\Start Menu\\Programs", program_data, MAX_PATH);
    scan_win_apps_recursive(program_data);

    WCHAR app_data[MAX_PATH];
    ExpandEnvironmentStringsW(L"%AppData%\\Microsoft\\Windows\\Start Menu\\Programs", app_data, MAX_PATH);
    scan_win_apps_recursive(app_data);
#elif defined(__linux__)
    scan_linux_apps("/usr/share/applications");
    char* home = getenv("HOME");
    if (home) {
        char local_apps[512];
        snprintf(local_apps, sizeof(local_apps), "%s/.local/share/applications", home);
        scan_linux_apps(local_apps);
    }
#endif

    // Pemindaian seluruh folder home pengguna secara rekursif (depth <= 3)
    char home_path[256];
    if (fs_get_user_home(home_path, sizeof(home_path))) {
        printf("[Indexer] Memindai seluruh folder pengguna di: %s\n", home_path);
#ifdef _WIN32
        WCHAR whome[MAX_PATH];
        MultiByteToWideChar(CP_UTF8, 0, home_path, -1, whome, MAX_PATH);
        scan_user_documents_recursive_win(whome, 0);
#else
        scan_user_documents_recursive(home_path, 0);
#endif
    }

    // Cari jumlah item terindeks
    sqlite3_stmt* stmt = NULL;
    int total = 0;
    if (sqlite3_prepare_v2((sqlite3*)db, "SELECT COUNT(*) FROM items;", -1, &stmt, NULL) == 0) {
        if (sqlite3_step(stmt) == 100) { // SQLITE_ROW = 100
            total = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    printf("[Indexer] Pemindaian selesai! Berhasil mengindeks %d item.\n", total);
}
