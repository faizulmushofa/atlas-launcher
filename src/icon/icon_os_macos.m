#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

unsigned char* macos_get_app_icon_rgba(const char* app_path, int* out_width, int* out_height) {
    @autoreleasepool {
        NSString* path = [NSString stringWithUTF8String:app_path];
        NSImage* image = [[NSWorkspace sharedWorkspace] iconForFile:path];
        if (!image) return NULL;

        NSSize targetSize = NSMakeSize(32, 32);
        [image setSize:targetSize];

        // Buat bitmap image representation kosong berukuran 32x32
        NSBitmapImageRep* rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                                        pixelsWide:32
                                                                        pixelsHigh:32
                                                                     bitsPerSample:8
                                                                   samplesPerPixel:4
                                                                          hasAlpha:YES
                                                                          isPlanar:NO
                                                                    colorSpaceName:NSCalibratedRGBColorSpace
                                                                       bytesPerRow:32 * 4
                                                                      bitsPerPixel:32];
        if (!rep) return NULL;

        // Render NSImage ke dalam NSBitmapImageRep yang baru dibuat
        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:rep]];
        [image drawInRect:NSMakeRect(0, 0, 32, 32)
                 fromRect:NSZeroRect
                operation:NSCompositingOperationCopy
                 fraction:1.0];
        [NSGraphicsContext restoreGraphicsState];

        unsigned char* pixels = (unsigned char*)malloc(32 * 32 * 4);
        if (!pixels) {
            return NULL;
        }

        // Salin data piksel hasil rasterisasi
        unsigned char* bitmapData = [rep bitmapData];
        if (bitmapData) {
            memcpy(pixels, bitmapData, 32 * 32 * 4);
        } else {
            free(pixels);
            return NULL;
        }

        *out_width = 32;
        *out_height = 32;
        return pixels;
    }
}
