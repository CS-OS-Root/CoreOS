---
name: 'mla-c-serverside-rendering'
description: 'Patterns for server-side rendering (SSR), compiling HTML templates into C++ code with mla-build-app webapp module, and downloading mla-build-app from the release server'
---

# Server-Side Rendering (SSR) & WebApp Module

The MLA-C framework provides a compile-time Server-Side Rendering (SSR) engine and web application packaging system. Using the `mla-build-app webapp` module, HTML templates (`*.mla-template.html`) and static assets (CSS, JS, images) are compiled into standalone, self-contained C++ header files (`*.web.h`).

This approach achieves:
- **Zero Runtime Dependencies**: No runtime template engines, disk I/O, or external static file hosting needed in production.
- **Embedded Gzip Compression**: Static assets are pre-compressed with gzip at compile time and served with native HTTP headers.
- **Dynamic Chunked Streaming**: Dynamic pages stream generated HTML blocks and evaluated C/C++ expressions directly to clients via HTTP chunked transfer encoding (`mla_http_chunked_stream_output_t`).

---

## Architecture Overview

```
 [ Module Configuration ]
   └── lib/<module>/webapp.json   ──> Explicit list of web application paths to compile

 [ Source Files ]
   ├── <app_name>.web.header.h   ──> Input struct & template helper functions
   ├── *.mla-template.html       ──> HTML with embedded <% %> & <%= %> tags
   └── Static Assets (.css, ...) ──> Embedded & gzipped assets

               │
               ▼ (mla-build-app webapp build)
 [ Compilation Pipeline ]
   ├── mla_ui_html_template_compile ──> Tokenize HTML vs C++ code blocks
   ├── Byte Array Generator         ──> Convert HTML chunks to hex byte arrays
   ├── Static Deflater              ──> Gzip-compress static assets
   └── Route & Handler Generator    ──> Generate mla_http_server_handler_item_t

               │
               ▼
 [ Generated Output Header ]
   └── <app_name>.web.h          ──> Single C++ header to include & register
```

---

## Module Configuration: `webapp.json`

Every library module containing web applications declares a `webapp.json` file directly inside its module root under `lib/<module>/` (e.g. `lib/build/webapp.json`).

The `webapp.json` file is the **single source of truth** for apps to compile or watch. It contains an `"apps"` array listing relative paths to the web app folders:

```json
{
  "apps": [
    "workspace/webapp/content_app",
    "workspace/webapp/dashboard_app"
  ]
}
```

> **Note:** The app name does not need to be declared in `webapp.json`; it is always derived directly from the `<app_name>.web.header.h` file name inside each app directory.

---

## Directory & File Conventions

Each web application component is organized inside its own directory. The compiler recognizes files using standard suffixes:

| File Pattern | Purpose | Description |
|---|---|---|
| `<app_name>.web.header.h` | **App Root Header** | Defines `mla_web_app_<app_name>_input_t` struct, `::init()` static method, and helper functions accessible within templates. |
| `*.mla-template.html` | **HTML Template** | HTML markup containing embedded C/C++ code blocks (`<% ... %>`) and expression outputs (`<%= ... %>`). |
| `*.<ext>` (e.g. `.css`, `.js`, `.png`) | **Static Asset** | Any file in the app directory not matching `.web.header.h` or `.web.h` is treated as a static asset. |
| `<app_name>.web.h` | **Generated Output** | Generated C++ header containing compiled handlers and registration routines. **Do NOT edit manually.** |

---

## Template Syntax

HTML templates (`*.mla-template.html`) support embedded C/C++ logic executed directly within the server process during HTTP response rendering.

### 1. Code Blocks: `<% ... %>`
Embed arbitrary C/C++ control flow, such as loops, conditional statements, or local variables:

```html
<%
    mla_array_list_t<mla_init_struct(mla_string_t)> items = input.items;
    for (mla_size_t i = 0; i < mla_array_list_size(items); ++i) {
        mla_string_t& item = mla_array_list_get_unsafe(items, i);
%>
    <div class="list-item">
        <%= item %>
    </div>
<%
    }
%>
```

### 2. Expression Output Blocks: `<%= ... %>`
Evaluates any C/C++ expression returning a string or string expression and writes it directly to the response output stream:

```html
<h1>Welcome, <%= input.username %>!</h1>
<a href="<%= mla_string_concat("/profile/", input.user_id) %>">Profile</a>
```

### 3. The `input` Parameter
Every template has access to a typed parameter named `input`, which is an instance of the struct `mla_web_app_<app_name>_input_t` declared in `<app_name>.web.header.h`.

---

## Compilation Mechanics

The `mla-build-app` compilation engine (`lib/build/workspace/webapp/mla_build_web_app.cpp`) processes templates through several stages:

