# Toolchain / paths
TC       ?= riscv64-unknown-elf
CC        = $(TC)-gcc
OBJCOPY   = $(TC)-objcopy
READELF := $(TC)-readelf
OBJDUMP := $(TC)-objdump
SIZE    := $(TC)-size

SRC_DIR   = programs
BLD       = _build/programs

ARCH     ?= rv32i
ABI      ?= ilp32
MARCHABI  = -march=$(ARCH) -mabi=$(ABI)

CFLAGS   := $(MARCHABI) -ffreestanding -fno-pic -O3 -ffunction-sections -fdata-sections
LDFLAGS  := $(MARCHABI) -T default.ld -nostartfiles -nostdlib -Wl,--gc-sections
START    := $(BLD)/init.o

.PHONY: all
all:
	@echo "Usage: make <name>.prog  (e.g., make prng.prog)"

$(BLD):
	@mkdir -p $@

# Startup
$(BLD)/init.o: $(SRC_DIR)/init/init.s | $(BLD)
	$(CC) $(MARCHABI) -c $< -o $@

# Compile C -> object
$(BLD)/%.o: $(SRC_DIR)/%.c | $(BLD)
	$(CC) $(CFLAGS) -I. -c $< -o $@

# Link startup + program -> ELF
$(BLD)/%.elf: $(BLD)/%.o $(START)
	$(CC) $(LDFLAGS) $(START) $< -lgcc -o $@

# ELF -> BIN + pad
$(BLD)/%.bin: $(BLD)/%.elf
	$(OBJCOPY) -O binary $< $@
	@sz=$$(wc -c < $@); pad=$$(( (4 - (sz % 4)) % 4 )); \
	if [ $$pad -ne 0 ]; then dd if=/dev/zero bs=1 count=$$pad >> $@ 2>/dev/null; fi

%.bin: $(BLD)/%.bin
	@ln -sf $< $@ 2>/dev/null || cp $< $@

%.prog: $(BLD)/%.bin
	iceprog -d d:32/1 -o 64k $<
	@echo "Flashed $<"

.PHONY: upload
upload:
	yosys -p "synth_ice40 -top system -json _build/default/hardware.json" -q \
	     src/memory.v src/riscv_32i.v src/spi_flash.v src/system.v
	nextpnr-ice40 --hx1k --package vq100 \
	     --json _build/default/hardware.json \
	     --asc _build/default/hardware.asc \
	     --report _build/default/hardware.pnr \
	     --pcf go-board.pcf -q
	icepack -s _build/default/hardware.asc _build/default/hardware.bin
	iceprog -d d:32/1 _build/default/hardware.bin

.PHONY: init
init:
	riscv64-elf-as -march=rv32i -mabi=ilp32 -o programs/init/init.o programs/init/init.s

.PHONY: clean
clean:
	rm -rf _build/programs *.bin *.hex


%.report: $(BLD)/%.elf
	@elf="$(BLD)/$*.elf"; \
	echo "=== REPORT for $* ==="; echo; \
	$(SIZE) -A "$$elf"; echo; \
	\
	echo "== FLASH code (.text*) =="; \
	$(OBJDUMP) -t "$$elf" \
	| awk '$$0 ~ /[[:space:]]\.text([[:space:]]|$$)/ { nm=$$NF; if(nm!="" && nm!~ /^\.text$$/) print "  " nm }' \
	| sort -u; \
	echo; \
	\
	echo "== RAM fast code (.fast) =="; \
	$(OBJDUMP) -t "$$elf" \
	| awk '$$0 ~ /[[:space:]]\.fast[[:space:]]/ { nm=$$NF; if(nm!="") print "  " nm }' \
	| sort -u; \
	echo; \
	\
	echo "== RAM data (.data/.bss) =="; \
	$(OBJDUMP) -t "$$elf" \
	| awk '$$0 ~ /[[:space:]]\.(data|bss)[[:space:]]/ { nm=$$NF; if(nm!="" && nm!=".data" && nm!=".bss") print "  " nm }' \
	| sort -u
