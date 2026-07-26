---
name: 'mla-c-hash-algorithms'
description: 'Patterns for using cryptographic and checksum hashing algorithms in the MLA framework'
---

# Hash Algorithms Module

The Hash module (`core/hash/`) provides implementations of hashing algorithms, including SHA-256, MD5, CRC32, and FNV1a.

## Hashing Algorithms Overview

| Header | Description | Typical Use Case |
|---|---|---|
| `mla_sha256.h` | SHA-256 cryptographic hash function | Secure digest, signatures, integrity verification |
| `mla_md5.h` | MD5 cryptographic hash function | Legacy digest, quick checksums |
| `mla_crc32.h` | Cyclic Redundancy Check 32-bit | Data transfer / network packet integrity |
| `mla_fnv1a.h` | Fowler-Noll-Vo 1a fast non-cryptographic hash | In-memory lookup tables, string hashing |

## Usage Examples

### SHA-256 Hash Computation

```cpp
#include "core/hash/mla_sha256.h"

mla_string_t input = mla_string_const("Hello MLA Framework");
mla_uint8_t digest[32];

mla_sha256_compute((const mla_uint8_t*)input.buffer, input.length, digest);
```

### FNV1a Fast Hash Computation

```cpp
#include "core/hash/mla_fnv1a.h"

mla_string_t str = mla_string_const("hash_key");
mla_uint32_t hash_val = mla_fnv1a_32_compute((const mla_uint8_t*)str.buffer, str.length);
```

## Rules & Best Practices

1. **No Standard C Libs**: Do not include `<openssl/sha.h>` or `<md5.h>`. Use the framework headers.
2. **Buffer Access**: Always inspect length boundaries before computing hashes over raw buffer bytes.
