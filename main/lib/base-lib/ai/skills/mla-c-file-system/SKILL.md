---
name: 'mla-c-file-system'
description: 'Patterns for using the virtual file system in the MLA framework'
---

# File System

The file system module (`core/filesystem/mla_file_system.h`) provides a virtual, mountable file system abstraction. Multiple file system implementations (OS native, in-memory, etc.) can be **mounted** at different paths and accessed through a unified API.

## Key Types

| Type | Purpose |
|---|---|
| `mla_file_system_t` | A file system implementation (callback struct) |
| `mla_file_system_stream_t` | An open file handle with seek, read, write |
| `mla_file_system_file_open_mode` | Enum: `READ`, `WRITE`, `READ_AND_WRITE` |

## File Open Modes

| Constant | Purpose |
|---|---|
| `MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ` | Open for reading only |
| `MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE` | Open for writing only (creates if needed) |
| `MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ_AND_WRITE` | Open for both reading and writing |

## Mounting a File System

Register a file system implementation at a mount path. All file operations whose paths start with the mount path are routed to that implementation:

```cpp
#include "../filesystem/mla_file_system.h"

// Mount the platform file system at "/"
mla_file_system_t platformFs = get_platform_file_system();
mla_file_system_initialize(mla_string_const("/"), platformFs);

// Mount an in-memory FS at "/tmp/"
#include "../filesystem/mla_file_system_inmemory.h"
mla_file_system_t memFs = mla_file_system_inmemory_create(4096);
mla_file_system_initialize(mla_string_const("/tmp/"), memFs);
```

Longer (more specific) mount paths are matched first. For example, `/tmp/data.txt` matches the `/tmp/` mount before the `/` mount.

### Unmounting

```cpp
mla_file_system_deinitialize(mla_string_const("/tmp/"));
```

## File Operations

All file operations use the `mla_fs_*` convenience functions, which automatically resolve the correct mount:

### Check Existence

```cpp
if (mla_fs_file_exists(mla_string_const("/config/settings.json"))) {
    // file exists
}
```

### Open a File

```cpp
mla_file_system_stream_t stream;
if (mla_fs_open_file(mla_string_const("/data/log.txt"),
                      MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, stream)) {
    // stream is now ready for reading
    mla_byte_t buf[256];
    mla_size_t bytesRead = stream.read(stream, 0, sizeof(buf), buf);

    // Get file info
    mla_size_t fileLen = stream.length(stream);
    mla_size_t pos     = stream.position(stream);
    stream.seek(stream, 0); // seek to beginning
}
```

### Delete a File

```cpp
mla_fs_delete_file(mla_string_const("/data/old.txt"));
```

### List Files in a Directory

```cpp
mla_array_list_t<mla_init_struct(mla_string_t)> files =
    mla_array_list<mla_init_struct(mla_string_t)>();
if (mla_fs_list_files(mla_string_const("/data/"), files)) {
    for (mla_size_t i = 0; i < mla_array_list_size(files); i++) {
        mla_info(mla_array_list_get_unsafe(files, i));
    }
}
```

## Directory Operations

```cpp
// Create a directory
mla_fs_create_directory(mla_string_const("/data/logs/"));

// Check if a directory exists
if (mla_fs_directory_exists(mla_string_const("/data/logs/"))) {
    // …
}

// List subdirectories
mla_array_list_t<mla_init_struct(mla_string_t)> dirs =
    mla_array_list<mla_init_struct(mla_string_t)>();
mla_fs_list_directory(mla_string_const("/data/"), dirs);

// Delete a directory
mla_fs_delete_directory(mla_string_const("/data/old/"));
```

## Path Utilities

