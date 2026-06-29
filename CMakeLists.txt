cmake_minimum_required(VERSION 3.14)
project(spotlight_search C)

if(APPLE)
    enable_language(OBJC)
endif()

# Ekspor berkas kompilasi untuk membantu Intellisense IDE menemukan header
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Konfigurasi output direktori build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

# Manajemen pustaka eksternal (SDL3)
set(SDL_SHARED ON CACHE BOOL "Build shared SDL library" FORCE)
set(SDL_STATIC OFF CACHE BOOL "Build static SDL library" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Build SDL test applications" FORCE)

add_subdirectory(external/sdl3)

# Integrasi SQLite3 (Gunakan sistem jika ada, jika tidak gunakan amalgamation)
find_package(SQLite3 QUIET)

if(SQLite3_FOUND)
    message(STATUS "[Spotlight Search] SQLite3 ditemukan di sistem: ${SQLite3_LIBRARIES}")
    add_library(sqlite3_lib INTERFACE)
    target_link_libraries(sqlite3_lib INTERFACE SQLite3::SQLite3)
else()
    message(STATUS "[Spotlight Search] SQLite3 tidak ditemukan di sistem. Menggunakan external/sqlite3...")
    add_library(sqlite3_lib STATIC external/sqlite3/sqlite3.c)
    target_include_directories(sqlite3_lib PUBLIC external/sqlite3)
    if(NOT WIN32)
        target_link_libraries(sqlite3_lib PRIVATE pthread ${CMAKE_DL_LIBS})
    endif()
endif()

# File sumber (sources)
set(SOURCES
    src/main.c
    src/core/app.c
    src/core/state.c
    src/render/gl_render.c
    src/render/draw2d.c
    src/platform/detection.c
    src/platform/fs.c
    src/ui/input.c
    src/ui/ui.c
    src/db/sqlite.c
    src/db/indexer.c
    src/icon/icon.c
    src/icon/icon_cache.c
    src/search/search.c
    src/search/ranking.c
)

# Integrasi file sumber berdasarkan Platform OS
if(WIN32)
    list(APPEND SOURCES src/render/platform_text_windows.c src/platform/platform.c)
elseif(APPLE)
    list(APPEND SOURCES src/render/platform_text_macos.m src/icon/icon_os_macos.m src/platform/platform_macos.m)
else()
    # Linux
    list(APPEND SOURCES src/platform/platform.c)
endif()

# Target executable & linking
add_executable(spotlight_search ${SOURCES})

target_include_directories(spotlight_search PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/sdl3/include"
)

target_link_libraries(spotlight_search PRIVATE
    SDL3-shared
    sqlite3_lib
)

if(APPLE)
    target_link_libraries(spotlight_search PRIVATE "-framework Cocoa" "-framework AppKit")
endif()

# Target run & install_deps menggunakan script dari folder scripts
if(WIN32)
    add_custom_target(run
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build.bat run
        DEPENDS spotlight_search
        COMMENT "Menjalankan spotlight_search..."
    )
    add_custom_target(install_deps
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/setup_compiler.bat
        COMMENT "Mengunduh & mengonfigurasi compiler lokal..."
    )
else()
    add_custom_target(run
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build.sh run
        DEPENDS spotlight_search
        COMMENT "Menjalankan spotlight_search..."
    )
    add_custom_target(install_deps
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/setup_compiler.sh
        COMMENT "Mengonfigurasi compiler lokal..."
    )
endif()
