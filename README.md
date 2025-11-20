# RISC-V (RV32I) for Nandland Go-Board

## Overview
- RV32I specification implemented in Verilog
- All peripherals are memory mapped with read/write when applicable (LEDS, 7SEG1, 7SEG2, Buttons, PMOD)
- Both RAM and SPI flash are readable for program execution
- PMOD OLED screen can be driven from C programs
- C programs can use directives to place functions in RAM or SPI flash


## Installation
1. Install [Apio](https://nandland.com/set-up-apio-fpga-build-and-program/).
2. Add Apio binaries to PATH (`icepack` and `iceprog` required).
3. Install the [riscv-none-embed](https://xpack-dev-tools.github.io/riscv-none-elf-gcc-xpack/docs/install/#manual-installation) toolchain.
4. Add `riscv-none-embed` toolchain binaries to PATH.

## Usage
```
git clone https://github.com/tylertms/riscv
cd riscv
apio build
make reflash
make <program_name>.prog
```

After a change to Verilog (`src/`)
```
apio build
make reflash
```

After a change to C (`programs/`)
```
make <program_name>.prog
```

Generate a report for a program
```
make <program_name>.report
# Generates an overview of code sections and the placement of functions/data (RAM/FLASH)
```

## Datasheets / Resources:
#### RISC-V Implementation:
- https://five-embeddev.com/riscv-user-isa-manual/Priv-v1.12/rv32.html
- https://github.com/BrunoLevy/learn-fpga/tree/master/FemtoRV/TUTORIALS/FROM_BLINKER_TO_RISCV/README.md
- https://github.com/BrunoLevy/learn-fpga/blob/master/FemtoRV/RTL/PROCESSOR/femtorv32_quark_bicycle.v

#### SSD1331 PMOD OLEDRGB:
- https://digilent.com/reference/_media/pmod:pmod:pmodoledrgb_rm.pdf
- https://cdn-shop.adafruit.com/datasheets/SSD1331_1.2.pdf

#### M25P10 1Mb Flash:
- https://www.digikey.com/htmldatasheets/production/1338845/0/0/1/M25P10-A.pdf

#### Nandland Go-Board Schematic:
- https://nandland.com/goboard/images/Go_Board_V1.pdf
