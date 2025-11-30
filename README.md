
# Systems Programming Laboratory: Assembler Project

![Language](https://img.shields.io/badge/language-C-00599C?style=flat-square&logo=c&logoColor=white)
![Build](https://img.shields.io/badge/build-Makefile-green?style=flat-square)
![Course](https://img.shields.io/badge/Open_University-20465-blue?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-lightgrey?style=flat-square)

## 📖 Overview

This repository contains the final project for the **Laboratory in Systems Programming (20465)** course.

The project involves designing and implementing a **Two-Pass Assembler** for a specific assembly language defined in the course curriculum. The assembler translates assembly source code into machine code, handling memory allocation, symbol management, and binary encoding.

---

## ⚙️ Key Features

* **Macro Expansion (Pre-Assembler):** Detects and expands macros, simplifying the code before compilation.
* **Syntax Validation:** Robust error handling that detects syntax errors, invalid addressing modes, and illegal operations.
* **Symbol Management:** Efficient handling of labels, variables, and external/entry directives using dynamic tables.
* **Binary Encoding:** Converts assembly instructions into the specific Base-32 format required by the imaginary CPU.
* **File Generation:** Produces the necessary output files for the linker/loader (`.ob`, `.ent`, `.ext`).

---

## 🛠️ Compilation

To compile the project, you can use the provided `Makefile`.

1. Open your terminal in the project root directory.
2. Run the following command:

```bash
make
````

*This will compile the source code and generate an executable named `assembler`.*

---

## 🚀 Usage

To run the assembler, provide the input filenames as command-line arguments.
**Note:** Do not include the `.as` extension in the argument; the program appends it automatically.

```bash
./assembler file1 file2 file3
```

### Example

If you have a file named `my_code.as`:

```bash
./assembler my_code
```

---

## 📂 Output Files

For a valid input file `filename.as`, the assembler generates:

| Extension  | Description                                                             |
| ---------- | ----------------------------------------------------------------------- |
| **`.am`**  | **After Macro:** The assembly file after macro expansion.               |
| **`.ob`**  | **Object File:** Contains the machine code (Instruction & Data memory). |
| **`.ent`** | **Entries:** Lists symbols exported to other files.                     |
| **`.ext`** | **Externals:** Lists external symbols used in this file.                |

*If errors are found during the process, no output files are generated (except the error log printed to `stderr`).*

---

## 🏗️ Project Architecture

The program operates in three main phases:

### 1. **Pre-Assembler Phase**

* Scans the source code for `mcr` and `endmcr` definitions.
* Expands macros and generates the `.am` file.

### 2. **First Pass**

* Parses the `.am` file line by line.
* Builds the **Symbol Table**.
* Updates the **Instruction Counter (IC)** and **Data Counter (DC)**.
* Flags basic syntax errors.

### 3. **Second Pass**

* Re-scans the code to resolve symbolic addresses.
* Encodes the instructions and data into machine word format.
* Generates the final output files (`.ob`, `.ent`, `.ext`).

---

## 🧪 Testing

The `tests/` directory contains example files and test programs used to verify the assembler's correctness:

* **Valid Inputs:** Ensure correct machine code generation.
* **Invalid Inputs:** Confirm that the assembler catches and reports errors properly.

---

## 🗂️ Project Structure

```
├── include/             # Header files
│   ├── assembler.h
│   ├── globals.h
│   ├── line_parser.h
│   ├── macro.h
│   ├── second_pass.h
│   ├── symbol_table.h
│   ├── util_hash.h
│   └── util_vec.h
│
├── src/                 # Source files
│   ├── assembler.c
│   ├── preprocessor.c
│   ├── first_pass.c
│   ├── second_pass.c
│   ├── line_parser.c
│   ├── symbol_table.c
│   ├── util_hash.c
│   ├── util_vec.c
│   └── utils.c
│
├── tests/               # Unit tests & example input files
│   ├── hash_test.c
│   ├── parser_test.c
│   ├── preprocessor_test.c
│   └── vector_test.c
│
├── Makefile             # Build configuration
├── CMakeLists.txt       # Optional CMake build system
└── README.md            # Project documentation
```

---

## 👨‍💻 Author

**Osher Akshikar**

* [GitHub Profile](https://github.com/osherakshikar)
* [LinkedIn](https://www.linkedin.com/in/osher-akshikar/)

---

## 📄 License

This project is licensed under the **MIT License**.
See the `LICENSE` file for details.

---

*This project is for educational purposes as part of the Open University Computer Science curriculum (20465).*

```
