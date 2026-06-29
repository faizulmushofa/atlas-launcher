#import <Cocoa/Cocoa.h>
#include <SDL3/SDL.h>

SDL_Texture* platform_create_text_texture_macos(SDL_Renderer* renderer, const char* text, SDL_Color color, int* out_w, int* out_h) {
    @autoreleasepool {
        NSString* ns_string = [NSString stringWithUTF8String:text];
        if (!ns_string || [ns_string length] == 0) {
            return NULL;
        }

        // Menggunakan font sistem macOS default (San Francisco) dengan ukuran premium 20pt
        NSFont* font = [NSFont systemFontOfSize:20.0];
        if (!font) {
            font = [NSFont userFontOfSize:20.0];
        }
        
        // Konversi warna SDL_Color ke NSColor
        NSColor* ns_color = [NSColor colorWithRed:color.r / 255.0 
                                            green:color.g / 255.0 
                                             blue:color.b / 255.0 
                                            alpha:color.a / 255.0];

        NSDictionary* attributes = @{
            NSFontAttributeName: font,
            NSForegroundColorAttributeName: ns_color
        };

        // Hitung ukuran yang dibutuhkan oleh teks
        NSSize size = [ns_string sizeWithAttributes:attributes];
        int width = (int)ceil(size.width);
        int height = (int)ceil(size.height);
        
        if (width <= 0 || height <= 0) return NULL;

        // Membuat representasi bitmap gambar RGBA 32-bit
        NSBitmapImageRep* rep = [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:NULL
                          pixelsWide:width
                          pixelsHigh:height
                       bitsPerSample:8
                     samplesPerPixel:4
                            hasAlpha:YES
                            isPlanar:NO
                      colorSpaceName:NSDeviceRGBColorSpace
                        bitmapFormat:0 // format default RGBA
                         bytesPerRow:width * 4
                        bitsPerPixel:32];

        if (!rep) return NULL;

        // Buat konteks grafis Cocoa untuk menggambar teks ke dalam bitmap
        NSGraphicsContext* context = [NSGraphicsContext graphicsContextWithBitmapImageRep:rep];
        if (!context) return NULL;

        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:context];

        // Gambar teks menggunakan antialiasing native macOS
        [ns_string drawAtPoint:NSMakePoint(0, 0) withAttributes:attributes];

        [NSGraphicsContext restoreGraphicsState];

        unsigned char* src_pixels = [rep bitmapData];
        if (!src_pixels) return NULL;

        // Buat SDL_Surface dari pixel buffer (referensi memori sementara)
        SDL_Surface* surface = SDL_CreateSurfaceFrom(
            width, height,
            SDL_PIXELFORMAT_RGBA32,
            src_pixels,
            width * 4
        );

        if (!surface) {
            return NULL;
        }

        // Buat tekstur GPU dari surface (piksel disalin ke memori GPU)
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);

        if (texture) {
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        }

        if (out_w) *out_w = width;
        if (out_h) *out_h = height;

        return texture;
    }
}
