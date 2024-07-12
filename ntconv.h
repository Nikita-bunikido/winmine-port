#ifndef __NT_CONV_H__
#define __NT_CONV_H__

#include <stddef.h>
#include <inttypes.h>
#include <wchar.h>

#define NT_NUL      (nt_wchar_t)'\0'

typedef uint16_t nt_wchar_t;

wchar_t *nt_wchar_to_wchar(const nt_wchar_t *);
wchar_t *nt_wchar_to_wchar_n(const nt_wchar_t *, size_t);

nt_wchar_t *char_to_nt_wchar(const char *);
nt_wchar_t *char_to_nt_wchar_n(const char *, size_t );

nt_wchar_t *wchar_to_nt_wchar(const wchar_t *);
nt_wchar_t *wchar_to_nt_wchar_n(const wchar_t *, size_t);

size_t nt_wchar_strlen(const nt_wchar_t *);
int nt_wchar_strcmp(const nt_wchar_t *, const nt_wchar_t *);

#endif /* __NT_CONV_H__ */