1. **Delimiter Parsing**: `mla_ui_html_template_compile` parses `<%` and `%>` delimiters to segment the template into HTML blocks and C++ code blocks.
2. **HTML Byte Array Emission**: Static HTML segments are converted into raw hexadecimal byte arrays:
   ```cpp
   static const mla_byte_t html_block[] = {0x3C, 0x68, 0x31, 0x3E, ...};
   out.write(out, 0, sizeof(html_block), html_block);
   ```
3. **Expression Streaming**: `<%= expr %>` statements are transformed into stream write calls:
   ```cpp
   mla_stream_output_write_string(out, expr);
   ```
4. **Chunked Response Writer**: The generated content writer wraps the response output stream in `mla_http_chunked_stream_output_t`, automatically streaming response chunks and finalizing with `mla_http_chunked_stream_output_finished(out_chunked)`.
5. **Static File Gzipping**: Static assets are compressed with `mla_stream_output_deflate_compress_wrapper(..., mla_deflate_mode_gzip)` and emitted as static byte arrays served with `Content-Encoding: gzip`.
6. **Registration Function**: Generates `mla_web_app_<app_name>_register(server, userdata, app_data)` to bind all routes under `/<app_name>/...` to the HTTP server.

---

## CLI Module: `mla-build-app webapp`

The build tool provides the `webapp` CLI module for creating, building, and live-developing web applications.

### 1. Create a New Web Application (`webapp create`)
Scaffolds a new web application and automatically registers it in the nearest module's `webapp.json`:

```bash
# Create a new web app scaffold and register it in webapp.json
mla-build-app "webapp && create --name dashboard --path lib/my-module/dashboard_app"
```

Parameters:
- `--name` (**Required**): Identifier of the web application (e.g. `dashboard`).
- `--path` (**Required**): Directory where `<name>.web.header.h` and `index.mla-template.html` will be scaffolded.
- `--verbose` (Optional): Enable verbose status output.

### 2. Build Web Applications (`webapp build`)
Compiles templates and static files into `.web.h` headers:

```bash
# Zero-config root execution: builds all apps configured in lib/*/webapp.json
mla-build-app webapp build

# Build a specific directory explicitly
mla-build-app webapp build --path lib/my-module/dashboard_app --verbose
```

### 3. Live Development & Watch Mode (`webapp watch`)
Starts a live HTTP server on `http://127.0.0.1:5080` that serves dynamic template changes instantly without restarting:

```bash
# Start watch mode on all apps declared in lib/*/webapp.json
mla-build-app webapp watch

# Watch a specific path with verbose logging
mla-build-app webapp watch --path lib/my-module/dashboard_app --verbose
```

In watch mode:
- Template requests recompile on the fly.
- Responses are served with `Cache-Control: no-cache` headers for immediate feedback in the browser.
- Background worker automatically recompiles `.web.h` header files on disk every second when changes are detected.

---

## Step-by-Step Implementation Example

### Step 1: Scaffold the App with `webapp create`

```bash
./build/clang/mla-build-app "webapp && create --name dashboard --path lib/my-module/dashboard_app"
```

This generates `lib/my-module/dashboard_app/dashboard.web.header.h`, `index.mla-template.html`, and registers the app in `lib/my-module/webapp.json`.

### Step 2: Customize the App Header (`dashboard.web.header.h`)

```cpp
#ifndef DASHBOARD_WEB_HEADER_H
#define DASHBOARD_WEB_HEADER_H

#include "../lib/base-lib/core/system/mla_string.h"
#include "../lib/base-lib/core/containers/mla_array_list.h"

// Define the input model struct passed to templates
struct mla_web_app_dashboard_input_t {
    mla_string_t title;
    mla_array_list_t<mla_init_struct(mla_string_t)> services;

    static mla_web_app_dashboard_input_t init() {
        return {
            mla_string_empty(),
            mla_array_list_empty<mla_init_struct(mla_string_t)>()
        };
    }
};

#endif // DASHBOARD_WEB_HEADER_H
```

### Step 3: Customize the Template (`index.mla-template.html`)

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title><%= input.title %></title>
    <link rel="stylesheet" href="/dashboard/style.css">
</head>
<body>
    <h1><%= input.title %></h1>
    <ul>
        <%
            for (mla_size_t i = 0; i < mla_array_list_size(input.services); ++i) {
                mla_string_t& svc = mla_array_list_get_unsafe(input.services, i);
        %>
            <li>Service: <strong><%= svc %></strong></li>
        <%
            }
        %>
    </ul>
</body>
</html>
```

### Step 4: Compile the Web App

Run the build command:
```bash
./build/clang/mla-build-app webapp build
```

This produces `dashboard.web.h` in `lib/my-module/dashboard_app/`.

### Step 5: Register & Serve with `mla_http_server_t`

```cpp
#include "dashboard.web.h"
#include "core/http/mla_http_server.h"

