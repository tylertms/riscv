/* 64 KiB SPI flash code, 6 KiB RAM */
MEMORY {
  FLASH (rx)  : ORIGIN = 0x00810000, LENGTH = 0x10000
  RAM   (rwx) : ORIGIN = 0x00000000, LENGTH = 0x1800
}

ENTRY(_start)

SECTIONS
{
  /* -------- CODE IN FLASH -------- */
  .text : {
    . = ALIGN(4);
    */programs/init/init.o(.text*)             /* startup in flash */
    *(.eh_frame_hdr) *(.eh_frame) *(.gcc_except_table*)
    KEEP(*(.init_array .init_array.*))
    *(EXCLUDE_FILE(*/libgcc.a:*.o) .text .text.*)
    . = ALIGN(4);
    _etext  = .;
    _sidata = _etext;         /* load addr for .data/.rodata below */
  } > FLASH

  /* -------- FAST CODE IN RAM (copied from flash) -------- */
  .fast : ALIGN(4) {
    _sfast = .;

    /* keep your own tagged functions in RAM */
    KEEP(*(.fast .fast.*))

    /* place ALL libgcc members' code in RAM */
    */libgcc.a:*.o(.text .text.*)

    . = ALIGN(4);
    _efast = .;
  } > RAM AT > FLASH
  _sfast_load = LOADADDR(.fast);

  /* -------- INITIALIZED DATA IN RAM (copied from flash) -------- */
  .data : ALIGN(4) {
    _sdata = .;
    *(.data .data.* .sdata .sdata.*)
    *(.rodata .rodata.* .srodata .srodata.*)   /* <— pulled into RAM */
    . = ALIGN(4);
    _edata = .;
  } > RAM AT > FLASH
  _sdata_load = LOADADDR(.data);

  /* -------- BSS IN RAM -------- */
  .bss (NOLOAD) : ALIGN(4) {
    _sbss = .;
    *(.bss .bss.* .sbss .sbss.*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } > RAM

  _stack_top = ORIGIN(RAM) + LENGTH(RAM);
}
