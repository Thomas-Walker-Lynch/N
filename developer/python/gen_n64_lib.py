#!/usr/bin/env python3

import sys
from fill_template import fill_template

def main():
    """
    Example: generate a .lib.c with an 'N64·' namespace,
    """
    code = fill_template(
        namespace = "N64·",
        digit_extent = 8,     # => digit_count = 1 => 32-bit
        digit_type = "uint8_t",
    )

    with open("N64.lib.c", "w") as f:
        f.write(code)
    print("Generated N64.lib.c")

if __name__ == "__main__":
    main()
