#ifndef __NT_GDI_H__
#define __NT_GDI_H__

#include <inttypes.h>
#include <stdbool.h>

#include <SDL.h>

#include "common.h"
#include "ntconv.h"

#define chk(_x)                     __extension__({                                   \
    if ((_x) < 0)                                                                     \
        PANIC("%s:%u:SDL function failed: %s", __FILE__, __LINE__, SDL_GetError());   \
})

#define COLOR_GET_R(_col)           (Uint8)((_col) & 0xff)
#define COLOR_GET_G(_col)           (Uint8)(((_col) >> 8) & 0xff)
#define COLOR_GET_B(_col)           (Uint8)(((_col) >> 16) & 0xff)
#define COLOR_GET_A(_col)           (Uint8)(((_col) >> 24) & 0xff)

#define DESKTOP_WINDOW_HWND         (void *)(uintptr_t)0xBAADBEEF

#define R2_COPYPEN	                13
#define R2_WHITE	                16

#define LTGRAY_BRUSH                1

struct _WNDCLASSW {
    unsigned style;
    uintptr_t (* __stdcall lpfnWndProc)(void *, unsigned, uintptr_t, uintptr_t);
    int cbClsExtra;
    int cbWndExtra;
    void *hInstance;
    void *hIcon;
    void *hCursor;
    void *hbrBackground;
    const nt_wchar_t *lpszMenuName;
    const nt_wchar_t *lpszClassName;
};

struct _POINT {
    long x;
    long y;
};

struct _RECT {
    long left;
    long top;
    long right;
    long bottom;
};

struct _MSG {
    void *hwnd;
    unsigned message;
    uintptr_t wParam;
    uintptr_t lParam;
    unsigned time;
    struct _POINT pt;
};

struct _PAINTSTRUCT {
    void *hdc;
    int fErase;
    struct _RECT rcPaint;
    int fRestore;
    int fIncUpdate;
    char rgbReserved[32];
};

struct __attribute__((packed)) _BITMAPFILEHEADER {
    unsigned short bfType;
    unsigned bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned bfOffBits;
};

struct __attribute__((packed)) _BITMAPINFOHEADER {
    unsigned biSize;
    long biWidth;
    long biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned biCompression;
    unsigned biSizeImage;
    long biXPelsPerMeter;
    long biYPelsPerMeter;
    unsigned biClrUsed;
    unsigned biClrImportant;
};

struct _RGBQUAD {
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
};

struct _BITMAPINFO {
    struct _BITMAPINFOHEADER bmiHeader;
};

enum GDI_Object_Type {
    GDI_BITMAP = 0,
    GDI_BRUSH,
    GDI_FONT,
    GDI_PEN,
    GDI_REGION,
};

struct GDI_Object {
    enum GDI_Object_Type type;
    void *data;
};

struct GDI_Pen {
    int width;
    unsigned color;
};

struct GDI_DC {
    SDL_Renderer *renderer;
    struct _POINT pos;
    unsigned rop2;

    union {
        struct GDI_Object *raw[5];
        struct {
            struct GDI_Object *bitmap, *brush, *font, *pen, *region;
        };
    };
};

struct Message {
    bool prepared;

    union {
        SDL_Event event;
        struct _MSG message;
    };
};

struct Window {
    struct GDI_DC dc;
    pthread_t renderer_thread;
    SDL_Window *window;
    SDL_Renderer *renderer;
};

void *header_and_data_to_bmp(const struct _BITMAPINFOHEADER *header, const void *data, long width, long height, size_t *size);

#endif /* __NT_GDI_H__ */