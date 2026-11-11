# AHMAPIpy

`AHMAPIpy` is a native Python extension library written in C/C++ and brought to Python via `nanobind`. Compiled using `scikit-build-core`, it allows you to control and interact with AHM radar devices from Python scripts.

## Prerequisites

To compile and install this extension module, you only need standard development tools installed on your operating system. **You do not need to pre-install `nanobind` or `scikit-build-core` manually**, as `pip` will automatically fetch them inside an isolated temporary build environment.

Ensure your system meets the following requirements:

*   **Python:** Version 3.12 or higher. On Linux/Ubuntu, ensure `python3-dev` is installed (sudo apt install python3-dev).
*   **CMake:** Version 3.15 or higher (can be installed via: pip install cmake).
*   **C++17-Capable compiler** for exemple: 
    *   **Linux:** GCC 8+ or Clang 8+
    *   **Windows:** Visual Studio 2019 or newer (MSVC compiler, requiring the "Desktop development with C++" workload)

## Installation

First, clone the repository and navigate to the project root directory:

```bash
git clone https://github.com <!-- Bisogna sostituire con il link e la directory corrette -->
cd APIPython
```

Install the extension module using `pip`:

```bash
pip install .
```

This command automatically triggers `scikit-build-core`, builds the C++ extensions with stable ABI support (`abi3`), and installs the final `AHMAPIpy` module.

## Supported Architecture

The `CMakeLists.txt` build pipeline handles cross-platform differences based on the compiler and available system libraries:

*   **Windows (MSVC compiler):** Automatically defines the `-DWIN` macro and compiles serial communication using native Windows API headers (`winSerial.c`).
*   **Linux/POSIX (Non-MSVC compilers):** Compiles serial communication using POSIX-compliant headers (`wiringSerial.c`).

## Verification

Once installed, you can verify that the Python extension module works and check the exposed C++ functions by running:

```bash
python -c "import AHMAPIpy; print('Exposed functions and structures:', dir(AHMAPIpy))"
```
