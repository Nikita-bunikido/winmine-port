#ifndef __NT_RSRC_H__
#define __NT_RSRC_H__

#include "ntconv.h"

struct _IMAGE_RESOURCE_DIRECTORY {
    unsigned long   Characteristics;
    unsigned long   TimeDateStamp;
    unsigned short  MajorVersion;
    unsigned short  MinorVersion;
    unsigned short  NumberOfNamedEntries;
    unsigned short  NumberOfIdEntries;
};

struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
    unsigned long   Name;
    unsigned long   OffsetToData;
};

struct _IMAGE_RESOURCE_DIR_STRING {
    unsigned short  Length;
    nt_wchar_t   NameString[];
};

struct _IMAGE_RESOURCE_DATA_ENTRY {
    unsigned long   OffsetToData;
    unsigned long   Size;
    unsigned long   CodePage;
    unsigned long   Reserved;
};

struct __attribute__((__packed__)) _GRPICONDIRENTRY {
    unsigned char bWidth;
    unsigned char  bHeight;
    unsigned char  bColorCount;
    unsigned char  bReserved;
    unsigned short  wPlanes;
    unsigned short  wBitCount;
    unsigned dwBytesInRes;
    unsigned short  nId;
};

struct __attribute__((__packed__)) _GRPICONDIR {
    unsigned short idReserved;
    unsigned short idType;
    unsigned short idCount;
    struct _GRPICONDIRENTRY idEntries[];
};

const struct _IMAGE_RESOURCE_DIRECTORY_ENTRY *search_for_resource_entry(const struct _IMAGE_RESOURCE_DIRECTORY *, const nt_wchar_t *);

#endif /* __NT_RSRC_H__ */