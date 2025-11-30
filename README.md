# Assembler Project

A two-pass assembler written in ANSI C (C89) for a custom assembly language. This project is a final project for a systems programming laboratory course.

## Overview

This assembler processes assembly source files and generates machine code output. It implements:

- **Preprocessor**: Expands macro definitions
- **First Pass**: Builds symbol table and calculates addresses
- **Second Pass**: Generates final machine code output

## Features

- Macro expansion support (`mcro` / `mcrend` directives)
- Multiple addressing modes:
  - Immediate (`#number`)
  - Direct (label reference)
  - Matrix access (`LABEL[rX][rY]`)
  - Register direct (`r0`-`r7`)
- Data directives: `.data`, `.string`, `.mat`
- Symbol directives: `.entry`, `.extern`
- 16 supported operation codes: `mov`, `cmp`, `add`, `sub`, `lea`, `clr`, `not`, `inc`, `dec`, `jmp`, `bne`, `jsr`, `red`, `prn`, `rts`, `stop`
- Comprehensive error handling and reporting

## Project Structure

```
├── CMakeLists.txt        # CMake build configuration
├── include/              # Header files
│   ├── errors.h          # Error codes and handling
│   ├── globals.h         # Global definitions and utilities
│   ├── line_parser.h     # Assembly line parsing
│   ├── macro.h           # Macro definitions
│   ├── second_pass.h     # Second pass declarations
│   ├── symbol_table.h    # Symbol table management
│   ├── util_hash.h       # Hash table implementation
│   └── util_vec.h        # Dynamic vector implementation
├── src/                  # Source files
│   ├── assembler.c       # Main entry point
│   ├── errors.c          # Error handling implementation
│   ├── first_pass.c      # First pass implementation
│   ├── line_parser.c     # Line parsing implementation
│   ├── preprocessor.c    # Macro preprocessor
│   ├── second_pass.c     # Second pass implementation
│   ├── symbol_table.c    # Symbol table implementation
│   ├── util_hash.c       # Hash table implementation
│   ├── util_vec.c        # Vector implementation
│   └── utils.c           # Utility functions
└── tests/                # Test files
    ├── hash_test.c       # Hash table tests
    ├── parser_test.c     # Line parser tests
    ├── preprocessor_test.c # Preprocessor tests
    └── vector_test.c     # Vector tests
```

## Building

### Prerequisites

- CMake 3.31 or higher
- GCC or compatible C compiler supporting ANSI C (C89)

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

This will create:
- `assembler` - The main assembler executable
- `test_hash` - Hash table test executable
- `test_parser` - Line parser test executable
- `test_vec` - Vector test executable
- `test_preprocessor` - Preprocessor test executable

## Usage

```bash
./assembler <file1> [file2] ... [fileN]
```

The assembler expects input files with the `.as` extension. For each input file, it will:

1. Preprocess the file (expand macros) and create a `.am` file
2. Perform the first pass to build the symbol table
3. Perform the second pass to generate output files

### Output Files

- `.am` - Preprocessed assembly file (macros expanded)
- `.ob` - Object file (machine code)
- `.ent` - Entry points file (if `.entry` directives are used)
- `.ext` - External references file (if `.extern` directives are used)

## Assembly Language Syntax

### Labels

Labels are defined at the beginning of a line, followed by a colon:

```assembly
LABEL: mov r1, r2
```

### Directives

- `.data` - Define numeric data: `.data 1, 2, 3, -5`
- `.string` - Define a string: `.string "Hello"`
- `.mat` - Define a matrix: `.mat [2][3] 1, 2, 3, 4, 5, 6`
- `.entry` - Mark a symbol as an entry point: `.entry LABEL`
- `.extern` - Declare an external symbol: `.extern LABEL`

### Macros

```assembly
mcro MACRO_NAME
    ; macro body
    mov r1, r2
mcrend
```

### Instructions

The assembler supports 16 instructions:

| Opcode | Description |
|--------|-------------|
| `mov`  | Move data |
| `cmp`  | Compare |
| `add`  | Addition |
| `sub`  | Subtraction |
| `lea`  | Load effective address |
| `clr`  | Clear |
| `not`  | Bitwise NOT |
| `inc`  | Increment |
| `dec`  | Decrement |
| `jmp`  | Jump |
| `bne`  | Branch if not equal |
| `jsr`  | Jump to subroutine |
| `red`  | Read |
| `prn`  | Print |
| `rts`  | Return from subroutine |
| `stop` | Stop execution |

## Running Tests

After building, run the individual test executables:

```bash
./test_hash
./test_parser
./test_vec
./test_preprocessor
```

## Technical Details

- **Memory Model**: Code starts at address 100
- **Line Length**: Maximum 80 characters per line
- **Label Length**: Maximum 30 characters
- **Image Size**: Maximum 256 words
- **Hash Table**: Uses djb2 hash function with chaining for collision resolution
- **Dynamic Vector**: Auto-growing array implementation for flexible data storage

## Compiler Flags

The project is compiled with strict ANSI C compliance:

- `-std=c89 -ansi` - ANSI C standard
- `-pedantic` - Issue all warnings demanded by strict ISO C
- `-Wall -Wextra` - Enable comprehensive warnings
- `-Werror` - Treat warnings as errors
- `-g` - Include debug information

## License

This project is part of an academic course and is intended for educational purposes.
