ENTRY(start)

MEMORY
{
  BRAM (rwx) : ORIGIN = 0x00000000, LENGTH = 0x00001800  /* 6 KiB */
}

PHDRS
{
  text PT_LOAD FLAGS(0x5);  /* R|X */
  data PT_LOAD FLAGS(0x6);  /* R|W */
}

SECTIONS
{
  . = ORIGIN(BRAM);

  .text : {
    KEEP(*(.init))
    *(.text .text.*)
    *(.rodata .rodata.*)
    *(.srodata .srodata.*)
  } > BRAM :text
  . = ALIGN(4);

  .data : {
    *(.data .data.*)
    *(.sdata .sdata.*)
  } > BRAM :data
  . = ALIGN(4);

  .bss (NOLOAD) : {
    *(.bss .bss.*)
    *(.sbss .sbss.*)
    *(COMMON)
  } > BRAM :data
  . = ALIGN(4);

  /DISCARD/ : { *(.comment*) *(.note*) *(.eh_frame*) }
}
