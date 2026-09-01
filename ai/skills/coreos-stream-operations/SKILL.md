---
name: coreos-stream-operations
description: Stream input/output patterns for the MLA framework. Use when reading from or writing to buffers, files, strings, memory streams, or when composing stream wrappers for compression, buffering, and interception.
metadata:
  author: coreos
  version: "1.0"
---

# Stream Operations

The stream module (`core-os/system/mla_stream.h`) provides a callback-based I/O abstraction. Streams are composable via wrappers and support buffering, compression, limiting, and interception.

## Core Stream Types

### Input Stream

```cpp
struct mla_stream_input_t {
    mla_user_data_t userdata;
    mla_size_t (*read)(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer);
    mla_size_t (*remaining_bytes)(mla_stream_input_t& input);
};
```

### Output Stream

```cpp
struct mla_stream_output_t {
    mla_user_data_t userdata;
    mla_size_t (*write)(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer);
    mla_size_t (*available_bytes)(mla_stream_output_t& output);
};
```

### Memory Stream (Input + Output)

```cpp
struct mla_memory_stream_t {
    mla_stream_input_t input;
    mla_stream_output_t output;
};
```

## Creating Streams

### From Buffers

```cpp
// Non-owning — wraps an external buffer
mla_byte_t buffer[1024];
mla_stream_input_t input = mla_stream_input_from_buffer(buffer, 1024);
mla_stream_output_t output = mla_stream_output_to_buffer(buffer, 1024);

// Owning — creates and manages its own buffer
mla_stream_input_t ownedInput = mla_stream_input_from_buffer(4096);
mla_stream_output_t ownedOutput = mla_stream_output_to_buffer(4096);
```

### From Strings

```cpp
mla_string_t str = mla_string_const("{\"key\":\"value\"}");
mla_stream_input_t input = mla_stream_input_from_string(str);
```

### Console I/O

```cpp
mla_stream_input_t stdin_stream = mla_stream_input_stdin();
mla_stream_output_t stdout_stream = mla_stream_output_stdout();
```

### In-Memory Stream

```cpp
// Growable memory stream
mla_memory_stream_t memStream = mla_memory_stream(4096);

// Fixed-size memory stream
mla_memory_stream_t fixed = mla_memory_stream(1024, false);

// Empty / invalid
mla_memory_stream_t empty = mla_memory_stream_empty();
mla_memory_stream_t invalid = mla_memory_stream_invalid();
```

### No-Op Streams

```cpp
mla_stream_input_t noInput = mla_stream_noop_input();
mla_stream_output_t noOutput = mla_stream_noop_output();
```

## Reading and Writing

```cpp
// Reading from an input stream
mla_byte_t data[256];
mla_size_t bytesRead = input.read(input, 0, 256, data);

// Writing to an output stream
const mla_byte_t* content = (const mla_byte_t*)"Hello";
mla_size_t bytesWritten = output.write(output, 0, 5, content);

// Check remaining/available bytes
mla_size_t remaining = input.remaining_bytes(input);
mla_size_t available = output.available_bytes(output);
```

## Memory Stream Operations

```cpp
mla_memory_stream_t ms = mla_memory_stream(4096);

// Get current size (bytes written)
mla_size_t size = mla_memory_stream_get_size(ms);

// Get current read/write position
mla_size_t pos = mla_memory_stream_get_position(ms);

// Set position for reading
mla_memory_stream_set_position(ms, 0);

// Reset to empty state
mla_memory_stream_reset(ms);
```

## Stream Wrappers

Wrappers compose streams to add capabilities. They take an inner stream and return a new stream with the added functionality.

### Buffered Wrapper

Adds read/write buffering for better performance on small I/O operations:

```cpp
// Buffered reading (256-byte buffer)
mla_stream_input_t buffered = mla_stream_input_buffered_wrapper(rawInput, 256);

// Buffered writing (4096-byte buffer)
mla_stream_output_t bufferedOut = mla_stream_output_buffered_wrapper(rawOutput, 4096);

// Flush pending writes
mla_stream_output_flush_buffered_wrapper(bufferedOut);
```

