# Tesseract OCR - GitHub Copilot Instructions

## Repository Overview

Tesseract is an open-source **OCR (Optical Character Recognition) engine** that recognizes text from images. This repository contains:

- **libtesseract**: C++ OCR library with C API wrapper
- **tesseract**: Command-line OCR program
- **Training tools**: For creating custom language models

**Key Facts:**
- Primary language: **C++17** (requires C++17-compliant compiler)
- Size: Large (~100MB+ with submodules)
- License: Apache 2.0
- Maintained by: Stefan Weil (lead), Zdenko Podobny (maintainer)

## Build Systems

Tesseract supports **two build systems**. Both are actively maintained and tested in CI.

### 1. Autotools (Traditional, POSIX Systems)

**When to use:** Linux, macOS (command-line), MSYS2 on Windows

**Build sequence:**
```bash
./autogen.sh                    # Generate configure script (only needed after git clone)
./configure                      # Configure build (creates Makefiles)
make                            # Build library and CLI
sudo make install               # Install to system
sudo ldconfig                   # Update library cache (Linux only)
make training                   # Build training tools (optional)
sudo make training-install      # Install training tools
```

**Important:**
- ALWAYS run `./autogen.sh` first if building from git clone
- Use `make -j N` for parallel builds (N = number of CPU cores)
- Check `configure --help` for build options
- To clean: `make clean` or `make distclean` (complete cleanup)

### 2. CMake (Modern, Cross-platform)

**When to use:** Windows (MSVC, MinGW), cross-platform, modern development

**Build sequence:**
```bash
mkdir build                     # MUST use out-of-source build
cd build
cmake ..                        # Configure (add options here)
make                            # Or: cmake --build .
sudo make install               # Install to system
```

**Important CMake options:**
- `BUILD_TRAINING_TOOLS=ON` - Enable training tools build
- `CMAKE_BUILD_TYPE=Release` - Release build (default is RelWithDebInfo)
- `GRAPHICS_DISABLED=ON` - Disable ScrollView (GUI debugger)
- `ENABLE_NATIVE=OFF` - Disable CPU-specific optimizations (for portability)

**CMake enforces out-of-source builds** - you cannot build in the source directory. If you get an error about this, remove `CMakeCache.txt` and build in a separate directory.

## Dependencies

### Core Required Dependencies

- **Leptonica 1.74.2+** (REQUIRED) - Image I/O library
  - Without this, build will fail
  - Usually installed via package manager: `libleptonica-dev` (Ubuntu) or `leptonica` (Homebrew)

- **C++17 compiler:**
  - GCC 7+, Clang 5+, MSVC 2017+
  - Verified compilers: gcc-11, gcc-12, gcc-14, clang-15, clang++

### Training Tools Dependencies

Only needed if building training tools (`make training` or `-DBUILD_TRAINING_TOOLS=ON`):

- pango-devel / libpango1.0-dev
- cairo-devel
- icu-devel

### Optional Dependencies

- **libarchive-dev**, **libcurl4-openssl-dev** - For advanced features
- **OpenMP** - For parallel processing (enabled by default if available)
- **cabextract** - For testing with CAB archives

### Traineddata Files

Tesseract requires **traineddata files** to function. Minimum required:
- `eng.traineddata` (English)
- `osd.traineddata` (Orientation and Script Detection)

**Installation:**
```bash
# Download individual files to TESSDATA_PREFIX directory
wget https://github.com/tesseract-ocr/tessdata/raw/main/eng.traineddata
wget https://github.com/tesseract-ocr/tessdata/raw/main/osd.traineddata

# Or clone all languages (WARNING: 1.2+ GB)
git clone https://github.com/tesseract-ocr/tessdata.git
```

**Set environment variable:**
```bash
export TESSDATA_PREFIX=/usr/local/share/tessdata/
```

Verify with: `tesseract --list-langs`

## Testing

### Running Unit Tests

**With autotools:**
```bash
./autogen.sh
./configure
make
make check                      # Runs all unit tests
```

**With CMake:**
```bash
mkdir build && cd build
cmake ..
make
ctest                          # Or: cmake --build . --target test
```

**Important:**
- Tests require `googletest` submodule: `git submodule update --init --recursive`
- Tests require tessdata files (eng, osd minimum)
- Test results in `test-suite.log` (autotools) or CTest output (CMake)

### Running Tesseract CLI

Basic test commands:
```bash
# After installation:
tesseract --version
tesseract --list-langs
tesseract input.png output      # OCR image, creates output.txt
tesseract input.png output pdf  # Create searchable PDF
```

Test files available in `test/testing/`:
- `phototest.tif` - English test image
- `devatest.png` - Hindi/Devanagari test image

## Project Structure

### Source Code Layout

```
src/
├── api/               # Public C/C++ API (baseapi.h, capi.h)
├── ccmain/            # Main OCR control logic
├── lstm/              # LSTM neural network engine (Tesseract 4+)
├── ccutil/, cutil/    # Core utilities, data structures
├── classify/          # Character classifier
├── dict/              # Dictionary and language model
├── textord/           # Text line and word detection
├── wordrec/           # Word recognition
├── training/          # Training tools (lstmtraining, text2image, etc.)
└── tesseract.cpp      # CLI main() entry point

include/tesseract/     # Public header files
unittest/              # Unit tests (requires googletest)
test/testing/          # Test images and data
tessdata/              # Default location for traineddata files
doc/                   # Documentation
```

### Key Files

- **src/api/baseapi.h** - Main C++ API class (`TessBaseAPI`)
- **src/api/capi.h** - C wrapper API
- **src/tesseract.cpp** - Command-line tool
- **CMakeLists.txt**, **configure.ac**, **Makefile.am** - Build configuration
- **VERSION** - Current version string

