.PHONY: all build run clean install

# Target default: memicu kompilasi/build proyek
all: build

# Mendownload dependensi compiler dan pustaka SDL3 secara lokal ke folder external
install:
	@./scripts/setup_compiler.sh

# Melakukan konfigurasi CMake jika belum ada, lalu mengompilasi proyek
build:
	@./scripts/build.sh

# Membangun proyek terlebih dahulu (jika ada perubahan), lalu langsung menjalankan aplikasi
run:
	@./scripts/build.sh run

# Menghapus direktori build dan membersihkan seluruh artefak kompilasi
clean:
	@./scripts/build.sh clean
