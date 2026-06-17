.PHONY: all build run clean install

# Target default: memicu kompilasi/build proyek
all: build

# Mendownload dependensi pustaka SDL3 secara lokal ke folder external/sdl3
install:
	@echo "Membersihkan dependensi lama..."
	@rm -rf external/sdl2
	@rm -rf external/sdl3
	@mkdir -p external
	@echo "Mengunduh SDL3 dari github.com ke external/sdl3..."
	@git clone --depth 1 -b main https://github.com/libsdl-org/SDL.git external/sdl3
	@echo "Instalasi selesai! Jalankan 'make build' untuk mengompilasi proyek."

# Melakukan konfigurasi CMake jika belum ada, lalu mengompilasi proyek
build:
	@cmake -B build -S .
	@cmake --build build

# Membangun proyek terlebih dahulu (jika ada perubahan), lalu langsung menjalankan aplikasi
run: build
	@cmake --build build --target run

# Menghapus direktori build dan membersihkan seluruh artefak kompilasi
clean:
	@rm -rf build
