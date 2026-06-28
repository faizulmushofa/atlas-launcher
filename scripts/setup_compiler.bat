@echo off
setlocal enabledelayedexpansion

echo [Spotlight Search] Memulai setup compiler lokal untuk Windows...
if not exist "%~dp0..\external" mkdir "%~dp0..\external"

if not exist "%~dp0..\external\w64devkit" (
    echo [Spotlight Search] Mengunduh compiler portable w64devkit (GCC/Make)...
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://github.com/skeeto/w64devkit/releases/download/v2.0.0/w64devkit-2.0.0.zip' -OutFile '%~dp0..\external\w64devkit.zip'"
    if !ERRORLEVEL! neq 0 (
        echo [Spotlight Search] Gagal mengunduh compiler. Pastikan koneksi internet aktif.
        exit /b 1
    )
    echo [Spotlight Search] Mengekstrak compiler ke external\w64devkit...
    powershell -Command "Expand-Archive -Path '%~dp0..\external\w64devkit.zip' -DestinationPath '%~dp0..\external'"
    del "%~dp0..\external\w64devkit.zip"
    echo [Spotlight Search] Compiler berhasil dipasang di external\w64devkit!
)

if not exist "%~dp0..\external\sdl3" (
    echo [Spotlight Search] Mengunduh SDL3 dari github.com ke external\sdl3...
    if exist "%~dp0..\external\sdl2" rmdir /s /q "%~dp0..\external\sdl2"
    if exist "%~dp0..\external\sdl3" rmdir /s /q "%~dp0..\external\sdl3"
    if not exist "%~dp0..\external" mkdir "%~dp0..\external"
    git clone --depth 1 -b main https://github.com/libsdl-org/SDL.git "%~dp0..\external\sdl3"
) else (
    echo [Spotlight Search] SDL3 sudah diunduh di external\sdl3.
)

if not exist "%~dp0..\external\sqlite3" (
    echo [Spotlight Search] Mengunduh SQLite amalgamation dari github.com ke external\sqlite3...
    if exist "%~dp0..\external\sqlite3" rmdir /s /q "%~dp0..\external\sqlite3"
    git clone --depth 1 https://github.com/azadkuh/sqlite-amalgamation.git "%~dp0..\external\sqlite3"
) else (
    echo [Spotlight Search] SQLite sudah diunduh di external\sqlite3.
)

echo [Spotlight Search] Setup selesai! Anda bisa menggunakan build.bat untuk mengompilasi proyek.
