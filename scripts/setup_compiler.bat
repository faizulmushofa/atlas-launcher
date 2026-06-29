@echo off
setlocal enabledelayedexpansion

:: Pastikan path sistem Windows dan PowerShell terdaftar di PATH
set "PATH=%SystemRoot%\System32;%SystemRoot%\System32\WindowsPowerShell\v1.0\;%PATH%"

echo [Spotlight Search] Memulai setup compiler lokal untuk Windows...
if not exist "%~dp0..\external" mkdir "%~dp0..\external"

if not exist "%~dp0..\external\w64devkit" (
    echo [Spotlight Search] Mengunduh compiler portable w64devkit GCC dan Make...
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; $url='https://github.com/skeeto/w64devkit/releases/download/v1.20.0/w64devkit-1.20.0.zip'; $output='%~dp0..\external\w64devkit.zip'; $wc=New-Object System.Net.WebClient; register-objectevent $wc DownloadProgressChanged -action { Write-Host -NoNewline ('`r[Spotlight Search] Mengunduh w64devkit: ' + $eventArgs.ProgressPercentage + '%%') } | Out-Null; $wc.DownloadFileAsync((New-Object System.Uri($url)), $output); while ($wc.IsBusy) { Start-Sleep -Milliseconds 100 }; Write-Host ''"
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
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; $url='https://github.com/libsdl-org/SDL/releases/download/release-3.4.10/SDL3-devel-3.4.10-mingw.zip'; $output='%~dp0..\external\sdl3_mingw.zip'; $wc=New-Object System.Net.WebClient; register-objectevent $wc DownloadProgressChanged -action { Write-Host -NoNewline ('`r[Spotlight Search] Mengunduh SDL3: ' + $eventArgs.ProgressPercentage + '%%') } | Out-Null; $wc.DownloadFileAsync((New-Object System.Uri($url)), $output); while ($wc.IsBusy) { Start-Sleep -Milliseconds 100 }; Write-Host ''"
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
    echo [Spotlight Search] Mengunduh SQLite amalgamation...
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; $url='https://github.com/azadkuh/sqlite-amalgamation/archive/refs/heads/master.zip'; $output='%~dp0..\external\sqlite3.zip'; $wc=New-Object System.Net.WebClient; register-objectevent $wc DownloadProgressChanged -action { Write-Host -NoNewline ('`r[Spotlight Search] Mengunduh SQLite: ' + $eventArgs.ProgressPercentage + '%%') } | Out-Null; $wc.DownloadFileAsync((New-Object System.Uri($url)), $output); while ($wc.IsBusy) { Start-Sleep -Milliseconds 100 }; Write-Host ''"
    if !ERRORLEVEL! neq 0 (
        echo [Spotlight Search] Gagal mengunduh SQLite. Pastikan koneksi internet aktif.
        exit /b 1
    )
    echo [Spotlight Search] Mengekstrak SQLite...
    powershell -Command "Expand-Archive -Path '%~dp0..\external\sqlite3.zip' -DestinationPath '%~dp0..\external'"
    del "%~dp0..\external\sqlite3.zip"
    
    if exist "%~dp0..\external\sqlite-amalgamation-master" (
        move "%~dp0..\external\sqlite-amalgamation-master" "%~dp0..\external\sqlite3" >nul
    )
    echo [Spotlight Search] SQLite berhasil dipasang di external\sqlite3!
) else (
    echo [Spotlight Search] SQLite sudah diunduh di external\sqlite3.
)

echo [Spotlight Search] Setup selesai! Anda bisa menggunakan build.bat untuk mengompilasi proyek.
