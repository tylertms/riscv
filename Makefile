TC      := riscv64-elf
CC      := $(TC)-gcc
OBJCOPY := $(TC)-objcopy
BLD     := _build/programs

CFLAGS  := -march=rv32i -mabi=ilp32 -ffreestanding -fno-pic -Os -ffunction-sections -fdata-sections
LDFLAGS := -march=rv32i -mabi=ilp32 -nostartfiles -nostdlib -Wl,-T,bram.ld,-e,start,--gc-sections


.PHONY: clean
%:
	@set -e; n=$@; test -f programs/$$n.c || { echo "missing programs/$$n.c"; exit 1; }; \
	mkdir -p $(BLD); \
	printf '%s\n' '.option norvc' '.section .text' '.globl start' 'start:' \
	'  lui   gp, 0x400' '  lui   sp, 0x2' '  addi  sp, sp, -2048' '  j     main' \
	> $(BLD)/crt0.s; \
	$(CC) $(CFLAGS) -c -x assembler $(BLD)/crt0.s -o $(BLD)/crt0.o; \
	$(CC) $(CFLAGS) -c programs/$$n.c -o $(BLD)/$$n.o; \
	$(CC) $(LDFLAGS) $(BLD)/crt0.o $(BLD)/$$n.o -lgcc -o $(BLD)/$$n.elf; \
	$(OBJCOPY) -O binary $(BLD)/$$n.elf $(BLD)/$$n.bin; \
	sz=$$(stat -f%z $(BLD)/$$n.bin); pad=$$(( (4 - (sz % 4)) % 4 )); \
	[ $$pad -eq 0 ] || dd if=/dev/zero bs=1 count=$$pad >> $(BLD)/$$n.bin 2>/dev/null; \
	hexdump -v -e '1/4 "%08x\n"' $(BLD)/$$n.bin > program.hex; \
	echo "OK -> program.hex"

clean:
	rm -rf _build program.hex
