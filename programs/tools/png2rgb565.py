import sys

from PIL import Image


def convert_to_header(input_file, output_file, var_name):
    try:
        img = Image.open(input_file)
        img = img.resize((96, 64), Image.Resampling.LANCZOS)
        img = img.convert("RGB")

        pixels = list(img.getdata())

        with open(output_file, "w") as f:
            f.write(f"#pragma once\n\n")
            f.write("#include <stdint.h>\n")
            f.write('#include "go-board.h"\n\n')

            f.write(f"_rodata const uint16_t {var_name}[6144] = {{\n")

            for i, pixel in enumerate(pixels):
                if i % 16 == 0:
                    f.write("\t")

                r, g, b = pixel
                rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

                f.write(f"0x{rgb565:04X}, ")

                if (i + 1) % 16 == 0:
                    f.write("\n")

            f.write("};\n")

        print(f"Done! Saved 96x64 RGB565 array to {output_file}")

    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python png2rgb565.py <input.png> <output.h> <c_variable_name>")
    else:
        convert_to_header(sys.argv[1], sys.argv[2], sys.argv[3])
