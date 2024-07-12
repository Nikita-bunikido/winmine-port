#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "common.h"
#include "ntconv.h"

/* TODO: Unimplemented */
int __stdcall ShellAboutW(void *hWnd, const nt_wchar_t *szApp, const nt_wchar_t *szOtherStuff, void *hIcon) {
    wchar_t *app_w, *other_stuff_w;

    TRACE("%p \"%ls\" \"%ls\" %p",
        hWnd,
        (app_w = nt_wchar_to_wchar(szApp))
            ? app_w
            : L"",
        (other_stuff_w = nt_wchar_to_wchar(szOtherStuff))
            ? other_stuff_w
            : L"",
        hIcon
    );

    IGNORED_PARAMETER(hWnd, szApp, szOtherStuff, hIcon);

    /* 
     * TRUE if successful; otherwise, FALSE.
     */
    return TRUE;
}