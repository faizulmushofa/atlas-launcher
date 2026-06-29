@echo off
setlocal enabledelayedexpansion

echo [Spotlight Search] Memulai orkestrasi build Windows...

:: Cek apakah dependensi dasar atau compiler portable belum siap
set "NEED_SETUP=0"
if not exist "%~dp0external\w64devkit" set "NEED_SETUP=1"
if not exist "%~dp0external\sdl3" set "NEED_SETUP=1"
if not exist "%~dp0external\sqlite3" set "NEED_SETUP=1"

if "!NEED_SETUP!"=="1" (
    echo [Spotlight Search] Dependensi atau compiler lokal belum lengkap.
    echo [Spotlight Search] Menjalankan setup otomatis...
    call "%~dp0scripts\setup_compiler.bat"
    if !ERRORLEVEL! neq 0 (
        echo [Spotlight Search] Gagal melakukan setup otomatis compiler/dependensi.
        exit /b 1
    )
)

:: Jalankan build dengan melewatkan argumen yang diberikan pengguna
call "%~dp0scripts\build.bat" %*
if !ERRORLEVEL! neq 0 (
    echo [Spotlight Search] Gagal melakukan build proyek.
    exit /b 1
)

exit /b 0
