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
    echo "[Spotlight Search] Mengonfigurasi build dengan CMake..."
    cmake -B build -S .
    if [ $? -ne 0 ]; then
        echo "[Spotlight Search] Gagal konfigurasi CMake."
        exit 1
    fi
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
