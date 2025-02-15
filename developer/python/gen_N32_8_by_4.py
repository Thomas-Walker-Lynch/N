#!/usr/bin/env python3

import sys
from fill_template import fill_template

def main():
    """
    Example: generate an N<type_name>.lib.c 
    """

    type_name = "N32";

    # 4 x 8 bit 
    code = fill_template(
        namespace = type_name + "·",
        digit_extent = 3,     
        digit_type = "uint8_t",
    )


    filename = "../cc/" + type_name + ".lib.c" 
    with open(filename, "w") as f:
        f.write(code)
    print("Generated " + filename)

if __name__ == "__main__":
    main()
