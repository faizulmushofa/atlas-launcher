# Cross-Platform Spotlight Clone (C + SDL3 + SDL_GPU + SQLite)

Repositori ini berisi implementasi proyek **Spotlight Search Clone** menggunakan bahasa pemrograman **C**, **SDL3** untuk manajemen window dan input, **SDL_GPU** (fitur GPU API modern bawaan SDL3) untuk rendering berakselerasi perangkat keras, dan **SQLite** untuk basis data pencarian real-time yang cepat.

Aplikasi ini dapat dibuka secara instan menggunakan hotkey global (`Ctrl + Space` atau `Cmd + Space` di macOS) dan memproses pencarian file/aplikasi secara real-time langsung dari basis data lokal dengan mengambil ikon asli langsung dari sistem operasi (*native OS icons*).

---

## 1. Visi Proyek

Membangun aplikasi Spotlight Search yang meniru perilaku macOS Spotlight:
* **Hotkey Global**: UI overlay yang dipicu oleh hotkey (`Ctrl + Space` / `Cmd + Space`).
* **UI Overlay**: Desain jendela mengambang minimalis dengan bar pencarian dan daftar hasil.
* **Pencarian Real-Time**: Pencarian cepat dari basis data SQLite menggunakan kueri teroptimasi.
* **Integrasi Ikon Native**: Mengambil ikon berkas asli langsung dari API sistem operasi.
* **Multi-Platform**: Mendukung macOS (Cocoa/LaunchServices), Windows (Win32), dan Linux (freedesktop/.desktop).

---

## 2. Teknologi

* **Core Language**: C (C99/C11)
* **Window, Input & Graphics**: SDL3 (menggunakan modul rendering modern **SDL_GPU**)
* **Database**: SQLite3
* **OS Integration (Native Icons & Hotkeys)**:
  * **Windows**: Win32 API (`ExtractIconEx` / `SHGetFileInfo`)
  * **macOS**: Cocoa / `NSWorkspace` / `LaunchServices`
  * **Linux**: Freedesktop `.desktop` files & pencarian tema ikon sistem

---

## 3. Struktur Direktori Proyek

Struktur folder proyek ini diatur secara modular sesuai dengan standar arsitektur clean-code (sebagaimana ditentukan dalam `directory-structure.txt`):

