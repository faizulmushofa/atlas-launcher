# Cross-Platform Spotlight Clone (C + SDL2 + OpenGL + SQLite)

Repositori ini berisi implementasi proyek **Spotlight Search Clone** menggunakan bahasa pemrograman **C**, **SDL2/SDL3** untuk manajemen window dan input, **OpenGL** untuk rendering berakselerasi perangkat keras, dan **SQLite** untuk basis data pencarian real-time yang cepat.

Aplikasi ini dapat dibuka secara instan menggunakan hotkey global (`Ctrl + Space` atau `Cmd + Space` di macOS) dan memproses pencarian file/aplikasi secara real-time langsung dari basis data lokal dengan mengambil ikon asli langsung dari sistem operasi (*native OS icons*).

---

## 1. Visi Proyek

Membangun aplikasi Spotlight Search yang meniru perilaku macOS Spotlight:
* **Hotkey Global**: UI overlay yang dipicu oleh hotkey (`Ctrl + Space` / `Cmd + Space`).
* **UI Overlay**: Desain jendela mengambang minimalis dengan bar pencarian dan daftar hasil.
* **Pencarian Real-Time**: Pencarian cepat dari basis data SQLite menggunakan kueri teroptimasi.
* **Integrasi Ikon Native**: Mengambil ikon berkas asli langsung dari API sistem operasi (bukan aset gambar statis).
* **Multi-Platform**: Mendukung macOS (Cocoa/LaunchServices), Windows (Win32), dan Linux (freedesktop/.desktop).

---

## 2. Teknologi

* **Core Language**: C (C99/C11)
* **Window & Input**: SDL2 (atau SDL3)
* **Rendering**: OpenGL (2.1/3.3)
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
├── CMakeLists.txt
├── README.md
├── Makefile
├── .gitignore
├── .clangd
├── external/
│   ├── sdl2/
│   └── openGL/
│
├── database/
│   ├── spotlight.db
│   └── schema.sql
├── assets/
│   ├── shaders/
│   │   ├── vertex.glsl
│   │   └── fragment.glsl
│   ├── fonts/
│   │   └── default.ttf
│   └── icons/
│       └── default.png
├── build/
├── src/
│   ├── main.c
│   │
│   ├── core/
│   │   ├── app.c
│   │   ├── app.h
│   │   ├── state.c
│   │   ├── state.h
│   │   └── config.h
│   │
│   ├── platform/
│   │   ├── platform.c
│   │   ├── platform.h
│   │   ├── fs.c
│   │   ├── fs.h
│   │   ├── hotkey.c
│   │   └── hotkey.h
│   │
│   ├── db/
│   │   ├── sqlite.c
│   │   ├── sqlite.h
│   │   ├── indexer.c
│   │   ├── indexer.h
│   │   └── schema.sql
│   │
│   ├── search/
│   │   ├── search.c
│   │   ├── search.h
│   │   ├── ranking.c
│   │   └── ranking.h
│   │
│   ├── ui/
│   │   ├── ui.c
│   │   ├── ui.h
│   │   ├── input.c
│   │   ├── input.h
│   │   ├── layout.c
│   │   └── layout.h
│   │
│   ├── render/
│   │   ├── gl_renderer.c
│   │   ├── gl_renderer.h
│   │   ├── shader.c
│   │   ├── shader.h
│   │   ├── texture.c
│   │   ├── texture.h
│   │   ├── draw2d.c
│   │   └── draw2d.h
│   │
│   ├── icon/
│   │   ├── icon.c
│   │   ├── icon.h
│   │   ├── icon_os.c
│   │   ├── icon_os.h
│   │   ├── icon_cache.c
│   │   └── icon_cache.h
│   │
│   └── utils/
│       ├── string.c
│       ├── string.h
│       ├── logger.c
│       ├── logger.h
│       ├── timer.c
│       └── timer.h
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
│   SDL INPUT LAYER   │  -> Menangkap hotkey global & input pengetikan
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
│   OPENGL RENDERER   │  -> Menggambar UI, teks, dan tekstur ikon ke layar
└─────────────────────┘
```

### B. Startup Flow
1. Menginisialisasi subsistem **SDL2** dan membuat OpenGL context.
2. Membuka koneksi ke database SQLite (`spotlight.db`).
3. **First-Run Only**: Memindai filesystem lokal, mengekstrak informasi ikon, dan menyimpannya ke database SQLite.
4. Membuat cache tekstur OpenGL awal untuk ikon fallback.
5. Memasuki loop utama (*main loop*).

### C. Main Loop Flow
Pada setiap iterasi/frame:
1. Mendeteksi event SDL (`SDL_PollEvent`).
2. Jika hotkey `Ctrl+Space`/`Cmd+Space` ditekan, tampilkan/sembunyikan (toggle) overlay UI.
3. Jika teks kueri berubah:
   * Jalankan kueri pencarian ke database SQLite (dilakukan secara **debounced** 100-200ms agar database tidak terbebani).
4. Melakukan proses render grafis:
   * Menggambar latar belakang semi-transparan (overlay).
   * Menggambar kotak pencarian (search bar) dan teks input.
   * Menggambar daftar baris hasil pencarian beserta ikonnya yang dimuat dari cache tekstur OpenGL.
5. Memanggil `SDL_GL_SwapWindow()` untuk menampilkan hasil render ke layar.

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
> * **✓ Debounce Input**: Kueri pencarian ditunda selama 100-200ms setelah user mengetik untuk menghindari overhead kueri SQLite.
> * **✓ Icon Caching**: Pixel buffer ikon yang diekstrak dari OS langsung diubah menjadi Tekstur OpenGL sekali saja, lalu disimpan dalam *texture cache hash map* untuk digambar ulang secara instan.
> * **✓ Preloading**: Data penting di-load dari SQLite saat startup.

---

## 7. Panduan Build & Eksekusi

### A. Clone Dependensi SDL2 (Lokal)
```bash
git clone --depth 1 -b release-2.30.4 https://github.com/libsdl-org/SDL.git external/sdl2
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
