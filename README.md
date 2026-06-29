# Cross-Platform Spotlight Clone

[![C Language](https://img.shields.io/badge/Language-C99%20%2F%20C11-blue?logo=c&logoColor=white)](https://en.cppreference.com/w/c)
[![SDL3](https://img.shields.io/badge/SDL-3.0-brightgreen?logo=sdl&logoColor=white)](https://www.libsdl.org)
[![SQLite](https://img.shields.io/badge/SQLite-3-blue?logo=sqlite&logoColor=white)](https://www.sqlite.org)
[![CMake](https://img.shields.io/badge/CMake-3.14+-red?logo=cmake&logoColor=white)](https://cmake.org)

**Cross-Platform Spotlight Clone** adalah aplikasi antarmuka pencarian cepat (*desktop overlay launcher*) berbasis bahasa pemrograman **C**, **SDL3** untuk manajemen jendela, event, dan input, serta **SQLite** untuk database pencarian lokal yang sangat cepat.

Aplikasi ini meniru antarmuka visual macOS Spotlight Search secara fungsional: melakukan pencarian real-time dengan pencocokan teroptimasi, serta merender hasil pencarian lengkap dengan ikon asli berkas dari sistem operasi (*native OS icons*).

---

## 🏗️ Arsitektur Hubungan Sistem

Berikut adalah diagram alur data dan interaksi antarmuka pengguna pada sistem Spotlight Clone:

```mermaid
graph TD
    User["Pengguna (Keyboard/Mouse)"] -->|1. Ketik Query / Navigasi| UI["UI Layer (input.c / ui.c)"]
    UI -->|2. Update Query| State["App State (state.c)"]
    State -->|3. Debounced Search (100-200ms)| Search["Search Engine (search.c)"]
    Search -->|4. Kueri Hasil Terbaik| SQLite["SQLite DB (sqlite.c)"]
    Indexer["Background Indexer (indexer.c)"] -->|Scan Filesystem| SQLite
    Search -->|5. Simpan Hasil Pilihan| State
    UI -->|6. Minta Ikon File| Icon["Icon Cache (icon_cache.c)"]
    Icon -->|7. Miss? Ekstrak Native| NativeOS["Native OS API (NSWorkspace / Win32)"]
    UI -->|8. Render Teks/Bentuk| Render["SDL3 Renderer (gl_render.c / draw2d.c)"]
    Render -->|9. Swap Buffer (Tampilkan)| Screen["Overlay Window (app.c)"]
```

---

## ⚡ Fitur Utama

- 🔍 **Pencarian SQLite Real-Time**: Pencarian secepat kilat dengan pencocokan string terindeks dan kueri teroptimasi.
- 🕒 **Sistem Debouncing Input**: Menunda eksekusi kueri SQLite selama 100-200ms setelah selesai mengetik untuk menghindari *disk overhead* yang membebani CPU.
- 🎨 **Ekstraksi Ikon Native OS**: Mengambil ikon asli sistem berkas menggunakan Cocoa (`NSWorkspace`) di macOS dan Win32 API di Windows.
- 💾 **GPU Texture Caching**: Mengubah pixel buffer ikon menjadi `SDL_Texture` sekali saja dan menyimpannya di memori GPU untuk rendering instan tanpa lag.
- 🔤 **Native OS Text Rendering**: Merender font sistem secara dinamis dengan kualitas tinggi (*antialiasing* native OS) serta mendukung karakter Unicode (UTF-8) penuh secara luwes (menggantikan sistem pemetaan ASCII manual yang kaku).

---

## 📁 Struktur Direktori & Tanggung Jawab

Struktur direktori proyek ini dirancang secara modular guna memisahkan tanggung jawab logika secara bersih:

| Direktori / Berkas | Tanggung Jawab (Responsibility) | File Kunci Utama |
| :--- | :--- | :--- |
| [bin/](bin/) | Menyimpan berkas biner eksekutabel hasil kompilasi. | `spotlight_search` |
| [db/](db/) | Tempat penyimpanan basis data SQLite lokal. | `spotlight.db` |
| [external/](external/) | Berisi kode sumber pihak ketiga yang dikelola secara lokal. | `sdl3/`, `sqlite3/` |
| [scripts/](scripts/) | Berisi skrip otomatisasi build dan kompilasi lintas platform. | `build.sh`, `setup_compiler.sh` |
| [src/core/](src/core/) | Manajemen siklus hidup aplikasi (startup, loop event, shutdown) dan state global. | `app.c`, `state.c` |
| [src/db/](src/db/) | Penghubung SQLite dan background indexer pemindaian folder lokal. | `sqlite.c`, `indexer.c`, `schema.sql` |
| [src/icon/](src/icon/) | Ekstraksi ikon dari API native OS serta manajemen cache tekstur GPU. | `icon.c`, `icon_cache.c`, `icon_os_macos.m` |
| [src/platform/](src/platform/) | Fungsi pembungkus native OS (deteksi platform dan pembacaan file). | `detection.c`, `fs.c`, `platform.c` |
| [src/render/](src/render/) | Penanganan inisialisasi renderer, gambar 2D primitive, dan tekstur teks native. | `gl_render.c`, `draw2d.c`, `platform_text_macos.m` |
| [src/search/](src/search/) | Logika pencarian kueri SQLite dan algoritma ranking berdasarkan relevansi. | `search.c`, `ranking.c` |
| [src/ui/](src/ui/) | Pemrosesan masukan keyboard, event cursor, dan rendering tata letak UI. | `ui.c`, `input.c` |


---

## 📜 Bagaimana Program Bekerja

Aplikasi berjalan dengan mengikuti alur kerja terstruktur untuk menjamin efisiensi tinggi pada setiap bagian:

### 1. Inisialisasi & Startup
Saat aplikasi dijalankan, siklus berikut akan terjadi:
- **SDL3 Initialization**: Sistem menginisialisasi subsistem video SDL3 dan membuat jendela borderless (`SDL_WINDOW_BORDERLESS`).
- **GPU Renderer Binding**: Membuat objek `SDL_Renderer` berakselerasi perangkat keras yang otomatis memanfaatkan Metal (macOS) atau Direct3D (Windows) di latar belakang.
- **Koneksi SQLite**: Membuka berkas `db/spotlight.db`. Pada jalannya aplikasi untuk pertama kali (*first-run*), program akan membuat struktur tabel berdasarkan `src/db/schema.sql` dan memicu **Background Indexer**.
- **Background Indexing**: Program memindai folder sistem (`/Applications` di Mac) dan direktori pengguna secara asinkron untuk mendata file, path, tipe, dan platform ke database.
- **Cache Preloading**: Menyiapkan struktur pencarian ikon GPU yang masih kosong.

### 2. Siklus Event Loop
Perulangan utama (`SDL_PollEvent`) menangani input pengguna secara responsif:
- Menangkap ketikan huruf demi huruf dan memperbarui buffer query pencarian di `AppState`.
- Mendeteksi penekanan tombol navigasi keyboard (panah `↑` dan `↓` untuk memindahkan pilihan baris, tombol `Enter` untuk mengeksekusi).
- Tombol `ESC` atau klik di luar area jendela akan menyembunyikan/menutup aplikasi secara instan.

### 3. Kueri Debounced & Perankingan
Ketika teks query diinputkan, pencarian tidak langsung ditembakkan ke database pada setiap frame render (untuk mencegah lag pengetikan). Sebagai gantinya:
- Sistem menunggu durasi diam pengetikan selama **100-200ms**.
- Query dikirimkan ke SQLite: `SELECT ... FROM items WHERE name LIKE ? LIMIT 10`.
- Hasil pencarian diurutkan menggunakan algoritma perankingan sederhana (`ranking.c`) untuk mengevaluasi kesamaan awalan huruf (prefix matching) dan panjang kata. 5 hasil terbaik disalin ke `AppState`.

### 4. Ekstraksi Ikon Native
Setiap baris hasil pencarian membutuhkan ikon aplikasi:
- UI meminta tekstur ikon ke `icon_cache`.
- Jika belum ter-cache (*cache miss*), program memanggil API native sistem operasi:
  - **macOS**: Menggunakan Cocoa API `[[NSWorkspace sharedWorkspace] iconForFile:path]` untuk mengambil pointer `NSImage` biner dari ikon berkas. `NSImage` ini kemudian di-rasterisasi ke bitmap buffer RGBA berukuran 32x32 piksel secara native.
  - **Windows**: Menggunakan Win32 API `SHGetFileInfoW` untuk memperoleh handle `HICON` yang kemudian disalin ke buffer piksel RGBA.
- Buffer piksel mentah tersebut dikonversi menjadi `SDL_Surface` menggunakan `SDL_CreateSurfaceFrom` lalu diunggah ke GPU menjadi `SDL_Texture*` (`SDL_CreateTextureFromSurface`).
- Tekstur ini disimpan permanen dalam cache untuk penggunaan ulang instan pada frame render berikutnya.

### 5. Rendering Teks Native
Untuk menggambar teks pencarian secara fleksibel tanpa keterbatasan ASCII:
- Program menggunakan modul `platform_text` native OS.
- Di macOS (`platform_text_macos.m`), teks query dikonversi menjadi `NSString` lalu digambar menggunakan objek `NSFont` sistem (**San Francisco 20pt**) dengan *antialiasing* native.
- Hasil rasterisasi font berupa bitmap langsung dibungkus menjadi `SDL_Texture` siap gambar. Hal ini memungkinkan dukungan penuh karakter Unicode UTF-8 secara native.

### 6. Eksekusi Peluncuran Aplikasi
Saat baris hasil dipilih dan tombol `Enter` tekanan, aplikasi memanggil fungsi native platform:
- macOS: Menggunakan API Cocoa untuk membuka file/aplikasi terkait di background secara bersih.
- Windows: Menggunakan instruksi `ShellExecuteA` untuk menjalankan berkas.

---

## 🚀 Panduan Build & Eksekusi (Clone-to-Run)

### Prasyarat Sistem
- **macOS**: Memiliki compiler `clang` (biasanya terpasang otomatis saat memasang Xcode Command Line Tools).
- **Windows**: Direkomendasikan memiliki `Git` dan compiler GCC.
- **Linux**: Memiliki compiler `gcc`/`clang` dan `cmake`.

### Langkah-Langkah

#### 1. Kloning Repositori
Jalankan perintah clone untuk mengunduh berkas proyek ke komputer Anda:
```bash
git clone https://github.com/faizulmushofa/atlas-launcher.git
cd spotlight_search
```

#### 2. Pasang Dependensi (`make install`)
Jalankan perintah berikut untuk mengunduh SDL3 dan SQLite ke folder lokal secara otomatis:
```bash
make install
```
> [!NOTE]  
> - Skrip instalasi secara otomatis mendeteksi ketersediaan SQLite di komputer Anda. Jika tidak ditemukan, skrip akan mengunduh paket kode sumber **SQLite amalgamation** secara otomatis.
> - Di macOS, skrip akan mendeteksi compiler `clang`. Jika belum terpasang, skrip otomatis memicu pop-up instalasi Command Line Tools resmi dari Apple via perintah `xcode-select --install`.
> - Di Windows, skrip secara otomatis mengunduh compiler GCC portable **w64devkit** ke dalam direktori lokal `external/w64devkit`.

#### 3. Kompilasi Proyek (`make build`)
Kompilasi seluruh kode sumber C menjadi file biner eksekutabel:
```bash
make build
```
Proses ini akan menghasilkan file eksekutabel bernama `spotlight_search` di dalam direktori `bin/`.

#### 4. Jalankan Aplikasi (`make run`)
Eksekusi program secara langsung melalui terminal Anda:
```bash
make run
```

#### 5. Bersihkan Hasil Build (`make clean`)
Menghapus file-file sementara hasil kompilasi agar direktori kembali bersih:
```bash
make clean
```

---

## ⚙️ Panduan Kustomisasi Rinci

Anda dapat dengan mudah mengubah tampilan dan dimensi antarmuka Spotlight Clone dengan memodifikasi nilai konfigurasi statis pada berkas kode sumber.

### 1. Mengubah Ukuran Lebar & Tinggi Default Window
Buka berkas **[src/core/app.c](src/core/app.c)** pada fungsi `app_run()`. Anda akan melihat konfigurasi lebar default:
```c
    app_window = SDL_CreateWindow(
        "Spotlight Search",
        800,  // <--- Ubah nilai 800 ini untuk menyesuaikan lebar window (pixel)
        100,  // Tinggi awal window (akan disesuaikan secara dinamis oleh dropdown)
        SDL_WINDOW_BORDERLESS
    );
```
Untuk kustomisasi tinggi dropdown hasil pencarian, sesuaikan perhitungannya di baris berikut pada berkas yang sama (`src/core/app.c`):
```c
        // Hitung tinggi target window secara dinamis berdasarkan jumlah baris hasil
        int target_h = 50; // Tinggi search bar utama (50px)
        AppState* state = state_get();
        if (state && state->result_count > 0) {
            // 50px (search bar) + (jumlah hasil * tinggi baris 40px) + offset 10px
            target_h = 50 + (state->result_count * 40) + 10; 
        }
```

### 2. Mengubah Radius Sudut Melengkung (Rounded Corners)
Untuk menyesuaikan tingkat kelengkungan sudut jendela utama, buka berkas **[src/render/gl_render.c](src/render/gl_render.c)** di dalam fungsi `gl_render_frame()`:
```c
    // Gambar Border Luar (Radius tumpul luar 12px)
    draw2d_fill_rounded_rect(g_renderer, 0, 0, w, h, 12, border_color);

    // Gambar Latar Belakang (Radius tumpul dalam 11px, inset 1px)
    draw2d_fill_rounded_rect(g_renderer, 1, 1, w - 2, h - 2, 11, bg_color);
```

### 3. Mengubah Skema Warna Tema (Warna Latar & Border)
Skema warna didefinisikan menggunakan format `SDL_Color` (RGBA, nilai `0` hingga `255`).
- **Warna Latar Belakang & Border**: Terletak di **[src/render/gl_render.c](src/render/gl_render.c)** (`gl_render_frame()`):
  ```c
  SDL_Color border_color = { 255, 255, 255, 255 }; // Putih padat
  SDL_Color bg_color = { 20, 24, 30, 255 };       // Gelap abu-biru solid (Dark Theme)
  ```
- **Warna Teks & Elemen UI**: Terletak di **[src/ui/ui.c](src/ui/ui.c)** (`ui_render()`):
  ```c
  SDL_Color color_active = { 240, 240, 245, 255 };       // Teks aktif (Putih perak)
  SDL_Color color_placeholder = { 90, 100, 120, 255 };   // Teks placeholder (Abu-abu teredam)
  SDL_Color color_cursor = { 64, 156, 255, 255 };        // Kursor mengetik (Biru Mac Style)
  SDL_Color color_divider = { 50, 55, 65, 255 };         // Garis batas pemisah dropdown
  SDL_Color color_highlight = { 64, 156, 255, 120 };     // Sorotan baris terpilih (Biru transparan)
  ```

### 4. Mengubah Tinggi Baris & Radius Highlight Dropdown
Buka berkas **[src/ui/ui.c](src/ui/ui.c)** pada fungsi `ui_render()`:
- **Tinggi Baris (Row Height)**:
  ```c
  int row_y = 50 + i * 40; // i * 40 menandakan tinggi tiap baris adalah 40px. Ubah 40 jika ingin lebih renggang.
  ```
- **Tinggi & Radius Sorotan Highlight**:
  ```c
  // w - 16 (margin kanan-kiri), tinggi sorotan 32px, radius kelengkungan sudut highlight 6px
  draw2d_fill_rounded_rect(renderer, 8, row_y + 4, window_w - 16, 32, 6, color_highlight);
  ```

### 5. Mengubah Batasan Limit Hasil Pencarian
Secara default, aplikasi menyimpan cache tekstur dan menyalin maksimal 5 hasil pencarian ke state utama. Jika ingin merubah batas limit pencarian ini menjadi 10 item:
1. Buka berkas **[src/ui/ui.h](src/ui/ui.h)** atau struktur array di **[src/ui/ui.c](src/ui/ui.c)**, sesuaikan ukuran cache array dari `5` ke `10`:
   ```c
   static SDL_Texture* g_result_name_textures[10] = {NULL};
   static int g_result_name_w[10] = {0};
   static int g_result_name_h[10] = {0};
   static SDL_Texture* g_result_type_textures[10] = {NULL};
   // ... sesuaikan perulangan pembersihan & rendering dari i < 5 ke i < 10 di ui.c
   ```
2. Buka berkas **[src/core/state.h](src/core/state.h)** dan perbarui kapasitas maksimal hasil kueri pada struct `AppState` (biasanya dibatasi oleh array `SearchResult results[5]` diubah menjadi `[10]`).
3. Buka berkas **[src/search/search.c](src/search/search.c)** dan ubah proses penyalinan data hasil pencarian kueri dari limit `5` menjadi `10`.

---

## 🧑‍💻 Penulis

* **Faizul Mushofa** - [faizulmushofa](https://github.com/faizulmushofa)
