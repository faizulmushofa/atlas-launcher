#ifndef PLATFORM_DETECTION_H
#define PLATFORM_DETECTION_H

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_OS_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
    #define PLATFORM_OS_MACOS 1
#elif defined(__linux__)
    #define PLATFORM_OS_LINUX 1
#else
    #define PLATFORM_OS_UNKNOWN 1
#endif

typedef enum {
    PLATFORM_OS_TYPE_WINDOWS,
    PLATFORM_OS_TYPE_MACOS,
    PLATFORM_OS_TYPE_LINUX,
    PLATFORM_OS_TYPE_UNKNOWN
} PlatformOsType;

/**
 * Mendapatkan tipe sistem operasi saat ini secara runtime.
 * @return Nilai PlatformOsType yang sesuai.
 */
PlatformOsType platform_get_os_type(void);

/**
 * Mendapatkan string nama sistem operasi saat ini secara runtime.
 * @return String nama OS ("Windows", "macOS", "Linux", atau "Unknown OS").
 */
const char* platform_get_os_name(void);

#endif // PLATFORM_DETECTION_H
