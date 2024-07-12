#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <inttypes.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <SDL.h>

#include "common.h"
#include "ntconv.h"
#include "ntrsrc.h"
#include "ntgdi.h"

#define WINDOW_CLASSES_CAP      16

#define POST_MESSAGE(_type)             __extension__({                             \
    NT_ASSERT(message_queue_size <= MESSAGE_QUEUE_CAP - 1);                         \
    message_queue[message_queue_size ++] = (struct _MSG){ .message = (_type) };     \
})

static struct _WNDCLASSW main_class = { 0 };
static struct Window main_window = { 0 };

#define MESSAGE_QUEUE_CAP        1024L

static struct _MSG message_queue[MESSAGE_QUEUE_CAP] = { 0 };
static size_t message_queue_size = 0;

void __stdcall *LoadIconW(void *hInstance, const nt_wchar_t *lpIconName) {
    wchar_t *icon_name_w = NULL;

    TRACE("%p %p (\"%ls\")",
        hInstance, 
        lpIconName, 
        ((uintptr_t)lpIconName & 0xFF0000) > 0 
            ? (icon_name_w = nt_wchar_to_wchar(lpIconName))
            : L""
    );
    
    SAFE_FREE(icon_name_w);

    extern void __stdcall *FindResourceW(void *hModule, const nt_wchar_t *lpName, const nt_wchar_t *lpType);
    extern void __stdcall *LockResource(void *hResData);

    void *icon_handle = FindResourceW(hInstance, lpIconName, (const nt_wchar_t *)(uintptr_t)RT_GROUP_ICON);
    if (!icon_handle) {
        TRACE_RET("%p", NULL);
        return NULL;
    }

    void *icon = LockResource(icon_handle);
    TRACE_RET("%p", icon);
    return icon;
}

void __stdcall *GetDesktopWindow(void) {
    TRACE_NOARGS();
    
    TRACE_RET("%p", DESKTOP_WINDOW_HWND);
    return DESKTOP_WINDOW_HWND;
}

static void timer_handler(int signum) {
    main_class.lpfnWndProc(&main_window, WM_TIMER, 0, 0);
}

uintptr_t __stdcall SetTimer(void *hWnd, uintptr_t nIDEvent, unsigned uElapse, void *(*lpTimerFunc)()) {
    TRACE("%p %x %u %p", hWnd, nIDEvent, uElapse, lpTimerFunc);

    static int timer_created;
    static timer_t timer_id;

    if (!timer_created) {
        struct sigaction sa;
        memset(&sa, 0, sizeof sa);
        sa.sa_handler = timer_handler;
        sigemptyset(&sa.sa_mask);
        NT_ASSERT(!sigaction(SIGUSR1, &sa, NULL), "sigaction failed: %s", strerror(errno));

        struct sigevent sev;
        memset(&sev, 0, sizeof sev);
        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = SIGUSR1;
        sev.sigev_value.sival_ptr = &timer_id;
        NT_ASSERT(!timer_create(CLOCK_REALTIME, &sev, &timer_id), "timer_create failed: %s", strerror(errno));

        timer_created = 1;
    }

    struct itimerspec its;
    its.it_value.tv_sec = its.it_interval.tv_sec = uElapse / 1000u;
    its.it_value.tv_nsec = its.it_interval.tv_nsec = 0;
    NT_ASSERT(!timer_settime(timer_id, 0, &its, NULL), "timer_settime failed: %s", strerror(errno));

    return 1;
}

/* TODO: Find out usage */
int __stdcall MessageBoxW(void *hWnd, const nt_wchar_t *lpText, const nt_wchar_t *lpCaption, unsigned uType) {
    wchar_t *text_w, *caption_w;

    TRACE("%p \"%ls\" \"%ls\" %u",
        hWnd,
        (text_w = nt_wchar_to_wchar(lpText))
            ? text_w
            : L"",
        (caption_w = nt_wchar_to_wchar(lpCaption))
            ? caption_w
            : L"",
        uType
    );

    return 1;
}