```text
spotlight_search/
├── CMakeLists.txt         # Konfigurasi sistem build CMake (Target: SDL3)
├── Makefile               # Perintah pintas untuk kompilasi & eksekusi (make install/build/run)
├── README.md              # Dokumentasi proyek ini
├── .gitignore             # Mengabaikan file hasil kompilasi & cache IDE
├── .clangd                # Konfigurasi untuk Intellisense editor (C/C++ clangd)
├── database/
│   ├── spotlight.db       # File database SQLite utama
│   └── schema.sql         # Skema struktur tabel database
├── assets/
│   ├── shaders/
│   │   ├── vertex.glsl    # GLSL Vertex Shader
│   │   └── fragment.glsl  # GLSL Fragment Shader
│   ├── fonts/
│   │   └── default.ttf    # Font bawaan untuk rendering teks
│   └── icons/
│       └── default.png    # Ikon fallback jika ikon OS tidak ditemukan
├── build/
├── external/
│   ├── sdl3/              # Kode sumber SDL3 (lokal per proyek)
│   └── openGL/            # Pustaka/Loader OpenGL eksternal jika diperlukan
└── src/
    ├── main.c             # Titik masuk aplikasi (sangat tipis)
    ├── core/
    │   ├── app.c          # Manajemen window, loop utama, dan siklus hidup aplikasi
    │   ├── app.h          
    │   ├── state.c        # Pengelolaan state aplikasi (query, hasil pencarian)
    │   ├── state.h        
    │   └── config.h       # Makro konfigurasi statis (ukuran window, warna, dll)
    ├── platform/
    │   ├── platform.c     # Deteksi platform dan fungsi sistem operasi abstrak
    │   ├── platform.h
    │   ├── fs.c           # Operasi pembacaan sistem berkas
    │   ├── fs.h
    │   ├── hotkey.c       # Pendaftaran hotkey global level OS
    │   └── hotkey.h
    ├── db/
    │   ├── sqlite.c       # Penghubung database SQLite
    │   ├── sqlite.h
    │   ├── indexer.c      # Background indexer untuk mendata file ke database
    │   ├── indexer.h
    │   └── schema.sql     # Skema database SQLite lokal
    ├── search/
    │   ├── search.c       # Logika kueri pencarian SQLite
    │   ├── search.h
    │   ├── ranking.c      # Algoritma perankingan hasil pencarian
    │   └── ranking.h
    ├── ui/
    │   ├── ui.c           # Struktur antarmuka pengguna
    │   ├── ui.h
    │   ├── input.c        # Pemrosesan input teks
    │   ├── input.h
    │   ├── layout.c       # Penghitungan layout baris hasil
    │   └── layout.h
    ├── render/
    │   ├── gl_renderer.c  # Pengaturan SDL_GPUDevice & swapchain target rendering
    │   ├── gl_renderer.h
    │   ├── shader.c       # Pembuat dan pengompilasi Shader GPU
    │   ├── shader.h
    │   ├── texture.c      # Pemuat pixel buffer ke OpenGL/GPU texture
    │   ├── texture.h
    │   ├── draw2d.c       # Fungsi utilitas penggambaran bentuk 2D primitive
    │   └── draw2d.h
    ├── icon/
    │   ├── icon.c         # Interface utama ekstraksi ikon
    │   ├── icon.h
    │   ├── icon_os.c      # Implementasi spesifik OS untuk ekstraksi ikon
    │   ├── icon_os.h
    │   ├── icon_cache.c   # Cache texture GPU agar tidak memuat ulang ikon
    │   └── icon_cache.h
    └── utils/
        ├── string.c       # Utilitas pemrosesan string C
        ├── string.h
        ├── logger.c       # Logging debug konsol
        ├── logger.h
        ├── timer.c        # Utilitas pengukuran waktu eksekusi
        └── timer.h
```

---

## 4. Desain Database (SQLite)

Tabel basis data utama bernama `items` dirancang untuk menampung indeks pencarian dengan struktur sebagai berikut:

```sql
CREATE TABLE items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT,             -- Nama file atau aplikasi (misal: "Safari", "Notepad")
    path TEXT,             -- Path lengkap file/aplikasi di sistem operasi
    type TEXT,             -- Jenis item (misal: "app", "file", "directory")
    icon_handle TEXT,      -- Token pengenal ikon (cached ID / path ikon)
    platform TEXT          -- Target OS (windows, linux, mac, atau all)
);
```

**Kueri Pencarian Utama (Search Query):**
Pencarian dilakukan secara dinamis menggunakan klausa `LIKE` dengan batasan 10 hasil teratas:
```sql
SELECT * FROM items
WHERE name LIKE '%' || :query || '%'
AND (platform = :current_platform OR platform = 'all')
LIMIT 10;
```

---

## 5. Arsitektur Sistem & Alur Kerja

### A. Arsitektur Data Flow
```text
┌─────────────────────┐
│   SDL INPUT LAYER   │  -> Menangkap hotkey global & input pengetikan (SDL3)
└─────────┬───────────┘
          ↓
┌─────────────────────┐
│     APP STATE       │  -> Menyimpan query pencarian & status UI
└─────────┬───────────┘
          ↓
┌─────────────────────┐
│    SEARCH ENGINE    │  -> Melakukan kueri ke SQLite & ranking hasil (debounced)
└─────────┬───────────┘
          ↓
┌─────────────────────┐
│   ICON ABSTRACTION  │  -> Menjembatani permintaan ikon file ke sistem operasi
└─────────┬───────────┘
          ↓
┌─────────────────────┐
│   OS ICON BACKEND   │  -> Mengambil raw pixel icon melalui API native OS
└─────────┬───────────┘
          ↓
┌─────────────────────┐
│   SDL_GPU RENDERER  │  -> Menggambar UI, tekstur ikon, dan teks via GPU API
└─────────────────────┘
```

