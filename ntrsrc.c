#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "common.h"
#include "ntconv.h"
#include "ntrsrc.h"

__alt_text
static const struct _IMAGE_RESOURCE_DIRECTORY_ENTRY *search_for_named_resource_entry(const struct _IMAGE_RESOURCE_DIRECTORY *directory, const nt_wchar_t *name) {
    const struct _IMAGE_RESOURCE_DIRECTORY_ENTRY *c_entry = (void *)directory + sizeof *directory;

    for (unsigned short i = directory->NumberOfNamedEntries; i-- > 0; c_entry ++) {
        if (!MSB_ISSET(c_entry->Name))
            PANIC("No name offset in named resource directory entry");
        
        const struct _IMAGE_RESOURCE_DIR_STRING *name_string = __rsrc_base + MSB_CLR(c_entry->Name);
        if (!nt_wchar_strcmp(name_string->NameString, name)) return c_entry;
    }

    return NULL;
}

__alt_text
static const struct _IMAGE_RESOURCE_DIRECTORY_ENTRY *search_for_unnamed_resource_entry(const struct _IMAGE_RESOURCE_DIRECTORY *directory, unsigned id) {
    const struct _IMAGE_RESOURCE_DIRECTORY_ENTRY *c_entry = (void *)directory + sizeof *directory + sizeof *c_entry * directory->NumberOfNamedEntries;

    for (unsigned short i = directory->NumberOfIdEntries; i-- > 0; c_entry ++) {
        if (MSB_ISSET(c_entry->Name))
            PANIC("ID resource directory entry contains name offset");
        
        if (c_entry->Name == id)
            return c_entry;
    }

    return NULL;
}

__alt_text
const struct _IMAGE_RESOURCE_DIRECTORY_ENTRY *search_for_resource_entry(const struct _IMAGE_RESOURCE_DIRECTORY *directory, const nt_wchar_t *name) {
    return MSB_ISSET((unsigned)(uintptr_t)name)
        ? search_for_named_resource_entry(directory, name)
        : search_for_unnamed_resource_entry(directory, (unsigned)(uintptr_t)name);
}