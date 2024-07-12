.code32

.macro IAT _name, _rva, _endsym
    .globl \_name\()_iat
    .type \_name\()_iat, %object
    .align 4

\_name\()_iat:
    .long \_rva
    .long (\_endsym - \_name\()_iat) / 4 - 1
.endm

.section .nt.iat, "a", @progbits

IAT kernel32, 0x1068, _kernel32_iat_end
    .long FindResourceW
    .long OutputDebugStringA
    .long LockResource
    .long LoadResource
    .long lstrlenW
    .long GetPrivateProfileIntW
    .long GetPrivateProfileStringW
    .long GetTickCount
    .long GetModuleFileNameA
    .long GetModuleHandleA
    .long GetStartupInfoA
    .long GetProcAddress
    .long lstrcpyW
    .long LoadLibraryA
.set _kernel32_iat_end, .

IAT msvcrt, 0x1170, _msvcrt_iat_end
    .long _controlfp
    .long __set_app_type
    .long __p__fmode
    .long _except_handler3
    .long _adjust_fdiv
    .long __setusermatherr
    .long _initterm
    .long __getmainargs
    .long _acmdln
    .long exit
    .long __p__commode
    .long _cexit
    .long _XcptFilter
    .long _exit
    .long _c_exit
    .long srand
    .long rand
.set _msvcrt_iat_end, .

IAT user32, 0x10ac, _user32_iat_end
    .long LoadIconW
    .long GetDesktopWindow
    .long SetTimer
    .long MessageBoxW
    .long LoadCursorW
    .long CheckMenuItem
    .long SetMenu
    .long GetDlgItemInt
    .long RegisterClassW
    .long LoadStringW
    .long LoadMenuW
    .long ReleaseCapture
    .long PeekMessageW
    .long MapWindowPoints
    .long SetCapture
    .long PtInRect
    .long WinHelpW
    .long SetDlgItemInt
    .long EndDialog
    .long SetDlgItemTextW
    .long wsprintfW
    .long SendMessageW
    .long GetDlgItem
    .long GetDlgItemTextW
    .long GetSystemMetrics
    .long InvalidateRect
    .long SetRect
    .long MoveWindow
    .long GetMenuItemRect
    .long DialogBoxParamW
    .long DefWindowProcW
    .long ReleaseDC
    .long GetDC
    .long PostMessageW
    .long ShowWindow
    .long PostQuitMessage
    .long KillTimer
    .long EndPaint
    .long BeginPaint
    .long DispatchMessageW
    .long TranslateMessage
    .long TranslateAcceleratorW
    .long GetMessageW
    .long UpdateWindow
    .long CreateWindowExW
    .long LoadAcceleratorsW
.set _user32_iat_end, .

IAT advapi32, 0x1000, _advapi32_iat_end
    .long RegQueryValueExW
    .long RegSetValueExW
    .long RegOpenKeyExA
    .long RegQueryValueExA
    .long RegCreateKeyExW
    .long RegCloseKey
.set _advapi32_iat_end, .

IAT gdi32, 0x1024, _gdi32_iat_end
    .long SetROP2
    .long GetLayout
    .long SetLayout
    .long GetDeviceCaps
    .long DeleteObject
    .long LineTo
    .long CreatePen
    .long CreateCompatibleDC
    .long CreateCompatibleBitmap
    .long SelectObject
    .long SetDIBitsToDevice
    .long DeleteDC
    .long MoveToEx
    .long SetPixel
    .long BitBlt
    .long GetStockObject
.set _gdi32_iat_end, .

IAT comctl32, 0x101c, _comctl32_iat_end
    .long InitCommonControlsEx
.set _comctl32_iat_end, .

IAT shell32, 0x10a4, _shell32_iat_end
    .long ShellAboutW
.set _shell32_iat_end, .

IAT winmm, 0x1168, _winmm_iat_end
    .long PlaySoundW
.set _winmm_iat_end, .

.section .bss
.lcomm old_gs0, 4

.section .startup, "ax", @progbits

/* SEH prologue/epilogue tries to access fs:0, and segfaults. 
   To fix this issue without patching, there are two functions
   to prepare & restore fs:0 and fs itself. They use gs segment, 
   because it is initialized. */

.globl SEH_prepare
.type SEH_prepare, %function
SEH_prepare:
    # save gs:0
    mov %gs:0, %eax
    mov %eax, old_gs0

    # change fs to gs
    movw %gs, %ax
    movw %ax, %fs

    # zero out fs:0 not to adjust esp
    xorw %ax, %ax
    movw %ax, %fs:0

    # done
    ret

.globl SEH_restore
.type SEH_restore, %function
SEH_restore:
    # restore old gs:0
    mov old_gs0, %eax
    mov %eax, %gs:0

    # done
    ret

.globl _start
.type _start, %function
_start:
    # extract argc & argv from stack
    pop %esi
    mov %esp, %edi

    # align stack by 16 bytes
    and $0xFFFFFFF0, %esp

    # jump to C
    push $0 # ldso_dummy
    push $0 # fini_dummy
    push $0 # init_dummy
    push %edi # argv
    push %esi # argc
    push $stage2 # main
    call __libc_start_main

    # never get here
    ud2
