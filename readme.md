# Base64 File Converter – reliable CLI tool for Windows 10

A lightweight, single‑file C++ utility that encodes **any binary file** to Base64 and decodes it back, exactly as online converters do. Perfect for embedding binary data in text‑based formats, email attachments, or debugging.

## Why this version?

Other converters often fail on files that contain extra whitespace, line breaks, or a UTF‑8 BOM. **This tool matches the behaviour of popular online Base64 decoders** – it filters out all non‑Base64 characters (space, newline, tab, BOM, etc.) before decoding. So it never rejects a valid Base64 file just because it has some "noise".

## Features

- ✅ **Encode** any file (images, executables, RDF, documents) to Base64.
- ✅ **Decode** any valid Base64 file – even if it contains spaces, newlines, or a BOM.
- ✅ **Soft validation** – only characters `A-Z a-z 0-9 + / =` are kept; everything else is ignored.
- ✅ **Robust decoding** – uses a standard bit‑shifting algorithm, correctly handles padding (`=` at the end).
- ✅ **Verbose logging** with timestamps, file sizes, and progress (every ~10% during encoding).
- ✅ **Works with large files** – encodes in 3 KB chunks (memory efficient).
- ✅ **No external dependencies** – pure C++17 (or C++11 with minor changes).

## Requirements

- Windows 10 (also works on 7/8/11)
- A C++ compiler with C++11 support:
  - MinGW‑w64 (GCC)
  - Microsoft Visual Studio (MSVC)
  - Clang

## Build

**Note:** Compilation commands assume you are in the directory containing `base64_converter.cpp`.

### Using MinGW (GCC)
```bash
g++ -std=c++11 base64_converter.cpp -o base64_converter.exe
```

### Using MSVC (Visual Studio Developer Command Prompt)
```bash
cl /EHsc /std:c++11 base64_converter.cpp /Fe:base64_converter.exe
```


## Usage
```bash
base64_converter.exe encode <input_file> <output_file>
base64_converter.exe decode <input_file> <output_file>
```

### Example: encode a binary file
``` bash
base64_converter.exe encode photo.jpg photo_base64.txt

```

**Console output**:
``` log
[LOG] 2025-04-30 12:34:56 - Encoding: photo.jpg
[LOG] 2025-04-30 12:34:56 - Input size: 245760 bytes
[LOG] 2025-04-30 12:34:57 - Progress: 10%
...
[LOG] 2025-04-30 12:34:58 - Encoding done -> photo_base64.txt
Encoding successful.
```



### Example: decode back
```bash
base64_converter.exe decode photo_base64.txt restored.jpg
```
```
[LOG] 2025-04-30 12:35:00 - Decoding: photo_base64.txt
[LOG] 2025-04-30 12:35:00 - Read 327692 bytes
[LOG] 2025-04-30 12:35:00 - After Base64 filter: 327680 chars
[LOG] 2025-04-30 12:35:00 - Decoded: 245760 bytes
[LOG] 2025-04-30 12:35:00 - Decoding done -> restored.jpg
Decoding successful.
```


## How it differs from naive validators

Most command‑line Base64 tools require the input to be **strictly** a valid Base64 string – no extra spaces, no line breaks, no BOM. This tool **first filters out all non‑Base64 characters**, then decodes the result. That's exactly what online decoders do.

For example, the following string (with spaces and newlines) is **rejected** by many tools, but accepted by this one:
```
SGVs bG8g
V29y bGQ=
```


After filtering, it becomes `SGVsbG8gV29ybGQ=` which decodes correctly to `"Hello World"`.

## Behind the scenes

- **Encoding**: reads the input in chunks of 3 KB (multiple of 3 for alignment), converts each chunk to Base64, writes to output.
- **Decoding**: reads the whole file (flexible, suitable for files up to a few hundred MB), strips all non‑Base64 characters, then runs a bit‑shifting decoder. If you need to decode terabytes of data, a streaming version can be added on request.
- **Logging**: uses `localtime_s` on Windows to avoid compiler warnings.

## Troubleshooting

### "Error: No valid Base64 characters found"

The input file contains none of the 64 symbols. Probably you are trying to decode a binary file. Run `encode` first.

### "Error: Cannot open input file"

Check file path and permissions.

### Decoding succeeds but output is corrupted

This should not happen with the latest version. Please open an issue with a minimal example.

## License

MIT – free to use, modify, and distribute.

## Author

Created for Windows 10 in standard C++11.


