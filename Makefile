.PHONY: all build run clean

# Target default: memicu kompilasi/build proyek
all: build

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
