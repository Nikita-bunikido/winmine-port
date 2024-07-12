#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <wchar.h>

#include "common.h"
#include "ntconv.h"

__alt_text
size_t nt_wchar_strlen(const nt_wchar_t *str) {
    if (!str) return 0ull;
    const nt_wchar_t *base = str;
    while (*str) str++;
    return str - base;
}

__alt_text
wchar_t *nt_wchar_to_wchar_n(const nt_wchar_t *source, size_t n) {
    if (!source || !n) return NULL;
    wchar_t *converted = malloc(sizeof(wchar_t) + sizeof(wchar_t) * n);
    NT_ASSERT(converted, "malloc failed: %s", strerror(errno));

    converted[n] = L'\0';
    for (wchar_t *p = converted; n > 0; *p++ = *source++, n --);
    return converted;
}

__alt_text
nt_wchar_t *char_to_nt_wchar_n(const char *source, size_t n) {
    if (!source || !n) return NULL;
    nt_wchar_t *converted = malloc(sizeof(nt_wchar_t) + sizeof(nt_wchar_t) * n);
    NT_ASSERT(converted, "malloc failed: %s", strerror(errno));

    converted[n] = NT_NUL;
    for (nt_wchar_t *p = converted; n > 0; *p++ = *source++, n --);
    return converted;
}

__alt_text
nt_wchar_t *wchar_to_nt_wchar_n(const wchar_t *source, size_t n) {
    if (!source || !n) return NULL;
    nt_wchar_t *converted = malloc(sizeof(nt_wchar_t) + sizeof(nt_wchar_t) * n);
    NT_ASSERT(converted, "malloc failed: %s", strerror(errno));

    converted[n] = NT_NUL;
    for (nt_wchar_t *p = converted; n > 0; *p++ = *source++, n --);
    return converted;
}

__alt_text
wchar_t *nt_wchar_to_wchar(const nt_wchar_t *source) {
    return nt_wchar_to_wchar_n(source, nt_wchar_strlen(source));
}

__alt_text
nt_wchar_t *char_to_nt_wchar(const char *source) {
    return char_to_nt_wchar_n(source, strlen(source));
}

__alt_text
nt_wchar_t *wchar_to_nt_wchar(const wchar_t *source) {
    return wchar_to_nt_wchar_n(source, wcslen(source));
}

__alt_text
int nt_wchar_strcmp(const nt_wchar_t *str1, const nt_wchar_t *str2) {
    while (*str1 && *str2 && *str1++ == *str2++);
    return *--str1 - *--str2;
}