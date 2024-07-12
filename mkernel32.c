#include <unistd.h>
#include <dlfcn.h>

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <wchar.h>

#include "common.h"
#include "ntconv.h"
#include "ntrsrc.h"

struct startup_info {
    int size;
    char *_reserved;
    char *desktop;
    char *title;
    int x, y;
    int x_size, y_size;
    int x_chars, y_chars;
    int fill_attrib;
    int flags;
    short show_window;
    short _reserved2;
    void *_reserved3;
    void *stdin, *stdout, *stderr;
};

void __stdcall *GetModuleHandleA(const char *name) {
    /* GetModuleHandleA gets called only two times during runtime librray 
       initialization:
        1. To get module base and analyze headers.
        2. To get hInstance parameter of WinMain

        Because it is the first function called, it needs to
        restore SEH setup.

        Header checking through runtime library initialization
        is not necessary. When .exe doesn't see expected magic
        value, it just skips that step.
    */
    SEH_restore();

    TRACE("\"%s\"", name);
    return dlopen(NULL, RTLD_LAZY);
}

void __stdcall *FindResourceW(void *hModule, const nt_wchar_t *lpName, const nt_wchar_t *lpType) {
    wchar_t *name_w = NULL, *type_w = NULL;

    TRACE("%p \"%ls\" \"%ls\"",
        hModule,
        ((uintptr_t)lpName & 0xFFFF0000) > 0 
            ? (name_w = nt_wchar_to_wchar(lpName))
            : L"",
        ((uintptr_t)lpType & 0xFFFF0000) > 0 
            ? (type_w = nt_wchar_to_wchar(lpType))
            : L""
        );

    SAFE_FREE(name_w);
    SAFE_FREE(type_w);

    const struct _IMAGE_RESOURCE_DIRECTORY_ENTRY *type_entry = search_for_resource_entry(__rsrc_base, lpType);
    if (!type_entry) {
        TRACE_RET("[Type entry was not found]");
        return NULL;
    }
    if (!MSB_ISSET(type_entry->OffsetToData))
        PANIC("Type directory does not contain address to name directory");

    const struct _IMAGE_RESOURCE_DIRECTORY_ENTRY *name_entry = search_for_resource_entry(__rsrc_base + MSB_CLR(type_entry->OffsetToData), lpName);
    if (!name_entry) {
        TRACE_RET("[Name entry was not found]");
        return NULL;
    }

    /*
     * If the function succeeds, the return value is a handle to the specified resource's information block.
     */
    const struct _IMAGE_RESOURCE_DIRECTORY_ENTRY *language_entry = __rsrc_base + MSB_CLR(name_entry->OffsetToData) + sizeof (struct _IMAGE_RESOURCE_DIRECTORY);
    const struct _IMAGE_RESOURCE_DATA_ENTRY *information_block = __rsrc_base + language_entry->OffsetToData;

    TRACE_RET("%p", information_block);
    return (void *)information_block;
}

void __stdcall OutputDebugStringA(const char *string) {
    TRACE("\"%s\"", string);
    fputs(string, stderr);
}

void __stdcall *LockResource(void *hResData) {
    TRACE("%p", hResData);
    
    if (!hResData)
        PANIC("Invalid parameter");
    
    const struct _IMAGE_RESOURCE_DATA_ENTRY *data_entry = hResData;
    const void *raw = __nt_base + data_entry->OffsetToData;
    
    TRACE_RET("%p", raw);
    return (void *)raw;
}

void __stdcall *LoadResource(void *hModule, void *hResInfo) {
    TRACE("%p %p", hModule, hResInfo);

    IGNORED_PARAMETER(hModule, hResInfo);

    /*
     * This function does the translation between HRSRC and HGLOBAL, which are the same in our case
     */
    TRACE_RET("%p", hResInfo);
    return hResInfo;
}

int __stdcall lstrlenW(const nt_wchar_t *lpString) {
    wchar_t *string_w;
    TRACE("\"%ls\"", (string_w = nt_wchar_to_wchar(lpString)) ? string_w : L"");

    if (string_w) free(string_w);
    int length = nt_wchar_strlen(lpString);

    TRACE_RET("%i", length);
    return length;
}

