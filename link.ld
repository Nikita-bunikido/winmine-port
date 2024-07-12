OUTPUT_FORMAT(elf32-i386)

_entry_point = 0x1003E21;
ENTRY(_start)

SECTIONS {
    . = 0x1000000;
    PROVIDE(_nt_base = .);

    .text 0x1001000 : ALIGN(4) {
        PROVIDE(_text_base = .);
        text.bin.elf (.data)
    }

    .data 0x1005000 : ALIGN(4) {
        PROVIDE(_data_base = .);
        data.bin.elf (.data)
    }

    .rsrc 0x01006000 : ALIGN(4) {
        PROVIDE(_rsrc_base = .);
        rsrc.bin.elf (.data)
    }

    .dynbss : { *(.dynbss) }
    .interp : { *(.interp) }
    .dynamic : { *(.dynamic) }
    .symtab : { *(.symtab) }
    .strtab : { *(.strtab) }
    .plt : { *(.plt) }
    .got.plt : { *(.got.plt) }
    .startup : { *(.startup) }
    .alttext : { *(.alttext) }
}