### Limited Wrapper

Restricts the amount of data that can be read:

```cpp
// Read at most 1024 bytes from the inner stream
mla_stream_input_t limited = mla_stream_input_limited_wrapper(input, 1024);
```

### Timeout Wrapper

Adds a timeout to read operations:

```cpp
// Read with a 5000ms timeout
mla_stream_input_t timedInput = mla_stream_input_timeout_wrapper(input, 5000);

// Or use the direct function
mla_byte_t buf[256];
mla_size_t read = mla_stream_input_read_with_timeout(input, 0, 256, buf, 5000);
```

### Compression Wrappers (DEFLATE)

```cpp
// Compress data as it is written
mla_stream_output_t compressed = mla_stream_output_deflate_compress_wrapper(rawOutput);
// ... write data ...
mla_stream_output_deflate_finish(compressed);  // finalize compression

// Decompress data as it is read
mla_stream_input_t decompressed = mla_stream_input_deflate_decompress_wrapper(rawInput);
// ... read decompressed data ...
```

### Interceptor Wrappers

Intercept read/write operations for monitoring or modification:

```cpp
// Input interceptor
static mla_size_t my_read_interceptor(
    mla_stream_input_t& p_Wrapper,
    mla_stream_input_t& p_Input,
    mla_size_t p_Offset,
    mla_size_t p_Length,
    const mla_byte_t* p_Buffer)
{
    // Monitor or modify the read
    mla_debug("Read intercepted");
    return p_Length;  // pass through
}

mla_stream_input_t intercepted = mla_stream_input_interceptor_wrapper(
    input, my_read_interceptor, nullptr
);
```

## Composing Wrappers

Wrappers can be stacked for complex pipelines:

```cpp
mla_stream_input_t rawInput = mla_stream_input_from_buffer(buffer, bufferSize);

// Stack: raw → decompression → buffering → limit
mla_stream_input_t decompressed = mla_stream_input_deflate_decompress_wrapper(rawInput);
mla_stream_input_t buffered = mla_stream_input_buffered_wrapper(decompressed, 4096);
mla_stream_input_t limited = mla_stream_input_limited_wrapper(buffered, maxSize);

// Read from 'limited' — data flows through the entire pipeline
```

## Helper Functions

```cpp
// Copy all data from input to output
mla_bool_t success = mla_stream_copy(input, output);

// Write a string directly to an output stream
mla_stream_output_write_string(output, mla_string_const("Hello World"));

// Calculate output size without writing
mla_stream_output_t sizeCalc = mla_stream_output_size_calculation();
// ... write to sizeCalc ...
mla_size_t totalSize = mla_stream_output_size_calculation_get_size(sizeCalc);
```

## Rules

- Streams are callback-based — always use the function pointers, not direct buffer access.
- The `offset` parameter in read/write tracks the cumulative position across calls.
- `remaining_bytes()` may return `mla_size_max` if the total size is unknown (e.g., network streams).
- Always call `mla_stream_output_deflate_finish()` after compression to flush internal buffers.
- Always call `mla_stream_output_flush_buffered_wrapper()` before reading back buffered output.
- Memory streams are growable by default; pass `false` to `mla_memory_stream()` for fixed-size.

## Incorrect Usage

```cpp
// ❌ Forgetting to finish deflate compression
mla_stream_output_t compressed = mla_stream_output_deflate_compress_wrapper(output);
// write data...
// missing: mla_stream_output_deflate_finish(compressed);

// ❌ Using FILE* or std::iostream
FILE* f = fopen("data.bin", "rb");  // use MLA streams instead

// ❌ Forgetting to flush buffered output
mla_stream_output_t buffered = mla_stream_output_buffered_wrapper(output, 4096);
// write data...
// missing: mla_stream_output_flush_buffered_wrapper(buffered);
```
