#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "common.h"
#include "ntconv.h"

#define REGISTRY_CAP                256

#define REG_CREATED_NEW_KEY         1L
#define REG_OPENED_EXISTING_KEY     2L

enum Predefined_Hkey {
    HKEY_CLASSES_ROOT   = 0x80000000,
    HKEY_CURRENT_USER   = 0x80000001,
    HKEY_LOCAL_MACHINE  = 0x80000002,
    HKEY_USERS          = 0x80000003,
    HKEY_CURRENT_CONFIG = 0x80000005,
};

enum Value_Type {
    REG_NONE = 0,
    REG_SZ,
    REG_EXPAND_SZ,
    REG_BINARY,
    REG_DWORD,
    REG_DWORD_BIG_ENDIAN,
    REG_LINK,
    REG_MULTI_SZ,
    REG_RESOURCE_LIST,
    REG_FULL_RESOURCE_DESCRIPTOR,
    REG_RESOURCE_REQUIREMENTS_LIST,
    REG_QWORD,
};

struct Registry_Value {
    nt_wchar_t name[256];
    enum Value_Type type;
    size_t size;
    void *value;
};

struct Registry_Entry {
    void *key;
    nt_wchar_t subkey[256];

    size_t values_number;
    struct Registry_Value *values;
};

static struct Registry_Entry REGISTRY[REGISTRY_CAP] = { 0 };
static size_t registry_size = 0;

static inline __attribute__((always_inline)) char *hkey_as_cstr(const void *key) {
    static char string_buffer[32];

    switch ((uintptr_t)key) {
        case HKEY_CLASSES_ROOT: return "HKEY_CLASSES_ROOT";
        case HKEY_CURRENT_USER: return "HKEY_CURRENT_USER";
        case HKEY_LOCAL_MACHINE: return "HKEY_LOCAL_MACHINE";
        case HKEY_USERS: return "HKEY_USERS";
        case HKEY_CURRENT_CONFIG: return "HKEY_CURRENT_CONFIG";
    }

    snprintf(string_buffer, sizeof string_buffer, "%x", (uintptr_t)key);
    return string_buffer;
}

static struct Registry_Value *search_for_value(const struct Registry_Entry *entry, const nt_wchar_t *name) {
    struct Registry_Value *c_value = entry->values;
    for (size_t i = entry->values_number; i > 0; i --, c_value ++)
        if (!nt_wchar_strcmp(c_value->name, name)) return c_value;
    
    return NULL;
}

static struct Registry_Entry *search_for_entry(const void *key, const nt_wchar_t *subkey) {
    struct Registry_Entry *c_entry = REGISTRY;
    for(; c_entry < REGISTRY + REGISTRY_CAP; c_entry ++)
        if ((c_entry->key == key) && !nt_wchar_strcmp(c_entry->subkey, subkey)) return c_entry;

    return NULL;
}

int __stdcall RegQueryValueExW(void *hKey, const nt_wchar_t *lpValueName, void *lpReserved, unsigned *lpType, void *lpData, unsigned *lpcbData) {
    wchar_t *value_name_w = NULL;    
    TRACE("\"%s\" \"%ls\" %p %p %p (%u)",
        hkey_as_cstr(hKey),
        (value_name_w = nt_wchar_to_wchar(lpValueName)),
        lpType,
        lpData,
        lpcbData,
        lpcbData
            ? *lpcbData
            : 0
    );

    SAFE_FREE(value_name_w);

    IGNORED_PARAMETER(lpReserved);

    /*
     * The lpcbData parameter can be NULL only if lpData is NULL.
     */
    if (!hKey || (!lpData && lpcbData) || (lpData && !lpcbData))
        PANIC("Invalid parameter");

    const struct Registry_Entry *entry = hKey;
    const struct Registry_Value *value = search_for_value(entry, lpValueName);

    if (!(value = search_for_value(entry, lpValueName))) {
        /*
         * If lpValueName specifies a value that is not in the registry, the function returns ERROR_FILE_NOT_FOUND.
         */
        TRACE_RET("[Entry not found]");
        return ERROR_FILE_NOT_FOUND;
    }

    if (lpType)
        *(enum Value_Type *)lpType = value->type;

    /* Minesweeper only uses REG_SZ & REG_DWORD, so implementation supports only two */
    size_t length = (value->type == REG_SZ) ? (nt_wchar_strlen(value->value) + 1) * sizeof(nt_wchar_t) : 4;

    if (*lpcbData < length) {
        /*
        * If the buffer specified by lpData parameter is not large enough to hold the data, the function returns ERROR_MORE_DATA and stores the required buffer size in the variable pointed to by lpcbData.
        * If lpData is NULL, and lpcbData is non-NULL, the function returns ERROR_SUCCESS and stores the size of the data, in bytes, in the variable pointed to by lpcbData.
        */
        *lpcbData = length;
        TRACE_RET("%s", lpData ? "[Buffer is too short]" : "[Calculated required buffer size]");
        return lpData ? ERROR_MORE_DATA : ERROR_SUCCESS;
    }

    wchar_t *string_w = NULL;

    if (!lpData) {
        /*
         * This parameter can be NULL if the data is not required.
         */
        TRACE_RET("[Data is not requied]");
        return ERROR_SUCCESS;
    }

    switch (value->type) {
    case REG_SZ:
        memcpy(lpData, value->value, length);
        
        string_w = nt_wchar_to_wchar(lpData);
        if (string_w) {
            TRACE_RET("\"%ls\"", string_w);
            free(string_w);
        }
        break;

    case REG_DWORD:
        *(unsigned int *)lpData = *(unsigned int *)value->value;
        TRACE_RET("%x", *(unsigned int *)lpData);
        break;
    }

    return ERROR_SUCCESS;
}

