---
name: 'mla-c-url-parsing'
description: 'Patterns for parsing, constructing, and manipulating URLs in the MLA framework'
---

# URL Parsing & Formatting Module

The URL module (`core/url/`) provides URL parsing, formatting, component extraction, and query string processing using `mla_url_t`.

## Key Types & Functions

| Function / Type | Header | Description |
|---|---|---|
| `mla_url_t` | `mla_url.h` | Parsed URL component structure (scheme, host, port, path, query, fragment) |
| `mla_url_parse` | `mla_url.h` | Parses an `mla_string_t` into an `mla_url_t` structure |
| `mla_url_to_string` | `mla_url.h` | Converts an `mla_url_t` structure back into a string |
| `mla_url_encode` / `decode` | `mla_url.h` | Percent-encoding and decoding utilities |

## Parsing a URL String

```cpp
#include "core/url/mla_url.h"

mla_string_t input = mla_string_const("https://api.example.com:8443/v1/resource?query=test#section1");
mla_url_t url;

if (mla_url_parse(input, url)) {
    // url.scheme -> "https"
    // url.host   -> "api.example.com"
    // url.port   -> 8443
    // url.path   -> "/v1/resource"
    // url.query  -> "query=test"
}
```

## Constructing a URL String

```cpp
#include "core/url/mla_url.h"

mla_url_t url = mla_url_empty();
url.scheme = mla_string_const("https");
url.host   = mla_string_const("example.org");
url.path   = mla_string_const("/index.html");

mla_string_t url_str = mla_url_to_string(url);
```

## Rules & Best Practices

1. **Immutable Framework Strings**: Use `mla_string_t` for all URL components.
2. **Encapsulation**: Always check `mla_url_parse` return value for parsing errors.
