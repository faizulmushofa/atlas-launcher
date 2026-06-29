#!/bin/bash

# Mendeteksi Sistem Operasi
OS_TYPE=""
if [[ "$OSTYPE" == "darwin"* ]]; then
    OS_TYPE="MACOS"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS_TYPE="LINUX"
else
    OS_TYPE="UNKNOWN"
fi

echo "[Spotlight Search] Memulai setup compiler lokal/sistem untuk $OSTYPE..."

if [ "$OS_TYPE" = "MACOS" ]; then
    # Di Mac, periksa clang
    if command -v clang &> /dev/null; then
        echo "[Spotlight Search] Compiler clang terdeteksi di sistem macOS!"
    else
        echo "[Spotlight Search] PERINGATAN: Clang/Xcode Command Line Tools tidak ditemukan."
        echo "[Spotlight Search] Menjalankan perintah instalasi Xcode CLI Tools..."
        xcode-select --install
        echo "[Spotlight Search] Silakan ikuti instruksi pop-up di layar Mac Anda, lalu jalankan kembali script ini setelah selesai."
    fi
elif [ "$OS_TYPE" = "LINUX" ]; then
    # Di Linux, periksa gcc/clang
    if command -v gcc &> /dev/null || command -v clang &> /dev/null; then
        echo "[Spotlight Search] Compiler gcc/clang terdeteksi di sistem Linux!"
    else
        echo "[Spotlight Search] compiler tidak ditemukan. Mencoba mencari paket manager..."
        if command -v apt-get &> /dev/null; then
            echo "[Spotlight Search] Jalankan perintah ini untuk memasang compiler:"
            echo "sudo apt-get update && sudo apt-get install build-essential cmake"
        elif command -v pacman &> /dev/null; then
            echo "[Spotlight Search] Jalankan perintah ini untuk memasang compiler:"
            echo "sudo pacman -S base-devel cmake"
        elif command -v dnf &> /dev/null; then
            echo "[Spotlight Search] Jalankan perintah ini untuk memasang compiler:"
            echo "sudo dnf groupinstall \"Development Tools\" && sudo dnf install cmake"
        else
            echo "[Spotlight Search] Silakan pasang paket 'gcc' dan 'cmake' secara manual pada sistem Linux Anda."
        fi
    fi
else
    echo "[Spotlight Search] Sistem operasi tidak dikenal untuk otomatisasi setup compiler."
fi

# Mengunduh SDL3 jika belum ada
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [ ! -d "$PROJECT_ROOT/external/sdl3" ]; then
    echo "[Spotlight Search] Mengunduh SDL3 dari github.com ke external/sdl3..."
    rm -rf "$PROJECT_ROOT/external/sdl2"
    rm -rf "$PROJECT_ROOT/external/sdl3"
    mkdir -p "$PROJECT_ROOT/external"
    git clone --depth 1 -b main https://github.com/libsdl-org/SDL.git "$PROJECT_ROOT/external/sdl3"
else
    echo "[Spotlight Search] SDL3 sudah diunduh di external/sdl3."
fi

# Mengunduh SQLite jika belum ada
if [ ! -d "$PROJECT_ROOT/external/sqlite3" ]; then
    echo "[Spotlight Search] Mengunduh SQLite amalgamation dari github.com ke external/sqlite3..."
    rm -rf "$PROJECT_ROOT/external/sqlite3"
    git clone --depth 1 https://github.com/azadkuh/sqlite-amalgamation.git "$PROJECT_ROOT/external/sqlite3"
else
    echo "[Spotlight Search] SQLite sudah diunduh di external/sqlite3."
fi

echo "[Spotlight Search] Setup selesai!"
