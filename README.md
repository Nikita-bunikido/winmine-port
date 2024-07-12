# WINMINE.EXE ported to *NIX!

Demo is running on _Alpine Linux (x86), XFCE_. (I ain't good minesweeper player, I know).

![showcase](img/preview.gif)

### Introduction

This is WINMINE.EXE ported from MS-Windows XP over to UNIX-like systems. The idea is to _keep original code "as is"_ and emulate WinApi it uses in order to make it run. So basically here is what we do to make this possible:

- PE to ELF32 format conversion.
- Section placement at specific addresses (as it was in PE):
    
    - ```.text``` (executable code) at _0x1001000_
    - ```.data``` (changeable data) at _0x1005000_
    - ```.rsrc``` (strings and images) at _0x01006000_

- Preparing & restoring ```fs:``` segment register, which was originally used to implement [SEH](https://learn.microsoft.com/en-us/cpp/cpp/structured-exception-handling-c-cpp?view=msvc-170).
- Constructing [ILT](https://ferreirasc.github.io/PE-imports/) for libraries _kernel32.dll_, _msvcrt.dll_, _user32.dll_, _advapi32.dll_, _gdi32.dll_, _comctl32.dll_, _shell32.dll_, _winmm.dll_.
- Partly implementing WinApi routines.
- Emulating GDI module, window message queue & window handling with _SDL2_.
- Conversion between Windows resource format and BMP.

### Building
Dependencies:
- X window system
- GNU coreutils
- GNU binutils
- Musl/GNU LibC
- GCC
- SDL2

I don't really want to include any proprietary software in here, so please obtain your copy of ```winmine.exe``` at [archive.org](https://archive.org/download/minesweeperxp/minesweeper.zip) and build everything together:

    git clone https://github.com/Nikita-bunikido/winmine-port.git
    cd winmine-port
    wget https://archive.org/download/minesweeperxp/minesweeper.zip 
    unzip minesweeper.zip
    mkdir bin
    make

### Running
Make sure ```.so``` libraries are visible to dynamic linker in your system & SDL2 was configured properly.

_NOTE: It's better to run compiled binary as root, because SDL2 will not be able to create window._

On **Musl LibC** do the following:

    export XDG_RUNTIME_DIR=/tmp
    export RUNLEVEL=3
    export LD_LIBRARY_PATH=<PATH TO CLONED REPO>
    ./winmine.elf

### Plans
- Test with glibc.
- Test on other systems/graphical environmnets.
- Test on MacOS.

### TODO
Some functionality is only briefly implemented, or not implemented completely yet:

- Upper menu doesn't exist.
- [Easter egg](https://www.minesweeper.info/wiki/Windows_Minesweeper#Easter_Egg) doesn't work because ```SetPixel``` is not implemented yet.
- Also I want to add ability to serialize/deserialize virtual registry to & from file.

What exactly needs to be done:

* ```mgdi32.c:SetPixel``` - Unimplemented
* ```mgdi32.c:GetDeviceCaps``` - Unimplemented
* ```mgdi32.c:GetLayout``` - Unimplemented
* ```mgdi32.c:SetLayout``` - Unimplemented
* ```mkernel32.c:GetModuleFileNameA``` - Find out usage
* ```mkernel32.c:GetStartupInfoA``` - Find out usage (Something in CRT startup, I guess)
* ```mkernel32.c:GetProcAddress``` - Find out usage (If any other library is loaded through LoadLibraryX call, we need to implement that routine and return valid function pointer)
* ```mkernel32.c:lstrcpyW``` - Unimplemented
* ```mkernel32.c:LoadLibraryA``` - Find out usage (See what libraries does it try to load)
* ```mmsvcrt.c:_controlfp``` - Find out usage (Probably unnecessary Windows-specific runtime stuff)
* ```mmsvcrt.c:__set_app_type``` - Find out usage (Totally unnecessary Windows-specific runtime stuff)
* ```mmsvcrt.c:_except_handler3``` - Find out usage (Some exception handler)
* ```mmsvcrt.c:__setusermatherr``` - Find out usage
* ```mmsvcrt.c:_initterm``` - Find out usage
* ```mmsvcrt.c:__getmainargs``` - Find out usage (Used only in CRT startup, I guess)
* ```mmsvcrt.c:_XcptFilter``` - Find out usage (Seems like yet another exception handler)
* ```mshell32.c:ShellAboutW``` - Unimplemented
* ```muser32.c:MessageBoxW``` - Find out usage (Called only in case of an error, I guess)
* ```muser32.c:LoadCursorW``` - Find out usage (Sounds really Windows-specific)
* ```muser32.c:CheckMenuItem``` - Unimplemented
* ```muser32.c:SetMenu``` - Unimplemented
* ```muser32.c:GetDlgItemInt``` - Unimplemented
* ```muser32.c:LoadMenuW``` - Unimplemented
* ```muser32.c:MapWindowPoints``` - Unimplemented
* ```muser32.c:SetCapture``` - Find out usage (Seems Windows-specific, and not really necessary for X window system)
* ```muset32.c:ReleaseCapture``` - Find out usage (Same story as ```muser32.c:SetCapture```)
* ```muser32.c:WinHelpW``` - Find out usage
* ```muser32.c:SetDlgItemInt``` - Unimplemented
* ```muser32.c:EndDialog``` - Unimplemented
* ```muser32.c:SetDlgItemTextW``` - Unimplemented
* ```muser32.c:SendMessageW``` - Find out usage
* ```muser32.c:GetDlgItem``` - Unimplemented
* ```muser32.c:GetDlgItemTextW``` - Unimplemented
* ```muser32.c:GetSystemMetrics``` - Unimplemented
* ```muser32.c:SetRect``` - Unimplemented
* ```muser32.c:GetMenuItemRect``` - Unimplemented
* ```muser32.c:DialogBoxParamW``` - Unimplemented
* ```muser32.c:DefWindowProcW``` - Unimplemented (Not sure for now if we even need to implement this one, because it's used for default message processing. We handle & translate from SDL2 message to ```struct _MSG``` only those, in which we are interested in)
* ```muser32.c:PostMessageW``` - Find out usage
* ```mwinmm.c:PlaySoundW``` - Unimplemented

### But why?...
Because it's fun.

### Why not WINE then?
I wanted to run Windows executable as natively as possible on *NIX system, without using tools such as wine.

_Feel free to contribute and enjoy!!_