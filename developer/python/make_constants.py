def make_constants_block(namespace: str,
                         digit_type: str,
                         digit_extent: int) -> str:
    """
    Returns a block of code defining static compile-time constants:
      static {namespace}T {namespace}constant[4] = {
        { { ...zero... } },
        { { ...one... } },
        { { ...allbits... } },
        { { ...msb... } }
      };

    The total digit count is digit_extent + 1.
    """
    digit_count = digit_extent + 1

    def digits_zero():
        return ", ".join("0" for _ in range(digit_count))

    def digits_one():
        # index 0 => 1, rest => 0
        return ", ".join("1" if i == 0 else "0" for i in range(digit_count))

    def digits_allbits():
        # each digit => (digit_type)(-1)
        return ", ".join(f"( {digit_type} )( -1 )" for _ in range(digit_count))

    def digits_msb():
        # top index => (digit_type)1 << ((sizeof(digit_type)*8)-1)
        items = []
        for i in range(digit_count):
            if i == digit_count - 1:
                items.append(f"( {digit_type} )1 << ((sizeof({digit_type})*8) - 1)")
            else:
                items.append("0")
        return ", ".join(items)

    return f'''\
static {namespace}T {namespace}constant[4] = {{
  {{
    // zero
    {{ {digits_zero()} }}
  }},
  {{
    // one
    {{ {digits_one()} }}
  }},
  {{
    // all one bits
    {{ {digits_allbits()} }}
  }},
  {{
    // msb
    {{ {digits_msb()} }}
  }}
}};'''
