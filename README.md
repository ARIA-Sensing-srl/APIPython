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
git clone https://github.com/ARIA-Sensing-srl/APIPython.git
```
```bash
cd APIPython
```
```bash
git submodule update --init --recursive
```

Install the extension module using `pip`:

```bash (for Linux)
pip install .
```
```bash (for Windows)
python.exe -m pip install .
```

This command automatically triggers `scikit-build-core`, builds the C++ extensions with stable ABI support (`abi3`), and installs the final `AHMAPIpy` module.

## Verification

Once installed, you can verify that the Python extension module works and check the exposed C++ functions by running:

```bash
python -c "import AHMAPIpy; print('Exposed functions and structures:', dir(AHMAPIpy))"
```
