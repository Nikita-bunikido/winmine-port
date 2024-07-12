# Compiler settings
CFLAGS =     -std=gnu99 -O0 -ggdb \
			 -fno-stack-protector -fno-common \
		     -Wno-pointer-arith -Wno-strict-aliasing \
		     -Werror=ignored-attributes -Werror=implicit-function-declaration -Wno-strict-aliasing \
			 -D_GNU_SOURCE -D_POSIX_C_SOURCE=199309L

CFLAGS_LIB = $(CFLAGS) -march=native -mincoming-stack-boundary=2 -mpreferred-stack-boundary=4 \
			 -fPIC -shared \
			 -nostdlib -nostartfiles -nolibc \
 		     `sdl2-config --cflags`

CFLAGS_OBJ = $(CFLAGS) -march=native -mincoming-stack-boundary=2 -mpreferred-stack-boundary=4 \
			`sdl2-config --cflags`

# Linker settings
SHARED_LIBS_NAMES = mmsvcrt   \
					mkernel32 \
					muser32   \
					madvapi32 \
					mcomctl32 \
					mgdi32    \
					mshell32  \
					mwinmm

SHARED_LIBS = $(addsuffix .so, $(addprefix lib, $(SHARED_LIBS_NAMES)))

LD_FLAGS    = -m elf_i386 \
			  --script link.ld \
			  --dynamic-linker /lib/ld-musl-i386.so.1

LD_LIBS_LIB = -L /usr/lib -lc
LD_LIBS     = -L /usr/lib -lc \
			  -L . $(addprefix -l, $(SHARED_LIBS_NAMES)) \
			  -L /usr/local/lib --enable-new-dtags -lSDL2

# Targets
TARGET      = winmine.elf

.PHONY: all
all: $(SHARED_LIBS) $(TARGET)

%: %.c
	cc $(CFLAGS) -o $@ $<

%.o: %.S
	as -o $@ $<

%.o: %.c
	cc -c $(CFLAGS_OBJ) -o $@ $<

lib%.so: %.c
	cc $(CFLAGS_LIB) -o $@ $< $(LD_LIBS_LIB)

bin/text.bin: winmine.exe
	dd if=$< of=$@ bs=1 skip=1024 count=14934

bin/data.bin: winmine.exe
	dd if=$< of=$@ bs=1 skip=16384 count=512

bin/rsrc.bin: winmine.exe
	dd if=$< of=$@ bs=1 skip=16896 count=102752

%.bin.elf: bin/%.bin
	objcopy --input-target=binary --output-target=elf32-i386 $< $@

$(TARGET): data.bin.elf text.bin.elf rsrc.bin.elf iat.o startup.o ntconv.o ntrsrc.o ntgdi.o
	ld $(LD_FLAGS) -o $@ $^ $(LD_LIBS)
	objcopy --set-section-flags .text=contents,alloc,load $@ $@
	chmod +x $@

.PHONY: clean
clean:
	rm $(SHARED_LIBS)
	rm $(TARGET)
	rm *.bin.elf
	rm bin/*
	rm *.o