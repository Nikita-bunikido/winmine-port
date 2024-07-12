#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <SDL.h>

#include "common.h"
#include "ntgdi.h"

static struct GDI_Object light_gray_brush = {
    GDI_BRUSH,
    (void *)(uintptr_t)0x00c5c5c5
};

int __stdcall SetROP2(void *hdc, unsigned rop2) {
    TRACE("%p %x", hdc, rop2);
    
    NT_ASSERT(hdc, "invalid parameter");

    struct GDI_DC *dc = hdc;

    unsigned previous_rop2 = dc->rop2;
    dc->rop2 = rop2;

    TRACE_RET("%x", previous_rop2);
    return previous_rop2;
}

/* TODO: Unimplemented */
int __stdcall GetLayout(void *dc) {
    TRACE("%p", dc);
    ;
}

/* TODO: Unimplemented */
int __stdcall SetLayout(void *dc, int layout) {
    TRACE("%p %i", dc, layout);
    ;
}

/* TODO: Unimplemented */
int __stdcall GetDeviceCaps(void *dc, int index) {
    TRACE("%p %i", dc, index);
    ;
}

int __stdcall DeleteObject(void *object) {
    TRACE("%p", object);
    ;
}

int __stdcall LineTo(void *hdc, int x, int y) {
    TRACE("%p %i %i", hdc, x, y);
    
    NT_ASSERT(hdc, "invalid parameter");

    struct GDI_DC *dc = hdc;
    struct GDI_Pen *pen = dc->pen->data;

    chk(SDL_SetRenderTarget(dc->renderer, dc->bitmap->data));

    switch (dc->rop2) {
    case R2_WHITE:
        chk(SDL_SetRenderDrawColor(dc->renderer, 255, 255, 255, 255));
        break;
    
    case R2_COPYPEN:
        chk(SDL_SetRenderDrawColor(dc->renderer, COLOR_GET_R(pen->color), COLOR_GET_G(pen->color), COLOR_GET_B(pen->color), 255));
        break;

    default:
        NT_ASSERT(0, "invalid rop2");
    }
    
    chk(SDL_RenderDrawLine(dc->renderer, dc->pos.x, dc->pos.y, x, y));
    chk(SDL_SetRenderTarget(dc->renderer, NULL));

    dc->pos.x = x;
    dc->pos.y = y;
    return 1;
}

void __stdcall *CreatePen(int iStyle, int cWidth, unsigned color) {
    TRACE("%x %i %x", iStyle, cWidth, color);
    
    struct GDI_Object *pen = malloc(sizeof *pen);
    NT_ASSERT(pen, "malloc failed: %s", strerror(errno));

    struct GDI_Pen *pen_data = malloc(sizeof *pen_data);
    NT_ASSERT(pen, "malloc failed: %s", strerror(errno));
    pen_data->width = cWidth;
    pen_data->color = color;

    *pen = (struct GDI_Object) {
        .type = GDI_PEN,
        .data = pen_data
    };

    TRACE_RET("%p", pen);
    return pen;
}

void __stdcall *CreateCompatibleDC(void *hdc) {
    TRACE("%p", hdc);

    struct GDI_DC *dc = hdc, *new_dc;
    new_dc = malloc(sizeof *new_dc);
    NT_ASSERT(new_dc, "malloc failed: %s", strerror(errno));

    /* Even though, MSDN says it must create 1x1 monochrome bitmap, 
     * Minesweeper immidiately replaces it with compatible bitmap,
     * so we omit this. */

    TRACE_RET("%p", new_dc);
    return memcpy(new_dc, dc, sizeof *new_dc);
}

void __stdcall *CreateCompatibleBitmap(void *hdc, int width, int height) {
    TRACE("%p %i %i", hdc, width, height);

    NT_ASSERT(hdc, "invalid parameter");

    struct GDI_DC *dc = hdc;

    struct GDI_Object *bitmap_object = malloc(sizeof *bitmap_object);
    NT_ASSERT(bitmap_object, "malloc failed: %s", strerror(errno));

    *bitmap_object = (struct GDI_Object) {
        .type = GDI_BITMAP,
        .data = SDL_CreateTexture(dc->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height)
    };

    NT_ASSERT(bitmap_object->data, "SDL_CreateTexture failed: %s", SDL_GetError());
    TRACE_RET("%p", bitmap_object);
    return bitmap_object;
}

