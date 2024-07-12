#ifndef _COMMON_H
#define _COMMON_H

#include <stdlib.h>
#include <errno.h>
#include <locale.h>
#include <inttypes.h>

#include <wchar.h>

#include <SDL.h>

#define TYPEOF(_X)                      __typeof__(_X)
#define STATIC_ASSERT(_expr, _str)      _Static_assert(_expr, _str)

#define MAX(_X, _Y)                     __extension__({                             \
    TYPEOF(_X) __X = (_X);                                                          \
    TYPEOF(_Y) __Y = (_Y);                                                          \
    __X > __Y ? __X : __Y;                                                          \
})

#define SDL_REQUIRED_SUBSYSTEMS          SDL_INIT_VIDEO

#define TRUE                             1
#define FALSE                            0

#define ERROR_SUCCESS                    0L
#define ERROR_FILE_NOT_FOUND             2L
#define ERROR_MORE_DATA                  234L
#define ERROR_INVALID_PARAMETER          87L

#define SW_SHOWNORMAL                    1

#define WM_LBUTTONDOWN                   0x0201
#define WM_RBUTTONDOWN                   0x0204
#define WM_MBUTTONDOWN                   0x0207
#define WM_LBUTTONUP                     0x0202
#define WM_RBUTTONUP                     0x0205
#define WM_MBUTTONUP                     0x0208
#define WM_MOUSEMOVE                     0x0200
#define WM_DESTROY                       0x0002
#define WM_PAINT                         0x000f
#define WM_QUIT                          0x0012
#define WM_NULL                          0x0000
#define WM_TIMER                         0x0113

#define MK_LBUTTON                       0x0001
#define MK_RBUTTON                       0x0002
#define MK_MBUTTON                       0x0010

#define SAFE_FREE(_ptr)                  if ((_ptr)){ free((_ptr)); (_ptr) = NULL; }

#define MSB_ISSET(_dword)                (!!((uintptr_t)(_dword) & 0x80000000))
#define MSB_CLR(_dword)                  ((uintptr_t)(_dword) & 0x7FFFFFFF)

#define IS_ID(_dword)                    !((uintptr_t)(_dword) & 0xFFFF0000)
#define IS_ATOM                          IS_ID

#define IGNORED_PARAMETER(...)           (void)(__VA_ARGS__)

#define RT_STRING                        6
#define RT_ICON                          3
#define RT_GROUP_ICON                    14

#define TRACE(_fmt, ...)                                                            \
do {                                                                                \
    errno = 0;                                                                      \
    printf("[^.^] %s( " _fmt " )\n", __FUNCTION__ __VA_OPT__(,) __VA_ARGS__);       \
    if (errno) perror("printf");                                                    \
} while ( 0 )

#define TRACE_RET(_fmt, ...)                                                        \
do {                                                                                \
    errno = 0;                                                                      \
    printf("\t==> " _fmt "\n" __VA_OPT__(,) __VA_ARGS__);                           \
    if (errno) perror("printf");                                                    \
} while ( 0 )

#define PANIC(_fmt, ...)                                                            \
do {                                                                                \
    errno = 0;                                                                      \
    printf("[X.X] %s: " _fmt "\n", __FUNCTION__ __VA_OPT__(,) __VA_ARGS__);         \
    if (errno) perror("printf");                                                    \
    abort();                                                                        \
} while( 0 )

#define NT_ASSERT(_expr, ...)                                                       \
    do { if ((_expr)) break; PANIC("assertion failed: " __VA_ARGS__); } while( 0 )

#define TRACE_NOARGS()                   TRACE("")

#if defined(__stdcall)
#   undef __stdcall
#endif

#define __stdcall                        __attribute__((__stdcall__))
#define __alt_text                       __attribute__((__section__(".alttext")))
#define __always_inline                  __attribute__((__always_inline__))
#define __noreturn                       __attribute__((__noreturn__))
#define __startup                        __attribute__((__section__(".startup")))
#define __packed                         __attribute__((__packed__))

/* from startup.c */
extern void *__rsrc_base,
            *__nt_base;

void __startup SEH_prepare(void);
void __startup SEH_restore(void);

#endif /* _COMMON_H */