int __stdcall RegSetValueExW(void *hKey, const nt_wchar_t *lpValueName, unsigned Reserved, unsigned dwType, void *lpData, unsigned cbData) {
    wchar_t *value_name_w = NULL;
    TRACE("\"%s\" \"%ls\" %i %p %i",
        hkey_as_cstr(hKey),
        (value_name_w = nt_wchar_to_wchar(lpValueName)),
        dwType,
        lpData,
        cbData
    );

    SAFE_FREE(value_name_w);

    if (!hKey)
        PANIC("Invalid parameter");

    struct Registry_Entry *entry = hKey;
    struct Registry_Value *value = search_for_value(entry, lpValueName);

    if (!value) {
        /*
         * If a value with this name is not already present in the key, the function adds it to the key.
         */
        void *tmp = realloc(entry->values, (sizeof *value) * ++entry->values_number);
        if (!tmp)
            PANIC("Could reallocate entries.");

        value = (entry->values = tmp) + entry->values_number - 1;
        memcpy(value->name, lpValueName, (nt_wchar_strlen(lpValueName) + 1) * sizeof(nt_wchar_t));
    }

    value->type = dwType;
    value->size = cbData;
    if (!cbData) {
        /*
         * lpData indicating a null value is valid, however, if this is the case, cbData must be set to '0'.
         */
        TRACE_RET("[No value set]");
        return ERROR_SUCCESS;
    }

    value->value = malloc(cbData);
    if (!value->value)
        PANIC("Could not allocate value.");

    memcpy(value->value, lpData, cbData);
    return ERROR_SUCCESS;
}

int __stdcall RegOpenKeyExA(void *hKey, const char *lpSubKey, unsigned ulOptions, int samDesired, void **phkResult) {
    TRACE("\"%s\" \"%s\" %x %x %p", hkey_as_cstr(hKey), lpSubKey, ulOptions, samDesired, phkResult);

    IGNORED_PARAMETER(ulOptions, samDesired);

    if (!hKey || !phkResult)
        PANIC("Invalid parameter");

    /* 
     * If the lpSubKey parameter is NULL or a pointer to an empty string, and if hKey is a predefined key, then the system refreshes the predefined key, and phkResult receives the same hKey handle passed into the function.
     */
    if (!lpSubKey)
        switch ((uintptr_t)hKey) {
        case HKEY_CLASSES_ROOT: case HKEY_CURRENT_USER: case HKEY_LOCAL_MACHINE:
        case HKEY_USERS: case HKEY_CURRENT_CONFIG:
            *phkResult = hKey;
            TRACE_RET("[No subkey, Returned input]");
            return ERROR_SUCCESS;
        }

    extern int __stdcall RegCreateKeyExW(void *hKey, const nt_wchar_t *lpSubKey, unsigned Reserved, const nt_wchar_t *lpClass, unsigned dwOptions, int samDesired, void *lpSecurityAttributes, void **phkResult, unsigned *lpdwDisposition);
    
    void *opened_key;
    unsigned disposition;
    nt_wchar_t *subkey_nt_w = char_to_nt_wchar(lpSubKey);
    if (!subkey_nt_w)
        PANIC("Could not allocate subkey");

    puts("Calling RegCreateKeyExW in order to open key. . .");
    (void) RegCreateKeyExW(hKey, subkey_nt_w, 0, NULL, 0, 0, NULL, &opened_key, &disposition);

    /* If the key was just created, it means there was no matching key. Delete & fail */
    if (disposition != REG_OPENED_EXISTING_KEY) {
        registry_size --;
        TRACE_RET("[Entry not found]");
        return ERROR_FILE_NOT_FOUND;
    }

    *phkResult = opened_key;

    free(subkey_nt_w);
    TRACE_RET("[Opened] %p", opened_key);
    return ERROR_SUCCESS;
}

