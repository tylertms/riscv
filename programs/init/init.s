.equ IO_BASE, 0x400000

.text
.global _start
.type _start, @function

_start:
.option push
.option norelax
    li   gp, IO_BASE
.option pop
    li   sp, 0x1800

# 1) Clear .bss
    la a0, _sbss
    la a1, _ebss
    bge a0, a1, 1f
0:
    sw zero, 0(a0)
    addi a0, a0, 4
    blt a0, a1, 0b
1:

# 2) Copy .fast (FLASH -> RAM)
    la a0, _sfast_load
    la a1, _sfast
    la a2, _efast
    bge a1, a2, 2f
0:
    lw a3, 0(a0)
    sw a3, 0(a1)
    addi a0, a0, 4
    addi a1, a1, 4
    blt a1, a2, 0b
2:

# 3) Copy .data (FLASH -> RAM)
    la a0, _sdata_load
    la a1, _sdata
    la a2, _edata
    bge a1, a2, 3f
0:
    lw a3, 0(a0)
    sw a3, 0(a1)
    addi a0, a0, 4
    addi a1, a1, 4
    blt a1, a2, 0b
3:

# 4) Jump to C
    call main
    ebreak
0:  j 0b
