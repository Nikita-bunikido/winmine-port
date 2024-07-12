#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "common.h"
#include "ntconv.h"

/* TODO: Unimplemented */
int __stdcall PlaySoundW(const nt_wchar_t *pszSound, void *hmod, unsigned fdwSound) {
    wchar_t *sound_w;

    TRACE("\"%ls\" %p %u", 
        (sound_w = nt_wchar_to_wchar(pszSound))
            ? sound_w
            : L"",
        hmod,
        fdwSound
    );

    IGNORED_PARAMETER(pszSound, hmod, fdwSound);

    /*
     * Returns TRUE if successful or FALSE otherwise.
     */
    return TRUE;
}