void __stdcall *SelectObject(void *hdc, void *h) {
    TRACE("%p %p", hdc, h);

    NT_ASSERT(hdc, "invalid parameter");

    struct GDI_DC *dc = hdc;
    struct GDI_Object *object = h;

    void **dc_object = &dc->raw[object->type];
    void *old_object = *dc_object;

    *dc_object = h;
    TRACE_RET("%p", old_object);
    return old_object;
}

int __stdcall SetDIBitsToDevice(void *hdc, int xDest, int yDest, unsigned w, unsigned h, int xSrc, int ySrc, unsigned StartScan, unsigned cLines, void *lpvBits, /* BITMAPINFO * */ void *lpbmi, unsigned ColorUse) {
    TRACE("%p %i %i %u %u %i %i %u %u %p %p %x", hdc, xDest, yDest, w, h, xSrc, ySrc, StartScan, cLines, lpvBits, lpbmi, ColorUse);
    
    NT_ASSERT(hdc && lpvBits && lpbmi, "invalid parameter");

    struct GDI_DC *dc = hdc;
    struct _BITMAPINFOHEADER info_header = *(struct _BITMAPINFOHEADER *)lpbmi;
    info_header.biWidth = w;
    info_header.biHeight = h;

    /* Now rendering goes not on window, but rather on bitmap in hdc */
    chk(SDL_SetRenderTarget(dc->renderer, dc->bitmap->data));

    /* STEP1: Prepare BMP file */
    size_t bmp_size;
    void *bmp_raw = header_and_data_to_bmp(lpbmi, lpvBits, w, h, &bmp_size);

    /* STEP2: Create SDL_Texture from BMP */
    SDL_RWops *bmp_stream = SDL_RWFromConstMem(bmp_raw, bmp_size);
    NT_ASSERT(bmp_stream, "SDL_RWFromConstMem failed: %s", SDL_GetError());

    SDL_Surface *bmp_surface = SDL_LoadBMP_RW(bmp_stream, 1);
    NT_ASSERT(bmp_surface, "SDL_LoadBMP_RW failed: %s", SDL_GetError());

    SDL_Texture *bmp_texture = SDL_CreateTextureFromSurface(dc->renderer, bmp_surface);
    NT_ASSERT(bmp_texture, "SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
    SDL_FreeSurface(bmp_surface);

    /* STEP3: Draw BMP on hdc */
    const SDL_Rect source_rect = { xSrc, ySrc, w, h },
                   dest_rect   = { xDest, yDest, w, h };
    chk(SDL_RenderCopy(dc->renderer, bmp_texture, &source_rect, &dest_rect));

    /* Restore render target */
    chk(SDL_SetRenderTarget(dc->renderer, NULL));

    SDL_DestroyTexture(bmp_texture);
    free(bmp_raw);

    /*
     * If the function succeeds, the return value is the number of scan lines set.
     */
    TRACE_RET("%u", cLines);
    return cLines;
}

int __stdcall DeleteDC(void *dc) {
    TRACE("%p", dc);
    ;
}

int __stdcall MoveToEx(void *hDc, int x, int y, struct _POINT *lppt) {
    TRACE("%p %i %i %p", hDc, x, y, lppt);
    
    NT_ASSERT(hDc, "invalid parameter");

    struct GDI_DC *dc = hDc;
    if (lppt)
        *lppt = dc->pos;
    
    dc->pos.x = x;
    dc->pos.y = y;
    return 1;
}

/* TODO: Unimplemented */
int __stdcall SetPixel(void *dc, int x, int y, unsigned color) {
    TRACE("%p %i %i %x", dc, x, y, color);
    ;
}

int __stdcall BitBlt(void *hdc, int x, int y, int cx, int cy, void *hdcSrc, int x1, int y1, unsigned rop) {
    TRACE("%p %i %i %i %i %p %i %i %x", hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
    
    NT_ASSERT(hdc && hdcSrc, "invalid parameter");

    struct GDI_DC *source = hdcSrc, *dest = hdc;
    
    chk(SDL_SetRenderTarget(dest->renderer, dest->bitmap->data));

    const SDL_Rect source_rect = { x1, y1, cx, cy },
                   dest_rect   = { x, y, cx, cy };
    chk(SDL_RenderCopy(dest->renderer, source->bitmap->data, &source_rect, &dest_rect));

    chk(SDL_SetRenderTarget(dest->renderer, NULL));
    return 1;
}

void __stdcall *GetStockObject(int type) {
    TRACE("%i", type);
    
    switch (type) {
    case LTGRAY_BRUSH:
        TRACE_RET("%p", &light_gray_brush);    
        return &light_gray_brush;

    default:
        NT_ASSERT(0, "invalid parameter");
    }

    return NULL;
}