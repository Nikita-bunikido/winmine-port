#include <stdlib.h>

#include "common.h"
#include "ntgdi.h"

__alt_text
static void *memappend(void **dest, const void *source, size_t size) {
    memcpy(*dest, source, size);
    *dest += size;
    return *dest;
}

__alt_text
__always_inline
static inline long align4(long x) {
    while (x % 4) x ++;
    return x;
}

__alt_text
void *header_and_data_to_bmp(const struct _BITMAPINFOHEADER *header, const void *data, long width, long height, size_t *size) {
    struct _BITMAPINFOHEADER info_header = *header;
    info_header.biWidth = width;
    info_header.biHeight = height;

    /*
     * The rest of BITMAPINFOHEADER is taken from MS-Paint
     * generated 4-bit bitmap. Tbh, I don't even know
     * what the hell is this. . .
     */
    const unsigned char header_rest[] = 
        "\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x00\x00"
        "\x00\x80\x80\x00\x80\x00\x00\x00\x80\x00\x80\x00"
        "\x80\x80\x00\x00\x80\x80\x80\x00\xC0\xC0\xC0\x00"
        "\x00\x00\xFF\x00\x00\xFF\x00\x00\x00\xFF\xFF\x00"
        "\xFF\x00\x00\x00\xFF\x00\xFF\x00\xFF\xFF\x00\x00"
        "\xFF\xFF\xFF";

    const size_t raw_size = align4(info_header.biWidth) * align4(info_header.biHeight) * info_header.biBitCount / 8;
    const struct _BITMAPFILEHEADER file_header = {
        .bfType = 0x4d42u,
        .bfSize = sizeof(struct _BITMAPFILEHEADER) 
                + sizeof(struct _BITMAPINFOHEADER) 
                + sizeof header_rest
                + raw_size,
        .bfReserved1 = 0u,
        .bfReserved1 = 0u,
        .bfOffBits = 0x76u
    };

    void *bmp_raw = malloc(file_header.bfSize), *bmp_ptr;
    NT_ASSERT(bmp_raw, "malloc failed: %s", strerror(errno));
    bmp_ptr = bmp_raw;

    memappend(&bmp_ptr, &file_header, sizeof file_header);
    memappend(&bmp_ptr, &info_header, sizeof info_header);
    memappend(&bmp_ptr, header_rest, sizeof header_rest);
    memappend(&bmp_ptr, data, raw_size);

    if (size)
        *size = file_header.bfSize;

    return bmp_raw;
}