/* TODO: Find out usage */
void __stdcall *LoadCursorW(void *instance, const nt_wchar_t *cursor_name) {
    wchar_t *cursor_name_w = NULL;

    TRACE("%p %p (\"%ls\")", 
        instance, 
        cursor_name,
        ((uintptr_t)cursor_name & 0xFF0000) > 0 
            ? (cursor_name_w = nt_wchar_to_wchar(cursor_name))
            : L""
    );
    
    if (cursor_name_w)
        free(cursor_name_w);
    ;
}

/* TODO: Unimplemented */
int __stdcall *CheckMenuItem(void *menu, unsigned item, unsigned flags) {
    TRACE("%p %u %x", menu, item, flags);
    ;
}

/* TODO: Unimplemented */
int __stdcall *SetMenu(void *window, void *menu) {
    TRACE("%p %p", window, menu);
    ;
}

/* TODO: Unimplemented */
unsigned __stdcall GetDlgItemInt(void *dialog, int item, int *status, int sign) {
    TRACE("%p %i %p \"%s\"", dialog, item, status, sign ? "TRUE" : "FALSE");
    ;
}

unsigned short __stdcall RegisterClassW(const struct _WNDCLASSW *lpWndClass) {
    wchar_t *class_name_w;
    TRACE("%p (\"%ls\")", lpWndClass,
        (class_name_w = nt_wchar_to_wchar(lpWndClass->lpszClassName))
            ? class_name_w
            : L""
        );
    SAFE_FREE(class_name_w);

    NT_ASSERT(lpWndClass, "Invalid paramater");

    main_class = *lpWndClass;
    return 1u;
}

int __stdcall LoadStringW(void *hInstance, unsigned uID, nt_wchar_t *lpBuffer, int cchBufferMax) {
    TRACE("%p %u %p %i", hInstance, uID, lpBuffer, cchBufferMax);

    if (!lpBuffer || cchBufferMax < 0)
        /* Invalid parameter */
        return 0;

    extern char _rsrc_base, _nt_base;
    const void *__rsrc_base = &_rsrc_base,
               *__nt_base   = &_nt_base;

    const struct _IMAGE_RESOURCE_DIRECTORY *type_directory = __rsrc_base;
    const struct _IMAGE_RESOURCE_DIRECTORY_ENTRY *type_entry = __rsrc_base + sizeof *type_directory;

    for (unsigned short i = type_directory->NumberOfNamedEntries + type_directory->NumberOfIdEntries; (--i > 0) && (type_entry->Name != RT_STRING); type_entry ++)
        ;

    if (type_entry->Name != RT_STRING)
        PANIC("No directory entry with ID: %i", RT_STRING);
    if (!MSB_ISSET(type_entry->OffsetToData))
        PANIC("String type directory entry does not contain address to name directory");

    const struct _IMAGE_RESOURCE_DIRECTORY_ENTRY *name_entry = __rsrc_base + MSB_CLR(type_entry->OffsetToData) + sizeof (struct _IMAGE_RESOURCE_DIRECTORY);
    if (!MSB_ISSET(name_entry->OffsetToData))
        PANIC("First name directory entry does not contain address to Language ID directory");

    const struct _IMAGE_RESOURCE_DIRECTORY_ENTRY *language_entry = __rsrc_base + MSB_CLR(name_entry->OffsetToData) + sizeof (struct _IMAGE_RESOURCE_DIRECTORY);
    const struct _IMAGE_RESOURCE_DATA_ENTRY *strings_data = __rsrc_base + language_entry->OffsetToData;
    const struct _IMAGE_RESOURCE_DIR_STRING *c_string = __nt_base + strings_data->OffsetToData;

    for (const struct _IMAGE_RESOURCE_DIR_STRING *base_string = c_string; (uID > 0) && ((char *)c_string < ((char *)base_string + strings_data->Size)); c_string = (const struct _IMAGE_RESOURCE_DIR_STRING *)((char *)c_string + sizeof(c_string->Length) + c_string->Length * 2), uID --)
        ;
    
    if (uID)
        /* String with this ID does not exist */
        return 0;

    int length = (c_string->Length + 1 > cchBufferMax) ? cchBufferMax : c_string->Length + 1;
    if (cchBufferMax) {
        memcpy(lpBuffer, c_string->NameString, length * 2);
        lpBuffer[length-1] = NT_NUL;
    } else
        *(const nt_wchar_t **)lpBuffer = c_string->NameString;

    wchar_t *c_string_w = nt_wchar_to_wchar_n(c_string->NameString, c_string->Length);
    if (c_string_w) {
        TRACE_RET("\"%ls\"", c_string_w);
        free(c_string_w);
    }

    return length;
}

