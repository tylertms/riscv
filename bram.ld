/* bram.ld — RV32I BRAM, split code (RX) and data (RW) */
ENTRY(start)

MEMORY
{
  BRAM (rwx) : ORIGIN = 0x00000000, LENGTH = 0x00001800  /* 6 KiB */
}

/* Program headers: PF_R|PF_X = 0x5, PF_R|PF_W = 0x6 */
PHDRS
{
  text PT_LOAD FLAGS(0x5);  /* R|X */
  data PT_LOAD FLAGS(0x6);  /* R|W */
}

SECTIONS
{
  . = ORIGIN(BRAM);

  /* Code + rodata → RX */
  .text : {
    KEEP(*(.init))
    *(.text .text.*)
    *(.rodata .rodata.*)
    *(.srodata .srodata.*)
  } > BRAM :text
  . = ALIGN(4);

  /* Writable data → RW */
  .data : {
    *(.data .data.*)
    *(.sdata .sdata.*)
  } > BRAM :data
  . = ALIGN(4);

  /* BSS: RW, no file contents */
  .bss (NOLOAD) : {
    *(.bss .bss.*)
    *(.sbss .sbss.*)
    *(COMMON)
  } > BRAM :data
  . = ALIGN(4);

  /DISCARD/ : { *(.comment*) *(.note*) *(.eh_frame*) }
}