### Configuration Files

- **.clang-format** - Code formatting rules (LLVM style)
- **tesseract.pc.in** - pkg-config template
- **.github/workflows/** - CI/CD definitions

## CI/CD Workflows

### Active Workflows

1. **cmake.yml** - CMake builds on Ubuntu/macOS, 6 configurations
2. **autotools.yml** - Autotools builds, comprehensive testing
3. **unittest.yml** - Unit tests with sanitizers (ASAN, UBSAN)
4. **codeql-analysis.yml** - Security static analysis
5. **vcpkg.yml**, **msys2.yml**, **cmake-win64.yml** - Windows builds

### Validation Requirements

All PRs trigger:
- **Build tests** on multiple platforms (Ubuntu 22.04, 24.04, macOS 14, 15)
- **Compiler tests** (GCC 11-14, Clang 15)
- **Unit tests** with sanitizers
- **CodeQL** security scan

**Expect ~10-30 minutes** for full CI validation.

### Common CI Failures

- **Missing dependencies:** Check workflow files for required packages
- **Test failures:** Often due to missing tessdata files
- **Sanitizer errors:** Memory leaks, undefined behavior
- **CodeQL alerts:** Security vulnerabilities in code

## Common Build Issues & Workarounds

### Issue: "configure: error: Leptonica not found"
**Solution:** Install leptonica development package
```bash
# Ubuntu/Debian:
sudo apt-get install libleptonica-dev
# macOS:
brew install leptonica
```

### Issue: "CMake Error: cannot build in source directory"
**Solution:** CMake requires out-of-source builds
```bash
rm -f CMakeCache.txt
mkdir build && cd build && cmake ..
```

### Issue: "make check" fails with "cannot find tessdata"
**Solution:** Set TESSDATA_PREFIX or download files
```bash
export TESSDATA_PREFIX=/usr/local/share/tessdata/
# Or copy files to /usr/local/share/tessdata/
```

### Issue: Submodule errors (googletest, test)
**Solution:** Initialize submodules
```bash
git submodule update --init --recursive
```

### Issue: Old Tesseract version conflicts
**Solution:** Remove previous installation before building
```bash
# Find installed files:
which tesseract
pkg-config --modversion tesseract
# Uninstall old version, then rebuild
```

### Issue: Training tools not building
**Solution:** Install pango, cairo, icu dependencies
```bash
sudo apt-get install libpango1.0-dev libcairo2-dev libicu-dev
```

## Validation Steps for Code Changes

When making code changes, follow these steps:

1. **Build the project** (choose one):
   ```bash
   # Autotools:
   ./autogen.sh && ./configure && make
   # CMake:
   mkdir build && cd build && cmake .. && make
   ```

2. **Run unit tests**:
   ```bash
   # Autotools:
   make check
   # CMake:
   ctest
   ```

3. **Test CLI manually**:
   ```bash
   tesseract test/testing/phototest.tif output
   cat output.txt  # Verify OCR output
   ```

4. **Check for memory issues** (if modifying C++ code):
   ```bash
   # Build with sanitizers:
   CXXFLAGS="-g -O2 -fsanitize=address,undefined" ./configure
   make && make check
   ```

5. **Run CodeQL** (security check):
   - Will run automatically in CI
   - Or use GitHub Code Scanning locally

6. **Verify documentation** (if API changes):
   - Update header comments in `include/tesseract/`
   - Update relevant docs in `doc/`

## Code Style & Conventions

- **Formatting:** Use clang-format with `.clang-format` config (LLVM style)
- **Naming:** 
  - Classes: `CamelCase` (e.g., `TessBaseAPI`)
  - Functions: `CamelCase` (e.g., `ProcessPage`)
  - Variables: `snake_case` or `lower_case`
- **Headers:** Use include guards, document public APIs
- **Comments:** Focus on "why", not "what"
- **Commits:** Use meaningful messages, reference issue numbers

## Important Notes for AI Coding Agents

1. **Always use out-of-source builds with CMake** - in-source builds are blocked
2. **Check for Leptonica** before building - it's a hard requirement
3. **Initialize git submodules** before running tests
4. **Set TESSDATA_PREFIX** or tests will fail
5. **Building takes time** - allow 2-5 minutes for full build
6. **Testing takes time** - `make check` can take 5-10 minutes
7. **Don't remove existing tests** - they're critical for preventing regressions
8. **Check CI workflows** for platform-specific requirements
9. **Sanitizer builds are slower** - 2-3x slower than normal builds
10. **Training tools are optional** - only build if needed for the task

## Useful Commands Reference

```bash
# Quick build and test (autotools):
./autogen.sh && ./configure && make -j8 && make check

# Quick build and test (CMake):
mkdir build && cd build && cmake .. && make -j8 && ctest

# Format code:
clang-format -i src/**/*.cpp src/**/*.h

# Check test results:
cat test-suite.log                    # autotools
ctest --output-on-failure             # CMake

# Install only library (no training):
make install                          # After ./configure && make

# Clean builds:
make clean                            # Partial clean
make distclean                        # Complete clean (autotools)
rm -rf build                          # Complete clean (CMake)

# Check installed version:
tesseract --version
pkg-config --modversion tesseract

# Debug OCR on specific image:
tesseract input.png output -l eng --psm 6 -c debug_file=/dev/null
```

---

**Trust these instructions.** Only search for additional information if these instructions are incomplete, outdated, or if you encounter an error not covered here. The workflows and build procedures are tested daily in CI and represent current best practices for this repository.