/* TODO: Unimplemented */
void __stdcall *LoadMenuW(void *instance, const nt_wchar_t *menu_name) {
    wchar_t *menu_name_w = NULL;

    TRACE("%p %p (\"%ls\")", 
        instance, 
        menu_name, 
        ((uintptr_t)menu_name & 0xFFFF0000) > 0 
            ? (menu_name_w = nt_wchar_to_wchar(menu_name))
            : L""
    );
    
    if (menu_name_w)
        free(menu_name_w);

    return (void *)(uintptr_t)0xBAADBEEF;
}

/* TODO: Find out usage */
int __stdcall ReleaseCapture(void) {
    TRACE_NOARGS();
    ;
}

int __stdcall PeekMessageW(struct _MSG *lpMsg, void *hWnd, unsigned wMsgFilterMin, unsigned wMsgFilterMax, unsigned wRemoveMsg) {
    TRACE("%p %p %u %u %x", lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    
    int __stdcall GetMessageW(struct _MSG *, void *, unsigned, unsigned);

    GetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

    /*
     * If a message is available, the return value is nonzero.
     */
    return 1;
}

/* TODO: Unimplemented */
int __stdcall MapWindowPoints(void *window_from, void *window_to, struct _POINT *points, unsigned points_number) {
    TRACE("%p %p %p %u", window_from, window_to, points, points_number);
    ;
}

/* TODO: Find out usage */
void __stdcall *SetCapture(void *window) {
    TRACE("%p", window);
    ;
}

int __stdcall PtInRect(const struct _RECT *lprc, struct _POINT pt) {
    TRACE("[%li %li %li %li] [%li %li]", lprc->left, lprc->top, lprc->right, lprc->bottom, pt.x, pt.y);
    
    int result = (pt.x >= lprc->left) && (pt.x <= lprc->right)
              && (pt.y >= lprc->top) && (pt.y <= lprc->bottom);
            
    TRACE_RET("%i", result);
    return result;
}

/* TODO: Find out usage */
int __stdcall WinHelpW(void *window, const wchar_t *name, unsigned type, uintptr_t data) {
    TRACE("%p \"%ls\" %u %x", window, name, type, data);
    ;
}

/* TODO: Unimplemented */
int __stdcall SetDlgItemInt(void *dialog, int item, unsigned value, int sign) {
    TRACE("%p %i %u \"%s\"", dialog, item, value, sign ? "TRUE" : "FALSE");
    ;
}

/* TODO: Unimplemented */
int __stdcall EndDialog(void *dialog, uintptr_t result) {
    TRACE("%p %x", dialog, result);
    ;
}

/* TODO: Unimplemented */
int __stdcall SetDlgItemTextW(void *dialog, int item, const wchar_t *string) {
    TRACE("%p %i \"%ls\"", dialog, item, string);
    ;
}

int wsprintfW(nt_wchar_t *dest, const nt_wchar_t *format, ...) {
    wchar_t *format_w;

    TRACE("%p \"%ls\"",
        dest,
        (format_w = nt_wchar_to_wchar(format))
            ? format_w
            : L""
    );

    if (!format)
        PANIC("Invalid parameter");

    /*
     * The maximum size of the buffer is 1,024 bytes.
     */
    static wchar_t printf_pool[1024] = { 0 };

    va_list vptr;
    va_start(vptr, format);
    vswprintf(printf_pool, sizeof printf_pool / sizeof(wchar_t), format_w, vptr);
    va_end(vptr);


    nt_wchar_t *printf_pool_n = wchar_to_nt_wchar(printf_pool);
    if (!printf_pool_n)
        PANIC("Could not allocate printf result string");

    memcpy(dest, printf_pool_n, (nt_wchar_strlen(printf_pool_n) + 1) * sizeof(nt_wchar_t));
    free(format_w);
    free(printf_pool_n);

    TRACE_RET("\"%ls\"", printf_pool);
    /*
     * If the function succeeds, the return value is the number of characters stored in the output buffer, not counting the terminating null character.
     */
    return nt_wchar_strlen(printf_pool_n);
}

/* TODO: Find out usage */
uintptr_t __stdcall SendMessageW(void *window, unsigned message, uintptr_t wparam, uintptr_t lparam) {
    TRACE("%p %u %x %x", window, message, wparam, lparam);
    ;
}

/* TODO: Unimplemented */
void __stdcall *GetDlgItem(void *dialog, int item) {
    TRACE("%p %i", dialog, item);
    ;
}

/* TODO: Unimplemented */
unsigned __stdcall GetDlgItemTextW(void *dialog, int item, wchar_t *string, int string_max) {
    TRACE("%p %i %p %i", dialog, item, string, string_max);
    ;
}

/* TODO: Unimplemented */
int __stdcall GetSystemMetrics(int index) {
    TRACE("%i", index);
    ;
}

int __stdcall InvalidateRect(void *window, const struct _RECT *rect, int background_erase) {
    TRACE("%p %p \"%s\"", window, rect, background_erase ? "TRUE" : "FALSE");
    
    POST_MESSAGE(WM_PAINT);
}

/* TODO: Unimplemented */
int __stdcall SetRect(struct _RECT *rectangle, int x, int y, int right, int bottom) {
    TRACE("%p %i %i %i %i", rectangle, x, y, right, bottom);
    ;
}

int __stdcall MoveWindow(void *hWnd, int X, int Y, int nWidth, int nHeight, int bRepaint) {
    TRACE("%p %i %i %i %i \"%s\"", hWnd, X, Y, nWidth, nHeight, bRepaint ? "TRUE" : "FALSE");

    struct Window *win = hWnd;
    SDL_SetWindowSize(win->window, nWidth, nHeight);

    if (win->dc.bitmap->data)
        SDL_DestroyTexture(win->dc.bitmap->data);

    unsigned background_color = (unsigned)(uintptr_t)win->dc.brush->data;

    win->dc.bitmap->data = SDL_CreateTexture(win->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, MAX(nWidth, 1), MAX(nHeight, 1));
    NT_ASSERT(win->dc.bitmap->data);

    chk(SDL_SetRenderTarget(win->renderer, win->dc.bitmap->data));
    chk(SDL_SetRenderDrawColor(win->renderer, COLOR_GET_R(background_color), COLOR_GET_G(background_color), COLOR_GET_B(background_color), 255));
    chk(SDL_RenderClear(win->renderer));
    chk(SDL_SetRenderTarget(win->renderer, NULL));

    if (bRepaint)
        POST_MESSAGE(WM_PAINT);

    return 1;
}

/* TODO: Unimplemented */
int __stdcall GetMenuItemRect(void *window, void *menu, unsigned item, struct _RECT *rectangle) {
    TRACE("%p %p %u %p", window, menu, item, rectangle);
    ;
}

/* TODO: Unimplemented */
uintptr_t __stdcall DialogBoxParamW(void *instance, const wchar_t *template_name, void *parent, void * (*dialog_function)(), uintptr_t init_param) {
    TRACE("^%p \"%ls\" %p %p %x", instance, template_name, parent, dialog_function, init_param);
    ;
}

/* TODO: Unimplemented */
uintptr_t __stdcall DefWindowProcW(void *window, unsigned message, uintptr_t wparam, uintptr_t lparam) {
    TRACE("%p %u %x %x", window, message, wparam, lparam);
    ;
}

int __stdcall ReleaseDC(void *hWnd, void *hDc) {
    TRACE("%p %p", hWnd, hDc);
    
    NT_ASSERT(hDc, "invalid parameter");
    /* Do nothing, because implementation does not work with handles */
    return 1;
}

void __stdcall *GetDC(void *hWnd) {
    TRACE("%p", hWnd);

    TRACE_RET("%p", hWnd);
    /* Because dc is first field in hWnd */
    return (void *)(struct GDI_DC *)hWnd;
}

/* TODO: Find out usage */
int __stdcall PostMessageW(void *window, unsigned message, uintptr_t wparam, uintptr_t lparam) {
    TRACE("%p %u %x %x", window, message, wparam, lparam);
    ;
}

int __stdcall ShowWindow(void *hWnd, int nCmdShow) {
    TRACE("%p %i", hWnd, nCmdShow);

    if (!hWnd)
        PANIC("Invalid parameter");
    
    struct Window *window = hWnd;

    ((void (*[2])(SDL_Window *)){ SDL_HideWindow, SDL_ShowWindow }[nCmdShow == SW_SHOWNORMAL]) (window->window);

    ;
}

void __stdcall PostQuitMessage(int code) {
    TRACE("%i", code);
    POST_MESSAGE(WM_QUIT);
}

int __stdcall KillTimer(void *window, uintptr_t timer) {
    TRACE("%p %i", window, timer);
    ;
}

int __stdcall EndPaint(void *window, const struct _PAINTSTRUCT *paint) {
    TRACE("%p %p", window, paint);
    ;
}

void __stdcall *BeginPaint(void *hWnd, struct _PAINTSTRUCT *lpPaint) {
    TRACE("%p %p", hWnd, lpPaint);
    
    NT_ASSERT(hWnd && lpPaint, "invalid parameter");

    struct Window *window = hWnd;
    memset(lpPaint, 0, sizeof *lpPaint);
    lpPaint->hdc = &window->dc;

    int w, h;
    SDL_GetWindowSizeInPixels(window->window, &w, &h);
    lpPaint->rcPaint.right = w;
    lpPaint->rcPaint.bottom = h;

    TRACE_RET("%p", lpPaint->hdc);
    return lpPaint->hdc;
}

uintptr_t __stdcall DispatchMessageW(const struct _MSG *lpMsg) {
    TRACE("%p", lpMsg);

    NT_ASSERT(lpMsg, "invalid parameter");

    NT_ASSERT(main_class.lpfnWndProc);
    main_class.lpfnWndProc(&main_window, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
    ;
}

int __stdcall TranslateMessage(const struct _MSG *message) {
    TRACE("%p", message);
    ;
}

int __stdcall TranslateAcceleratorW(void *window, void *acc_table, struct _MSG *message) {
    TRACE("%p %p %p", window, acc_table, message);
    ;
}

static int sdl_event_to_msg(struct _MSG *message, SDL_Event event, void *hwnd) {
    memset(message, 0, sizeof *message);
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        if (SDL_GetWindowID(main_window.window) != event.button.windowID)
            return 1;

        switch (event.button.button) {
        case SDL_BUTTON_LEFT:
            *message = (struct _MSG){ .message = (event.button.state == SDL_PRESSED) ? WM_LBUTTONDOWN : WM_LBUTTONUP, .wParam = MK_LBUTTON };
            break;
        
        case SDL_BUTTON_RIGHT:
            *message = (struct _MSG){ .message = (event.button.state == SDL_PRESSED) ? WM_RBUTTONDOWN : WM_RBUTTONUP, .wParam = MK_RBUTTON };
            break;
        
        case SDL_BUTTON_MIDDLE:
            *message = (struct _MSG){ .message = (event.button.state == SDL_PRESSED) ? WM_MBUTTONDOWN : WM_MBUTTONUP, .wParam = MK_MBUTTON };
            break;
        }

        message->pt = (struct _POINT){ event.button.x, event.button.y };
        /*
         * The low-order word specifies the x-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
         * The high-order word specifies the y-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
         */
        message->lParam = event.button.x | (event.button.y << 16);
        break;

    case SDL_MOUSEMOTION:
        if (SDL_GetWindowID(main_window.window) != event.button.windowID)
            return 1;
        
        message->message = WM_MOUSEMOVE;
        message->pt = (struct _POINT){ event.motion.x, event.motion.y };
        printf("[%li %li]\n", message->pt.x, message->pt.y);
        /* It should also fill wParam with pressed mouse key, but we omit this */ 
        
        /*
         * The low-order word specifies the x-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
         * The high-order word specifies the y-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
         */
        message->lParam = event.motion.x | (event.motion.y << 16);
        break;

    case SDL_QUIT:
        message->message = WM_DESTROY;
        break;

    default:
        return 1;
    }

    message->hwnd = hwnd;
    return 0;
}

int __stdcall GetMessageW(struct _MSG *lpMsg, void *hWnd, unsigned wMsgFilterMin, unsigned wMsgFilterMax) {
    TRACE("%p %p %u %u", lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

    NT_ASSERT(lpMsg, "invalid parameter");

    SDL_Event event;
    if (SDL_PollEvent(&event) && (message_queue_size < MESSAGE_QUEUE_CAP))
        if (!sdl_event_to_msg(message_queue + message_queue_size, event, hWnd))
            message_queue_size ++;

    *lpMsg = (message_queue_size > 0) ? message_queue[-- message_queue_size] : (struct _MSG){ .message = WM_NULL };

    /* Rendering goes here */
    chk(SDL_RenderClear(main_window.renderer));
    chk(SDL_RenderCopy(main_window.renderer, main_window.dc.bitmap->data, NULL, NULL));
    SDL_RenderPresent(main_window.renderer);

    /*
     * If the function retrieves a message other than WM_QUIT, the return value is nonzero.
     * If the function retrieves the WM_QUIT message, the return value is zero.
     */

    TRACE_RET("%u [%li %li]", lpMsg->message, lpMsg->pt.x, lpMsg->pt.y);
    return lpMsg->message != WM_QUIT;
}

int __stdcall UpdateWindow(void *hWnd) {
    TRACE("%p", hWnd);
    
    NT_ASSERT(hWnd, "invalid parameter");

    /*
     * The UpdateWindow function updates the client area of the specified window by sending a WM_PAINT message to the window if the window's update region is not empty.
     */
    return main_class.lpfnWndProc(&main_window, WM_PAINT, 0, 0);
}

void *__CreateWindowExW_internal(const char *title, int x, int y, int width, int height) {
    SDL_Window *window = SDL_CreateWindow(title ? title : "",
                              x, y, width, height, 0);
    NT_ASSERT(window, "failed to create window: %s", SDL_GetError());

    SDL_Renderer *renderer = SDL_CreateRenderer(window,
                       -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
    NT_ASSERT(renderer, "failed to create renderer: %s", SDL_GetError());

    /* Prepare dc */
    struct GDI_DC dc;

    dc.renderer = renderer;
    dc.bitmap   = malloc(sizeof(*dc.bitmap));
    NT_ASSERT(dc.bitmap, "failed to allocate bitmap: %s", strerror(errno));

    *(dc.bitmap) = (struct GDI_Object) {
        .type = GDI_BITMAP,
        .data = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, MAX(width, 1), MAX(height, 1))
    };
    NT_ASSERT(dc.bitmap->data, "failed to create bitmap: %s", SDL_GetError());

    void __stdcall *CreatePen(int iStyle, int cWidth, unsigned color);
    dc.pen = CreatePen(0, 1, 0xffffffff);
    dc.brush = main_class.hbrBackground;
    dc.rop2 = R2_COPYPEN;

    unsigned background_color = (unsigned)(uintptr_t)dc.brush->data;

    /* Fill background with brush */
    chk(SDL_SetRenderTarget(renderer, dc.bitmap->data));
    chk(SDL_SetRenderDrawColor(renderer, COLOR_GET_R(background_color), COLOR_GET_G(background_color), COLOR_GET_B(background_color), 255));
    chk(SDL_RenderClear(renderer));
    chk(SDL_SetRenderTarget(renderer, NULL));

    /* Create icon */
    const struct _GRPICONDIRENTRY icon_entry = ((const struct _GRPICONDIR *)main_class.hIcon)->idEntries[6];
    NT_ASSERT(icon_entry.bWidth == 32 && icon_entry.bHeight == 32, "no window icon");

    extern void __stdcall *FindResourceW(void *hModule, const nt_wchar_t *lpName, const nt_wchar_t *lpType);

    const struct _IMAGE_RESOURCE_DATA_ENTRY *icon = FindResourceW((void *)__nt_base, (const nt_wchar_t *)(uintptr_t)icon_entry.nId, (const nt_wchar_t *)(uintptr_t)RT_ICON);
    NT_ASSERT(icon, "cannot get window icon handle");
    
    const struct _BITMAPINFOHEADER *info_header = __nt_base + icon->OffsetToData;
    const void *icon_raw = (const void *)(__nt_base + icon->OffsetToData + sizeof(*info_header));

    size_t icon_size;
    void *icon_bmp = header_and_data_to_bmp(info_header, icon_raw, icon_entry.bWidth, icon_entry.bHeight, &icon_size);

    SDL_RWops *icon_stream = SDL_RWFromConstMem(icon_bmp, icon_size);
    NT_ASSERT(icon_stream, "SDL_RWFromConstMem failed: %s", SDL_GetError());

    SDL_Surface *icon_surface = SDL_LoadBMP_RW(icon_stream, 1);
    NT_ASSERT(icon_surface, "SDL_LoadBMP_RW failed: %s", SDL_GetError());

    SDL_SetWindowIcon(window, icon_surface);

    SDL_FreeSurface(icon_surface);
    free(icon_bmp);

    /* TODO: implement other gdi objects */

    main_window = (struct Window) {
        .dc = dc,
        .window = window,
        .renderer = renderer
    };

    return &main_window;
}

void __stdcall *CreateWindowExW(unsigned dwExStyle, const nt_wchar_t *lpClassName, const nt_wchar_t *lpWindowName, unsigned dwStyle, int X, int Y, int nWidth, int nHeight, void *hWndParent, void *hMenu, void *hInstance, void *lpParam) {
    wchar_t *class_name_w, *window_name_w;

    TRACE("%x \"%ls\" \"%ls\" %x %i %i %i %i %p %p %p %p",
        dwExStyle,
        (class_name_w = nt_wchar_to_wchar(lpClassName))
            ? class_name_w
            : L"",
        (window_name_w = nt_wchar_to_wchar(lpWindowName))
            ? window_name_w
            : L"",
        dwStyle,
        X, Y,
        nWidth, nHeight,
        hWndParent,
        hMenu,
        hInstance,
        lpParam
    );

    NT_ASSERT(lpClassName, "Invalid parameter");
    NT_ASSERT(!IS_ATOM(lpClassName), "Specifying class name by atom is not implemented");

    const size_t window_name_length = wcslen(window_name_w);
    char window_name[window_name_length + 1];
    wcstombs(window_name, window_name_w, window_name_length + 1);

    void *window = __CreateWindowExW_internal(window_name, X, Y, nWidth, nHeight);

    free(class_name_w);
    SAFE_FREE(window_name_w);

    TRACE_RET("%p", window);
    return window;
}

void __stdcall *LoadAcceleratorsW(void *instance, const nt_wchar_t *table_name) {
    wchar_t *table_name_w = NULL;

    TRACE("%p %p (\"%ls\")", 
        instance, 
        table_name, 
        ((uintptr_t)table_name & 0xFF0000) > 0 
            ? (table_name_w = nt_wchar_to_wchar(table_name))
            : L""
    );
    
    if (table_name_w)
        free(table_name_w);
}