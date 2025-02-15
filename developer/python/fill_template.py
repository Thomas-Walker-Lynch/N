#+BEGIN_SRC python
#!/usr/bin/env python3

from get_template import get_template
from make_constants import make_constants_block

def fill_template(namespace: str,
                  digit_extent: int,
                  digit_type: str) -> str:
    """
    Renders the final .lib.c code by merging:
      - the base template from get_template()
      - the compile-time constants block from make_constants_block()
      - placeholders for namespace, digit_extent, digit_type, extent_type
    """
    template = get_template()
    constants_block = make_constants_block(namespace, digit_type, digit_extent)

    # Substitute placeholders
    code = template.format(
        NAMESPACE = namespace,
        DIGIT_EXTENT = digit_extent,
        DIGIT_TYPE = digit_type,
        CONSTANTS_BLOCK = constants_block
    )
    return code
#+END_SRC
