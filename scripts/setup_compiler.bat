@echo off
setlocal enabledelayedexpansion

:: Pastikan path sistem Windows dan PowerShell terdaftar di PATH
set "PATH=%SystemRoot%\System32;%SystemRoot%\System32\WindowsPowerShell\v1.0\;%PATH%"

echo [Spotlight Search] Memulai setup compiler lokal untuk Windows...
if not exist "%~dp0..\external" mkdir "%~dp0..\external"

if not exist "%~dp0..\external\w64devkit" (
    echo [Spotlight Search] Mengunduh compiler portable w64devkit GCC dan Make...
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

rem Periksa apakah SDL3 precompiled sudah ada. Jika folder ada tapi tidak ada libSDL3.dll.a seperti sisa klon git lama, kita hapus dan unduh ulang.
if exist "%~dp0..\external\sdl3" (
    if not exist "%~dp0..\external\sdl3\lib\libSDL3.dll.a" (
        echo [Spotlight Search] Menghapus folder sisa klon git SDL3 lama...
        rmdir /s /q "%~dp0..\external\sdl3"
    )
)

if not exist "%~dp0..\external\sdl3" (
    echo [Spotlight Search] Mengunduh precompiled SDL3 development library...
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://github.com/libsdl-org/SDL/releases/download/release-3.2.0/SDL3-devel-3.2.0-mingw.zip' -OutFile '%~dp0..\external\sdl3_mingw.zip'"
    if !ERRORLEVEL! neq 0 (
        echo [Spotlight Search] Gagal mengunduh SDL3. Pastikan koneksi internet aktif.
        exit /b 1
    )
    echo [Spotlight Search] Mengekstrak SDL3...
    powershell -Command "Expand-Archive -Path '%~dp0..\external\sdl3_mingw.zip' -DestinationPath '%~dp0..\external'"
    del "%~dp0..\external\sdl3_mingw.zip"
    
    rem Salin isi folder x86_64-w64-mingw32 ke external\sdl3 secara dinamis
    set "FOUND_SDL3_DIR="
    for /d %%d in ("%~dp0..\external\SDL3-*") do (
        if exist "%%d\x86_64-w64-mingw32" (
            set "FOUND_SDL3_DIR=%%d"
        )
    )
    if defined FOUND_SDL3_DIR (
        xcopy /e /i /y "!FOUND_SDL3_DIR!\x86_64-w64-mingw32" "%~dp0..\external\sdl3" >nul
        rmdir /s /q "!FOUND_SDL3_DIR!"
        echo [Spotlight Search] SDL3 berhasil dipasang di external\sdl3!
    ) else (
        echo [Spotlight Search] Gagal menemukan folder hasil ekstrak SDL3.
        exit /b 1
    )
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