### B. Startup Flow
1. Menginisialisasi subsistem **SDL3** (`SDL_Init(SDL_INIT_VIDEO)`).
2. Membuat window utama dan mengklaim window tersebut ke perangkat GPU (`SDL_CreateGPUDevice` & `SDL_ClaimWindowForGPUDevice`).
3. Membuka koneksi ke database SQLite (`spotlight.db`).
4. **First-Run Only**: Memindai filesystem lokal, mengekstrak informasi ikon, dan menyimpannya ke database SQLite.
5. Membuat cache tekstur GPU awal untuk ikon fallback.
6. Memasuki loop utama (*main loop*).

### C. Main Loop Flow
Pada setiap iterasi/frame:
1. Mendeteksi event SDL3 (`SDL_PollEvent`).
2. Jika hotkey `Ctrl+Space`/`Cmd+Space` ditekan, tampilkan/sembunyikan (toggle) overlay UI.
3. Jika teks kueri berubah:
   * Jalankan kueri pencarian ke database SQLite (dilakukan secara **debounced** 100-200ms).
4. Melakukan proses render grafis menggunakan SDL_GPU:
   * Mengambil *Command Buffer* (`SDL_AcquireGPUCommandBuffer`).
   * Mengambil tekstur swapchain aktif milik window (`SDL_AcquireGPUSwapchainTexture`).
   * Memulai *Render Pass* (`SDL_BeginGPURenderPass`) dengan membersihkan warna latar belakang menjadi warna gelap solid.
   * Melakukan rendering UI (daftar baris hasil pencarian beserta ikonnya).
   * Mengakhiri *Render Pass* (`SDL_EndGPURenderPass`) dan menyerahkan antrean perintah ke GPU (`SDL_SubmitGPUCommandBuffer`).

---

## 6. Aturan Optimasi & Hal yang Wajib Dihindari

> [!CAUTION]
> **Hal yang Wajib Dihindari (X):**
> * **X** Melakukan kueri database SQLite pada setiap frame render.
> * **X** Melakukan pemindaian filesystem (*filesystem scanning*) terus-menerus saat aplikasi aktif.
> * **X** Melakukan ekstraksi/pemuatan ikon dari sistem operasi pada setiap frame render.
> * **X** Mencampurkan kode spesifik OS (platform logic) ke dalam core engine aplikasi.
> * **X** Mencampurkan logika tampilan antarmuka (UI logic) dengan database logic.

> [!TIP]
> **Aturan Optimasi Bawaan (✓):**
> * **✓ Debounce Input**: Kueri pencarian ditunda selama 100-200ms setelah mengetik untuk menghindari overhead kueri SQLite.
> * **✓ Icon Caching**: Pixel buffer ikon yang diekstrak dari OS langsung diubah menjadi Tekstur GPU sekali saja, lalu disimpan dalam *texture cache* untuk digambar ulang secara instan.
> * **✓ Preloading**: Data penting di-load dari SQLite saat startup.

---

## 7. Panduan Build & Eksekusi

### A. Unduh Dependensi SDL3 secara Otomatis
Jalankan perintah ini dari root direktori proyek (`spotlight_search`) untuk mengunduh kode sumber SDL3 secara lokal:
```bash
make install
```

### B. Kompilasi (Build) Proyek
```bash
make build
```

### C. Jalankan Aplikasi (Run)
```bash
make run
```

### D. Bersihkan Direktori Build (Clean)
```bash
make clean
```

---

## 8. Konfigurasi Ukuran Window & Tampilan

Anda dapat mengatur dimensi lebar (width) dan tinggi (height) window pada berkas **`src/core/app.c`** di bagian pembuatan window (pemanggilan fungsi `SDL_CreateWindow` sekitar baris 27-32):

```c
    app_window = SDL_CreateWindow(
        "Spotlight Search",
        800,  // <--- Lebar window (Width)
        100,  // <--- Tinggi window (Height)
        SDL_WINDOW_BORDERLESS // <--- Membuat window tidak memiliki frame/bingkai bawaan OS
    );
```