```cpp
// Check if path represents a directory (ends with "/")
mla_bool_t isDir = mla_fs_is_directory_path(mla_string_const("/data/logs/"));

// Get parent directory
mla_string_t parent = mla_fs_get_parent_directory(mla_string_const("/data/logs/file.txt"));
// → "/data/logs/"

// Get filename
mla_string_t name = mla_fs_get_file_name(mla_string_const("/data/logs/file.txt"));
// → "file.txt"

// Get file extension
mla_string_t ext = mla_fs_get_file_extension(mla_string_const("/data/logs/file.txt"));
// → "txt"

// Change file extension
mla_string_t newPath = mla_fs_change_file_extension(
    mla_string_const("/data/file.json"),
    mla_string_const("xml"));
// → "/data/file.xml"

// Combine paths
mla_string_t full = mla_fs_combine_paths(
    mla_string_const("/data/"),
    mla_string_const("file.txt"));
// → "/data/file.txt"
```

## File System Stream

The `mla_file_system_stream_t` struct provides a rich file handle:

```cpp
struct mla_file_system_stream_t {
    mla_string_t path;

    mla_bool_t (*seek)(const mla_file_system_stream_t& stream, mla_size_t offset);
    mla_size_t (*position)(const mla_file_system_stream_t& stream);
    mla_size_t (*length)(const mla_file_system_stream_t& stream);
    mla_bool_t (*set_length)(const mla_file_system_stream_t& stream, mla_size_t length);

    mla_size_t (*read)(const mla_file_system_stream_t& input,
                       mla_size_t offset, mla_size_t length, mla_byte_t* buffer);
    mla_size_t (*write)(const mla_file_system_stream_t& output,
                        mla_size_t offset, mla_size_t length, const mla_byte_t* buffer);

    mla_dynamic_data_t resource;
    mla_buffer_reference_t resourceOwner;
};
```

`read` and `write` may be `nullptr` depending on the open mode.

## In-Memory File System

Useful for testing or for embedded platforms without persistent storage:

```cpp
#include "../filesystem/mla_file_system_inmemory.h"

// Create with 8 KB capacity
mla_file_system_t memFs = mla_file_system_inmemory_create(8192);
mla_file_system_initialize(mla_string_const("/mem/"), memFs);

// Or create from an existing buffer
mla_byte_t buffer[4096];
mla_file_system_t bufFs = mla_file_system_inmemory_create_from_buffer(buffer, sizeof(buffer));
```

## Implementing a Custom File System

Create a `mla_file_system_t` with function pointers for each operation:

```cpp
mla_file_system_t my_custom_fs;
my_custom_fs.file_exists      = my_file_exists;
my_custom_fs.open_file        = my_open_file;
my_custom_fs.delete_file      = my_delete_file;
my_custom_fs.list_files       = my_list_files;
my_custom_fs.create_directory = my_create_directory;
my_custom_fs.directory_exists = my_directory_exists;
my_custom_fs.delete_directory = my_delete_directory;
my_custom_fs.list_directory   = my_list_directory;

mla_file_system_initialize(mla_string_const("/custom/"), my_custom_fs);
```

## Rules

- **Never** use `fopen`, `fread`, `fwrite`, or any standard C/POSIX file I/O directly — always use the `mla_fs_*` functions.
- Mount file systems during boot events before any file operations are attempted.
- The mount path must end with `/` for directory mounts.
- Always check the return value of `mla_fs_open_file` before using the stream.
- File system operations are **not thread-safe** by default — use `mla_file_system_lock()` / `mla_file_system_unlock()` if concurrent access is needed.

## Incorrect Usage

```cpp
// ❌ Using standard C file I/O
FILE* f = fopen("/data/file.txt", "r"); // use mla_fs_open_file

// ❌ Using the stream without checking open success
mla_file_system_stream_t stream;
// In test code, you must always verify that mla_fs_open_file returns true:
assert_true(mla_fs_open_file(path, MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, stream), "Failed to open file");
stream.read(stream, 0, 100, buf); // crash if open failed

// ❌ Writing to a read-only stream
mla_fs_open_file(path, MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, stream);
stream.write(stream, 0, 10, data); // write is nullptr — crash

// ❌ Not mounting before accessing
mla_fs_file_exists(mla_string_const("/unmounted/file.txt")); // no mount — always false
```

