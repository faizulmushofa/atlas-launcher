#include "detection.h"

PlatformOsType platform_get_os_type(void) {
#if defined(PLATFORM_OS_WINDOWS)
    return PLATFORM_OS_TYPE_WINDOWS;
#elif defined(PLATFORM_OS_MACOS)
    return PLATFORM_OS_TYPE_MACOS;
#elif defined(PLATFORM_OS_LINUX)
    return PLATFORM_OS_TYPE_LINUX;
#else
    return PLATFORM_OS_TYPE_UNKNOWN;
#endif
}

const char* platform_get_os_name(void) {
#if defined(PLATFORM_OS_WINDOWS)
    return "Windows";
#elif defined(PLATFORM_OS_MACOS)
    return "macOS";
#elif defined(PLATFORM_OS_LINUX)
    return "Linux";
#else
    return "Unknown OS";
#endif
}
