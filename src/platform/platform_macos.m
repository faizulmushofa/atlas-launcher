#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#include "platform.h"

bool platform_open_app(const char* path) {
    @autoreleasepool {
        NSString* path_str = [NSString stringWithUTF8String:path];
        NSURL* url = [NSURL fileURLWithPath:path_str];
        if (!url) return false;
        
        // Membuka aplikasi menggunakan NSWorkspace
        return [[NSWorkspace sharedWorkspace] openURL:url];
    }
}
