#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>

SDL_Texture* platform_create_text_texture_windows(SDL_Renderer* renderer, const char* text, SDL_Color color, int* out_w, int* out_h) {
    if (!text || strlen(text) == 0) return NULL;

    // Convert UTF-8 to UTF-16
    int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    if (len <= 0) return NULL;
    WCHAR* wtext = (WCHAR*)malloc(len * sizeof(WCHAR));
    if (!wtext) return NULL;
    MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext, len);

    // Create memory DC
    HDC hdc = CreateCompatibleDC(NULL);
    if (!hdc) {
        free(wtext);
        return NULL;
    }

    // Create high-quality Segoe UI font (20pt size, which is approximately 27 pixels height)
    HFONT hFont = CreateFontW(
        -24,                   // Height
        0,                     // Width
        0,                     // Escapement
        0,                     // Orientation
        FW_NORMAL,             // Weight
        FALSE,                 // Italic
        FALSE,                 // Underline
        FALSE,                 // StrikeOut
        DEFAULT_CHARSET,       // CharSet
        OUT_DEFAULT_PRECIS,    // OutPrecision
        CLIP_DEFAULT_PRECIS,   // ClipPrecision
        CLEARTYPE_QUALITY,     // Quality (ClearType)
        DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"            // FaceName
    );

    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    // Measure text size
    SIZE size;
    if (!GetTextExtentPoint32W(hdc, wtext, len - 1, &size)) {
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
        DeleteDC(hdc);
        free(wtext);
        return NULL;
    }

    int w = size.cx;
    int h = size.cy;
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    if (out_w) *out_w = w;
    if (out_h) *out_h = h;

    // Create DIB section
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pvBits = NULL;
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
    if (!hBitmap) {
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
        DeleteDC(hdc);
        free(wtext);
        return NULL;
    }

    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdc, hBitmap);

    // Set text and background properties
    // Render text in white on a black background to extract the anti-aliased font mask
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, OPAQUE);

    // Clear bitmap bits to black
    memset(pvBits, 0, w * h * 4);

    // Draw text
    RECT rect = { 0, 0, w, h };
    DrawTextW(hdc, wtext, len - 1, &rect, DT_LEFT | DT_TOP | DT_NOCLIP);

    // Extract alpha from pixel intensity and apply color in RGBA layout
    unsigned char* dest = (unsigned char*)pvBits;
    for (int i = 0; i < w * h * 4; i += 4) {
        // In DIB, the drawn white pixels will have Blue, Green, Red all equal to the antialiasing coverage (0-255).
        unsigned char alpha = dest[i]; // Get blue channel as alpha mask
        dest[i]     = color.r; // R
        dest[i + 1] = color.g; // G
        dest[i + 2] = color.b; // B
        dest[i + 3] = alpha;   // A
    }

    // Create SDL_Surface from the RGBA DIB pixels
    SDL_Surface* surface = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pvBits, w * 4);
    SDL_Texture* texture = NULL;
    if (surface) {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
    }

    // Cleanup Win32 GDI objects
    SelectObject(hdc, hOldBitmap);
    DeleteObject(hBitmap);
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    DeleteDC(hdc);
    free(wtext);

    return texture;
}
#endif
