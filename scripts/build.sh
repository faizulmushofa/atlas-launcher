#!/bin/bash

# Dapatkan direktori script dan root proyek secara absolut
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/.."

# Masuk ke direktori utama proyek
cd "$PROJECT_ROOT" || exit 1

ACTION=$1

if [ "$ACTION" = "clean" ]; then
    echo "[Spotlight Search] Pembersihan build dan bin folder..."
    rm -rf build bin
    echo "[Spotlight Search] Clean selesai."
elif [ "$ACTION" = "run" ]; then
    if [ -f bin/spotlight_search ]; then
        echo "[Spotlight Search] Menjalankan aplikasi..."
        ./bin/spotlight_search
    else
        echo "[Spotlight Search] Eksekutabel tidak ditemukan. Silakan jalankan './scripts/build.sh' terlebih dahulu."
    fi
else
    goto_direct_build=false
    if command -v cmake &> /dev/null; then
        echo "[Spotlight Search] Mengonfigurasi build dengan CMake..."
        cmake -B build -S .
        if [ $? -ne 0 ]; then
            echo "[Spotlight Search] Gagal konfigurasi CMake. Mencoba kompilasi langsung..."
            goto_direct_build=true
        else
            echo "[Spotlight Search] Memulai proses kompilasi..."
            cmake --build build
            if [ $? -ne 0 ]; then
                echo "[Spotlight Search] Gagal kompilasi proyek."
                exit 1
            fi
            mkdir -p bin
            cp build/spotlight_search bin/spotlight_search
            echo "[Spotlight Search] Build sukses! Executable disalin ke bin/spotlight_search"
        fi
    else
        goto_direct_build=true
    fi

    if [ "$goto_direct_build" = true ]; then
        echo "[Spotlight Search] CMake tidak terdeteksi. Melakukan kompilasi langsung..."
        mkdir -p bin
        
        OS_NAME=$(uname -s)
        
        if command -v clang &> /dev/null; then
            CC=clang
        elif command -v gcc &> /dev/null; then
            CC=gcc
        else
            echo "[Spotlight Search] Error: Tidak ada compiler (gcc/clang) yang ditemukan!"
            exit 1
        fi
        
        CFLAGS="-O3 -Wall -Wextra -Isrc -Iexternal/sqlite3"
        LIBS=""
        
        if command -v pkg-config &> /dev/null && pkg-config --exists sdl3; then
            CFLAGS="$CFLAGS $(pkg-config --cflags sdl3)"
            LIBS="$LIBS $(pkg-config --libs sdl3)"
        else
            CFLAGS="$CFLAGS -Iexternal/sdl3/include"
            LIBS="$LIBS -Lexternal/sdl3/lib -lSDL3"
        fi
        
        SRCS="src/main.c src/core/app.c src/core/state.c src/render/gl_render.c src/render/draw2d.c src/platform/detection.c src/platform/fs.c src/ui/input.c src/ui/ui.c src/db/sqlite.c src/db/indexer.c src/icon/icon.c src/icon/icon_cache.c src/search/search.c src/search/ranking.c external/sqlite3/sqlite3.c"
        
        if [ "$OS_NAME" = "Darwin" ]; then
            SRCS="$SRCS src/render/platform_text_macos.m src/icon/icon_os_macos.m src/platform/platform_macos.m"
            LIBS="$LIBS -framework Cocoa -framework AppKit"
        else
            SRCS="$SRCS src/platform/platform.c"
            LIBS="$LIBS -lm -lpthread -ldl"
        fi
        
        $CC $CFLAGS $SRCS $LIBS -o bin/spotlight_search
        if [ $? -ne 0 ]; then
            echo "[Spotlight Search] Gagal kompilasi proyek secara langsung."
            exit 1
        fi
        echo "[Spotlight Search] Build sukses! Executable dibuat di bin/spotlight_search"
    fi
fi
