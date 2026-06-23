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
    echo [Spotlight Search] Mengonfigurasi build dengan CMake...
    cmake -B "%~dp0..\build" -S "%~dp0.." -G "Unix Makefiles" -DCMAKE_C_COMPILER=gcc
    if !ERRORLEVEL! neq 0 (
        echo [Spotlight Search] Gagal konfigurasi CMake.
        exit /b 1
    )
    echo [Spotlight Search] Memulai proses kompilasi...
    cmake --build "%~dp0..\build"
    if !ERRORLEVEL! neq 0 (
        echo [Spotlight Search] Gagal kompilasi proyek.
        exit /b 1
    )
    if not exist "%~dp0..\bin" mkdir "%~dp0..\bin"
    copy /y "%~dp0..\build\spotlight_search.exe" "%~dp0..\bin\spotlight_search.exe" >nul
    echo [Spotlight Search] Build sukses! Executable disalin ke bin\spotlight_search.exe
)
