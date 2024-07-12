#include <SDL.h>
#include <inttypes.h>

#include <sys/mman.h>

#include "common.h"

STATIC_ASSERT(sizeof(uintptr_t) == 4, 
    "Not x86");

#define INITIALIZE_IMPORT_TABLE(_it)                                      __extension__({       \
    mprotect(__nt_base + (_it)->rva, (_it)->number * 4, PROT_READ | PROT_WRITE | PROT_EXEC);    \
    memcpy32(__nt_base + (_it)->rva, (_it)->procedures, (_it)->number);                         \
    mprotect(__nt_base + (_it)->rva, (_it)->number * 4, PROT_READ | PROT_EXEC);                 \
})

extern char _rsrc_base, _nt_base;
void *__rsrc_base = &_rsrc_base,
     *__nt_base   = &_nt_base;

struct __packed Import_Table {
    uintptr_t rva;
    uint32_t number;
    void * (*procedures[])();
};

__startup
__always_inline
static inline void memcpy32(void *dest, const void *src, long sz) {
    asm volatile ("rep movsl \n"
         : "+D"(dest), "+S"(src)
         : "c"(sz)
         : "memory");
}

__startup
__noreturn
void stage2(int argc, char **argv, char **envp) {
    extern struct Import_Table
        kernel32_iat, msvcrt_iat, user32_iat,
        advapi32_iat, gdi32_iat, comctl32_iat,
        shell32_iat, winmm_iat;

    INITIALIZE_IMPORT_TABLE(&kernel32_iat);
    INITIALIZE_IMPORT_TABLE(&msvcrt_iat);
    INITIALIZE_IMPORT_TABLE(&user32_iat);
    INITIALIZE_IMPORT_TABLE(&advapi32_iat);
    INITIALIZE_IMPORT_TABLE(&gdi32_iat);
    INITIALIZE_IMPORT_TABLE(&comctl32_iat);
    INITIALIZE_IMPORT_TABLE(&shell32_iat);
    INITIALIZE_IMPORT_TABLE(&winmm_iat);

    if (SDL_Init(SDL_REQUIRED_SUBSYSTEMS) < 0)
        PANIC("Failed to initialize SDL: %s", SDL_GetError());

    SEH_prepare();

    extern char _entry_point;
    asm volatile ("jmp *%0" :: "r"(&_entry_point));
    __builtin_unreachable();
}