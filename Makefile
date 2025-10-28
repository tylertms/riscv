TC      := riscv-none-elf
CC      := $(TC)-gcc
OBJCOPY := $(TC)-objcopy
OBJDUMP := $(TC)-objdump
SIZE    := $(TC)-size

SRC_DIR := programs
BLD     := _build/programs

ARCH    := rv32i
ABI     := ilp32
CFLAGS  := -march=$(ARCH) -mabi=$(ABI) -ffreestanding -fno-pic -O3 \
           -ffunction-sections -fdata-sections -I$(SRC_DIR)/include
LDFLAGS := -march=$(ARCH) -mabi=$(ABI) -T default.ld -nostartfiles -nostdlib \
           -Wl,--gc-sections

DEVICE  := 0x0403:0x6010

.PHONY: all clean reflash

all:
	@echo "Usage: make <name>.prog"

$(BLD):
	@mkdir -p $@

$(BLD)/init.o: $(SRC_DIR)/init/init.s | $(BLD)
	$(CC) -march=$(ARCH) -mabi=$(ABI) -c $< -o $@

$(BLD)/%.o: $(SRC_DIR)/%.c | $(BLD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BLD)/%.elf: $(BLD)/%.o $(BLD)/init.o
	$(CC) $(LDFLAGS) $(BLD)/init.o $< -lgcc -o $@

$(BLD)/%.bin: $(BLD)/%.elf
	$(OBJCOPY) -O binary $< $@

%.prog: $(BLD)/%.bin
	iceprog -d i:$(DEVICE) -o 64k $<

%.report: $(BLD)/%.elf
	@echo "=== REPORT for $* ==="; echo; \
	$(SIZE) -A $<; echo; \
	echo "== FLASH (.text) =="; \
	$(OBJDUMP) -t $< | awk '/\.text[[:space:]]/ && !/^\.text$$/ {print "  "$$NF}' | sort -u; echo; \
	echo "== RAM (.fast) =="; \
	$(OBJDUMP) -t $< | awk '/\.fast[[:space:]]/ {print "  "$$NF}' | sort -u; echo; \
	echo "== DATA (.data/.bss) =="; \
	$(OBJDUMP) -t $< | awk '/\.(data|bss)[[:space:]]/ && !/^\.(data|bss)$$/ {print "  "$$NF}' | sort -u

reflash:
	icepack -s _build/default/hardware.asc _build/default/hardware.bin
	iceprog -d i:$(DEVICE) _build/default/hardware.bin

clean:
	rm -rf $(BLD)
