TC      := riscv64-elf
CC      := $(TC)-gcc
OBJCOPY := $(TC)-objcopy
BLD     := _build/programs

CFLAGS  := -march=rv32i -mabi=ilp32 -ffreestanding -fno-pic -O3 -ffast-math -ffunction-sections -fdata-sections
LDFLAGS := -march=rv32i -mabi=ilp32 -nostartfiles -nostdlib -Wl,-T,bram.ld,-e,start,--gc-sections

.PHONY: clean
%:
	@set -e; n=$@; test -f programs/$$n.c || { echo "missing programs/$$n.c"; exit 1; }; \
	mkdir -p $(BLD); \
	$(CC) $(CFLAGS) -I. -c programs/$$n.c -o $(BLD)/$$n.o; \
	$(CC) $(LDFLAGS) $(BLD)/$$n.o -lgcc -o $(BLD)/$$n.elf; \
	$(OBJCOPY) -O binary $(BLD)/$$n.elf $(BLD)/$$n.bin; \
	sz=$$(stat -f%z $(BLD)/$$n.bin); pad=$$(( (4 - (sz % 4)) % 4 )); \
	[ $$pad -eq 0 ] || dd if=/dev/zero bs=1 count=$$pad >> $(BLD)/$$n.bin 2>/dev/null; \
	hexdump -v -e '1/4 "%08x\n"' $(BLD)/$$n.bin > program.hex; \
	echo "OK -> program.hex"

upload:
	yosys -p "synth_ice40 -top system -json _build/default/hardware.json" -q system.v
	nextpnr-ice40 --hx1k --package vq100 --json _build/default/hardware.json --asc _build/default/hardware.asc --report _build/default/hardware.pnr --pcf go-board.pcf -q
	icepack -s _build/default/hardware.asc _build/default/hardware.bin
	iceprog -d d:32/1 _build/default/hardware.bin

clean:
	rm -rf _build program.hex