int __stdcall RegQueryValueExA(void *hKey, const char *lpValueName, unsigned *lpReserved, unsigned *lpType, void *lpData, unsigned *lpcbData) {
    TRACE("\"%s\" \"%s\" %p %p %p (%u)", hkey_as_cstr(hKey), lpValueName, lpType, lpData, lpcbData, lpcbData ? *lpcbData : 0);

    IGNORED_PARAMETER(lpReserved);

    /*
     * The lpcbData parameter can be NULL only if lpData is NULL.
     */
    if (!hKey || (!lpData && lpcbData) || (lpData && !lpcbData))
        PANIC("Invalid parameter");

    extern int __stdcall RegQueryValueExW(void *hKey, const nt_wchar_t *lpValueName, void *lpReserved, unsigned *lpType, void *lpData, unsigned *lpcbData);

    nt_wchar_t *value_name_nt_w = char_to_nt_wchar(lpValueName);
    if (!value_name_nt_w)
        PANIC("Could not allocate value name");

    puts("Calling RegQueryValueExW in order to query value. . .");
    return RegQueryValueExW(hKey, value_name_nt_w, NULL, lpType, lpData, lpcbData);
}

int __stdcall RegCreateKeyExW(void *hKey, const nt_wchar_t *lpSubKey, unsigned Reserved, const nt_wchar_t *lpClass, unsigned dwOptions, int samDesired, void *lpSecurityAttributes, void **phkResult, unsigned *lpdwDisposition) {
    wchar_t *subkey_name_w = NULL, *class_name_w = NULL;
    TRACE("\"%s\" \"%ls\" \"%ls\" %x %x %p %p %p",
        hkey_as_cstr(hKey),
        (subkey_name_w = nt_wchar_to_wchar(lpSubKey)),
        !(class_name_w = nt_wchar_to_wchar(lpClass))
            ? L""
            : class_name_w,
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult,
        lpdwDisposition
    );

    SAFE_FREE(subkey_name_w);
    SAFE_FREE(class_name_w);

    IGNORED_PARAMETER(Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes);

    if (!hKey || !lpSubKey || !phkResult)
        PANIC("Invalid parameter");

    /*
     * If the key already exists, the function opens it
     */
    unsigned disposition = REG_OPENED_EXISTING_KEY;
    struct Registry_Entry *entry = search_for_entry(hKey, lpSubKey);

    if (!entry) {
        disposition = REG_CREATED_NEW_KEY;

        if (registry_size >= REGISTRY_CAP - 1)
            PANIC("Ran out of registry space");

        entry = REGISTRY + registry_size++;
        memset(entry, 0, sizeof *entry);
        memcpy(entry->subkey, lpSubKey, (nt_wchar_strlen(lpSubKey) + 1) * sizeof(nt_wchar_t));
        entry->key = hKey;
    }

    /*
     * If lpdwDisposition is NULL, no disposition information is returned.
     */
    if (lpdwDisposition)
        *lpdwDisposition = disposition;
    
    *phkResult = entry;

    TRACE_RET("\"%s\" %p", (disposition == REG_CREATED_NEW_KEY) ? "[Created]" : "[Opened]", *phkResult);
    return ERROR_SUCCESS;
}

int __stdcall RegCloseKey(void *hKey) {
    TRACE("%p", hKey);

    IGNORED_PARAMETER(hKey);

    /* Because this registry system does not work with handles, no need to close anything */
    return ERROR_SUCCESS;
}