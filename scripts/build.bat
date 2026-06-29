@echo off
setlocal enabledelayedexpansion

:: 1. Cari compiler lokal w64devkit
if exist "%~dp0..\external\w64devkit\bin" (
    set "PATH=%~dp0..\external\w64devkit\bin;%PATH%"
    echo [Spotlight Search] Menggunakan compiler lokal w64devkit dari external\w64devkit.
) else (
    echo [Spotlight Search] PERINGATAN: Compiler lokal tidak ditemukan di external\w64devkit.
    echo [Spotlight Search] Mencoba menggunakan compiler global dari sistem PATH...
)

:: 2. Proses parameter
if "%1"=="clean" (
    echo [Spotlight Search] Pembersihan build dan bin folder...
    if exist "%~dp0..\build" rmdir /s /q "%~dp0..\build"
    if exist "%~dp0..\bin" rmdir /s /q "%~dp0..\bin"
    echo [Spotlight Search] Clean selesai.
) else if "%1"=="run" (
    if exist "%~dp0..\bin\spotlight_search.exe" (
        echo [Spotlight Search] Menjalankan aplikasi...
        "%~dp0..\bin\spotlight_search.exe"
    ) else (
        echo [Spotlight Search] Eksekutabel tidak ditemukan. Silakan jalankan 'build.bat' terlebih dahulu.
    )
) else (
    :: Periksa apakah cmake ada di sistem
    where cmake >nul 2>nul
    if !ERRORLEVEL! equ 0 (
        echo [Spotlight Search] Mengonfigurasi build dengan CMake...
        cmake -B "%~dp0..\build" -S "%~dp0.." -G "Unix Makefiles" -DCMAKE_C_COMPILER=gcc
        if !ERRORLEVEL! neq 0 (
            echo [Spotlight Search] Gagal konfigurasi CMake. Mencoba kompilasi langsung dengan GCC...
            goto :gcc_build
        )
        echo [Spotlight Search] Memulai proses kompilasi dengan CMake...
        cmake --build "%~dp0..\build"
        if !ERRORLEVEL! neq 0 (
            echo [Spotlight Search] Gagal kompilasi proyek dengan CMake.
            exit /b 1
        )
        if not exist "%~dp0..\bin" mkdir "%~dp0..\bin"
        copy /y "%~dp0..\build\spotlight_search.exe" "%~dp0..\bin\spotlight_search.exe" >nul
        
        :: Salin DLL jika ada
        if exist "%~dp0..\external\sdl3\bin\SDL3.dll" (
            copy /y "%~dp0..\external\sdl3\bin\SDL3.dll" "%~dp0..\bin\SDL3.dll" >nul
        )
        echo [Spotlight Search] Build sukses! Executable disalin ke bin\spotlight_search.exe
    ) else (
        :gcc_build
        echo [Spotlight Search] CMake tidak ditemukan. Melakukan kompilasi langsung menggunakan GCC...
        if not exist "%~dp0..\bin" mkdir "%~dp0..\bin"
        
        :: Kompilasi langsung semua source file
        gcc -O3 -Wall -Wextra ^
            "%~dp0..\src\main.c" ^
            "%~dp0..\src\core\app.c" ^
            "%~dp0..\src\core\state.c" ^
            "%~dp0..\src\render\gl_render.c" ^
            "%~dp0..\src\render\draw2d.c" ^
            "%~dp0..\src\platform\detection.c" ^
            "%~dp0..\src\platform\fs.c" ^
            "%~dp0..\src\ui\input.c" ^
            "%~dp0..\src\ui\ui.c" ^
            "%~dp0..\src\db\sqlite.c" ^
            "%~dp0..\src\db\indexer.c" ^
            "%~dp0..\src\icon\icon.c" ^
            "%~dp0..\src\icon\icon_cache.c" ^
            "%~dp0..\src\search\search.c" ^
            "%~dp0..\src\search\ranking.c" ^
            "%~dp0..\src\render\platform_text_windows.c" ^
            "%~dp0..\src\platform\platform.c" ^
            "%~dp0..\external\sqlite3\sqlite3.c" ^
            -I"%~dp0..\src" -I"%~dp0..\external\sqlite3" -I"%~dp0..\external\sdl3\include" -L"%~dp0..\external\sdl3\lib" ^
            -lSDL3 -lkernel32 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid ^
            -o "%~dp0..\bin\spotlight_search.exe"
            
        if !ERRORLEVEL! neq 0 (
            echo [Spotlight Search] Gagal kompilasi proyek dengan GCC langsung.
            exit /b 1
        )
        
        :: Salin DLL pendukung agar program bisa berjalan
        if exist "%~dp0..\external\sdl3\bin\SDL3.dll" (
            copy /y "%~dp0..\external\sdl3\bin\SDL3.dll" "%~dp0..\bin\SDL3.dll" >nul
        )
        echo [Spotlight Search] Build sukses! Executable disalin ke bin\spotlight_search.exe
    )
)