unsigned __stdcall GetPrivateProfileIntW(const nt_wchar_t *lpAppName, const nt_wchar_t *lpKeyName, int nDefault, const nt_wchar_t *lpFileName) {
    wchar_t *app_name_w, *key_name_w, *file_name_w;
    TRACE("\"%ls\" \"%ls\" %i \"%ls\"",
        (app_name_w = nt_wchar_to_wchar(lpAppName)),
        (key_name_w = nt_wchar_to_wchar(lpKeyName)),
        nDefault,
        (file_name_w = nt_wchar_to_wchar(lpFileName))
    );

    if (app_name_w) free(app_name_w);
    if (key_name_w) free(key_name_w);
    if (file_name_w) free(file_name_w);

    IGNORED_PARAMETER(lpAppName, lpKeyName, lpFileName);

    if (!lpFileName)
        PANIC("Invalid parameter");

    /* No .ini files in this emulation, so return default */
    TRACE_RET("%u", (unsigned)nDefault);
    return nDefault;
}

int __stdcall GetPrivateProfileStringW(const nt_wchar_t *lpAppName, const nt_wchar_t *lpKeyName, const nt_wchar_t *lpDefault, nt_wchar_t *lpReturnedString, int nSize, const nt_wchar_t *lpFileName) {
    wchar_t *app_name_w, *key_name_w, *default_w, *file_name_w;
    TRACE("\"%ls\" \"%ls\" \"%ls\" %p %i \"%ls\"",
        (app_name_w = nt_wchar_to_wchar(lpAppName)),
        (key_name_w = nt_wchar_to_wchar(lpKeyName)),
        (default_w  = nt_wchar_to_wchar(lpDefault)),
        lpReturnedString,
        nSize,
        (file_name_w = nt_wchar_to_wchar(lpFileName))
    );
    
    if (app_name_w) free(app_name_w);
    if (key_name_w) free(key_name_w);
    if (default_w) free(default_w);
    if (file_name_w) free(file_name_w);

    if (!lpReturnedString || !lpFileName)
        PANIC("Invalid parameter");

    IGNORED_PARAMETER(lpAppName, lpKeyName, lpFileName);

    size_t length = nt_wchar_strlen(lpDefault) + 1;
    if (length > nSize) {
        /* 
         * If neither lpAppName nor lpKeyName is NULL and the supplied destination buffer is too small to hold the requested string, the string is truncated and followed by a null character, and the return value is equal to nSize minus one.
         */
        length = nSize;
    }

    /*
     * If this parameter is NULL, the default is an empty string, "".
     */
    if (!lpDefault)
        lpDefault = &(nt_wchar_t){ 0 };

    memcpy(lpReturnedString, lpDefault, length * sizeof(nt_wchar_t));
    lpReturnedString[length - 1] = NT_NUL;

    /*
     * The return value is the number of characters copied to the buffer, not including the terminating null character.
     */
    wchar_t *returned_string_w;
    TRACE_RET("\"%ls\" (%zu)",
        (returned_string_w = nt_wchar_to_wchar(lpReturnedString))
            ? returned_string_w
            : L"",
        length - 1
    );
    SAFE_FREE(returned_string_w);
    return length - 1;
}

int __stdcall GetTickCount(void) {
    TRACE_NOARGS();
    
    FILE *uptime_fd = fopen("/proc/uptime", "r");
    NT_ASSERT(uptime_fd, "cannot open /proc/uptime: %s", strerror(errno));

    float seconds;
    NT_ASSERT(fscanf(uptime_fd, "%g", &seconds) == 1, "failed to get uptime seconds: %s", strerror(errno));

    int milliseconds = seconds * 1000.0f;
    fclose(uptime_fd);

    TRACE_RET("%i", milliseconds);
    return milliseconds;
}

/* TODO: Find out usage */
int __stdcall GetModuleFileNameA(void *module, char *file_name, int size) {
    TRACE("%p \"%s\" %i", module, file_name, size);
    ;
}

/* TODO: Find out usage */
void __stdcall GetStartupInfoA(struct startup_info *startup_info) {
    TRACE("%p", startup_info);
    ;
}

/* TODO: Find out usage */
void __stdcall * (*GetProcAddress(void *module, const char *name))() {
    TRACE("%p \"%s\"", module, name);
    ;
}

/* TODO: Unimplemented */
nt_wchar_t __stdcall *lstrcpyW(nt_wchar_t *dest, const nt_wchar_t *source) {
    TRACE("%p \"%ls\"", dest, source);
    ;
}

/* TODO: Find out usage */
void __stdcall *LoadLibraryA(const char *name) {
    TRACE("\"%s\"", name);
    ;
}