void start_web_server() {
    mla_network_host_t host = mla_network_host_ip4(mla_string_const("0.0.0.0"), 8080);
    mla_http_server_t server = mla_http_server(host);

    // Populate the template input data
    mla_web_app_dashboard_input_t data = mla_web_app_dashboard_input_t::init();
    data.title = mla_string_const("System Dashboard");
    mla_array_list_add(data.services, mla_string_const("Database Service"));
    mla_array_list_add(data.services, mla_string_const("Auth Service"));

    // Register all routes for /dashboard/ to the server
    mla_user_data_t userdata = mla_user_data_empty();
    mla_web_app_dashboard_register(server, userdata, data);

    // Start server with 4 worker threads
    mla_http_server_start(server, 4);
}
```

---

## Downloading `mla-build-app` from the Release Server

Precompiled release binaries of `mla-build-app` are hosted on the official release server:

- **Release Server Base URL**: `https://releases.home.schlegel.ovh`
- **Product Name**: `mla-build`
- **Release Path Layout**:
  ```
  https://releases.home.schlegel.ovh/mla-build/${VERSION}/${PLATFORM}/${COMPILER}/mla-build-app
  ```

### Platform & Compiler Identifiers

| Parameter | Supported Values |
|---|---|
| `${PLATFORM}` | `linux_x86_64`, `linux_aarch64`, `windows64`, `darwin_x86_64`, `darwin_arm64` |
| `${COMPILER}` | `clang`, `gcc`, `zig_native`, `filc` |

---

### Method A: Built-in Self-Update via CLI

If you already have `mla-build-app`, use the built-in `update` module:

```bash
# 1. Check current version
./mla-build-app update version

# 2. Check if a newer version is available on the release server
./mla-build-app update check

# 3. Upgrade to the latest version automatically
./mla-build-app update upgrade

# 4. Upgrade (or downgrade) to a specific version
./mla-build-app update upgrade --version 0.0.2
```

The application downloads the updated binary, initiates self-replacement with `--mla-apply-update`, and restarts automatically.

---

### Method B: Downloading via `curl` / Shell Script

To download `mla-build-app` directly in CI/CD pipelines, Dockerfiles, or fresh developer environments:

```bash
#!/usr/bin/env bash
set -e

RELEASES_SERVER="https://releases.home.schlegel.ovh"
PRODUCT="mla-build"
COMPILER="clang"

# 1. Detect OS architecture
ARCH="$(uname -m)"
OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
case "$OS" in
    linux*)   PLATFORM="linux_${ARCH}" ;;
    darwin*)  PLATFORM="darwin_${ARCH}" ;;
    msys*|mingw*|cygwin*) PLATFORM="windows64" ;;
    *) PLATFORM="${OS}_${ARCH}" ;;
esac

# 2. Query the latest version string from the server
VERSION=$(curl -sL "${RELEASES_SERVER}/${PRODUCT}/version" | tr -d '\r\n')
if [ -z "$VERSION" ]; then
    echo "Failed to fetch version from ${RELEASES_SERVER}/${PRODUCT}/version"
    exit 1
fi

echo "Latest ${PRODUCT} version: ${VERSION}"

# 3. Form download URL and fetch binary
BINARY_NAME="mla-build-app"
if [ "$PLATFORM" = "windows64" ]; then
    BINARY_NAME="mla-build-app.exe"
fi

DOWNLOAD_URL="${RELEASES_SERVER}/${PRODUCT}/${VERSION}/${PLATFORM}/${COMPILER}/${BINARY_NAME}"
echo "Downloading from: ${DOWNLOAD_URL}"

curl -sSL "$DOWNLOAD_URL" -o "$BINARY_NAME"
chmod +x "$BINARY_NAME"

# 4. Verify installation
./"${BINARY_NAME}" update version
```

---

## Best Practices & Guidelines

1. **Explicit Target Declaration**: Declare all web application directories in `lib/<module>/webapp.json` so zero-config builds and watch commands operate on exact target sets.
2. **Scaffold with `webapp create`**: Use `mla-build-app "webapp && create --name <name> --path <path>"` to quickly create template scaffolding and auto-update `webapp.json`.
3. **Never Modify `.web.h` Directly**: All changes must be made in `<app_name>.web.header.h`, `*.mla-template.html`, or static assets, and regenerated via `webapp build`.
4. **Keep Business Logic in Helpers**: Template files should focus on UI layout and formatting. Place complex queries or calculations in inline helper functions within `<app_name>.web.header.h`.
5. **Use Framework Memory Conventions**: When allocating strings or lists in templates, ensure proper MLA-C reference counting rules are followed.
6. **Use Chunked Encoding for Large Dynamic Content**: HTML templates automatically stream via chunked encoding, avoiding buffer size limitations for large rendered pages.
7. **Static Assets Organization**: Place relative assets (e.g. `style.css`, `logo.png`) inside the app directory alongside `index.mla-template.html` so they are bundled and gzipped automatically under `/<app_name>/<asset_name>`.
