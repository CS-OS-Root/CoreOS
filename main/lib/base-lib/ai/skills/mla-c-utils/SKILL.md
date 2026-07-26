---
name: 'mla-c-utils'
description: 'Patterns for character manipulation and endianness conversion utilities in the MLA framework'
---

# Utilities Module

The Utilities module (`core/utils/`) provides helper functions for character classification/case conversion, byte swap / endianness conversions, and general helper routines.

## Character Utilities (`mla_char_utils.h`)

Provides functions for character transformations and whitespace/digit checks without `<ctype.h>` dependencies:

```cpp
#include "core/utils/mla_char_utils.h"

mla_char_t lower = mla_char_toLower('G');           // 'g'
mla_char_t upper = mla_char_toUpper('m');           // 'M'
mla_bool_t is_space = mla_char_is_whitespace(' ');  // true
mla_bool_t is_digit = mla_char_is_digit('7');       // true
```

## Endian Utilities (`mla_endian_utils.h`)

Provides endian conversion functions between host, big-endian, and little-endian representation:

```cpp
#include "core/utils/mla_endian_utils.h"

mla_uint16_t val16 = 0x1234;
mla_uint16_t swapped16 = mla_endian_swap16(val16); // 0x3412

mla_uint32_t val32 = 0x12345678;
mla_uint32_t swapped32 = mla_endian_swap32(val32); // 0x78563412
```

## Rules & Best Practices

1. **No Standard Headers**: Do not include `<ctype.h>` or `<byteswap.h>`. Use framework functions.
2. **Framework Types**: Always use `mla_char_t`, `mla_uint16_t`, `mla_uint32_t`, etc.
