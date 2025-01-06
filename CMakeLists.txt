#
# tesseract
#

# ##############################################################################
#
# cmake settings
#
# ##############################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# In-source builds are disabled.
if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  message(
    FATAL_ERROR
      "CMake generation is not possible within the source directory!"
      "\n Remove the CMakeCache.txt file and try again from another folder, "
      "e.g.:\n "
      "\n rm CMakeCache.txt"
      "\n mkdir build"
      "\n cd build"
      "\n cmake ..")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake Targets")

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.15.0")
  if(WIN32)
    cmake_policy(SET CMP0091 NEW)
    message(STATUS "Setting policy CMP0091 to NEW")
  endif()
endif()

# ##############################################################################
#
# project settings
#
# ##############################################################################

project(tesseract C CXX)

# Get version with components from VERSION file.
file(STRINGS "VERSION" VERSION_PLAIN)
string(REGEX REPLACE "^([^.]*)\\..*" "\\1" VERSION_MAJOR ${VERSION_PLAIN})
string(REGEX REPLACE "^[^.]*\\.([^.]*)\\..*" "\\1" VERSION_MINOR
                     ${VERSION_PLAIN})
string(REGEX REPLACE "^[^.]*\\.[^.]*\\.([0-9]*).*" "\\1" VERSION_PATCH
                     ${VERSION_PLAIN})
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  execute_process(COMMAND git --git-dir ${CMAKE_CURRENT_SOURCE_DIR}/.git
                          describe --abbrev=4 OUTPUT_VARIABLE GIT_REV)
  string(REGEX REPLACE "\n$" "" PACKAGE_VERSION "${GIT_REV}")
endif()
if(NOT PACKAGE_VERSION)
  set(PACKAGE_VERSION ${VERSION_PLAIN})
endif()

# Provide also same macro names as autoconf (see configure.ac).
set(GENERIC_MAJOR_VERSION ${VERSION_MAJOR})
set(GENERIC_MINOR_VERSION ${VERSION_MINOR})
set(GENERIC_MICRO_VERSION ${VERSION_PATCH})

set(MINIMUM_LEPTONICA_VERSION 1.74)

# ##############################################################################
#
# options
#
# ##############################################################################

message(STATUS "Configuring tesseract version ${PACKAGE_VERSION}...")

if(WIN32)
  option(SW_BUILD "Build with sw" ON)
else()
  option(SW_BUILD "Build with sw" OFF)
endif()
option(OPENMP_BUILD "Build with openmp support" OFF) # see issue #1662
option(GRAPHICS_DISABLED "Disable disable graphics (ScrollView)" OFF)
option(DISABLED_LEGACY_ENGINE "Disable the legacy OCR engine" OFF)
option(ENABLE_LTO "Enable link-time optimization" OFF)
option(FAST_FLOAT "Enable float for LSTM" ON)
option(ENABLE_NATIVE
       "Enable optimization for host CPU (could break HW compatibility)" OFF)
# see
# https://stackoverflow.com/questions/52653025/why-is-march-native-used-so-rarely
option(BUILD_TRAINING_TOOLS "Build training tools" ON)
option(BUILD_TESTS "Build tests" OFF)
option(USE_SYSTEM_ICU "Use system ICU" OFF)
option(DISABLE_TIFF "Disable build with libtiff (if available)" OFF)
option(DISABLE_ARCHIVE "Disable build with libarchive (if available)" OFF)
option(DISABLE_CURL "Disable build with libcurl (if available)" OFF)
option(INSTALL_CONFIGS "Install tesseract configs" ON)

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.15.0")
  if(WIN32 AND MSVC)
    option(WIN32_MT_BUILD "Build with MT flag for MSVC" OFF)
  endif()
endif()

# ##############################################################################
#
# compiler and linker
#
# ##############################################################################

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CLANG 1)
endif()

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 17)
if("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # cygwin gnu c++ needs to use -std=gnu++17 instead of -std=c++17
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(BUILD_SHARED_LIBS)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
endif()

# LTO
cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT error)
if(LTO_SUPPORTED)
  message(STATUS "IPO / LTO supported")
else()
  message(STATUS "IPO / LTO not supported: <${error}>")
endif()

set(MARCH_NATIVE_OPT OFF)
if(ENABLE_NATIVE)
  check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
  if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(DOTPRODUCT_FLAGS "${DOTPRODUCT_FLAGS} -march=native")
    if(NOT CLANG AND MSVC)
      # clang-cl does not know this argument
      set(DOTPRODUCT_FLAGS "${DOTPRODUCT_FLAGS} -mtune=native")
    endif()
    set(MARCH_NATIVE_OPT ON)
  endif(COMPILER_SUPPORTS_MARCH_NATIVE)
endif(ENABLE_NATIVE)

message(STATUS "CMAKE_SYSTEM_PROCESSOR=<${CMAKE_SYSTEM_PROCESSOR}>")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|x86_64|AMD64|amd64|i386|i686")

  set(HAVE_NEON FALSE)
  if(MSVC)
    set(HAVE_AVX ON)
    set(AVX_COMPILE_FLAGS "/arch:AVX")
    add_definitions("-DHAVE_AVX")

    set(HAVE_AVX2 ON)
    set(AVX2_COMPILE_FLAGS "/arch:AVX2")
    add_definitions("-DHAVE_AVX2")

    set(HAVE_AVX512F ON)
    set(AVX512F_COMPILE_FLAGS "/arch:AVX512")
    add_definitions("-DHAVE_AVX512F")

    set(HAVE_FMA ON)
    set(FMA_COMPILE_FLAGS "-D__FMA__")
    add_definitions("-DHAVE_FMA")

    set(HAVE_SSE4_1 ON)
    set(SSE4_1_COMPILE_FLAGS "-D__SSE4_1__")
    add_definitions("-DHAVE_SSE4_1")

    set(DOTPRODUCT_FLAGS "${DOTPRODUCT_FLAGS} -openmp:experimental")
    add_definitions("-DOPENMP_SIMD")

    # clang with MSVC compatibility
    if(CLANG)
      set(CMAKE_CXX_FLAGS
          "${CMAKE_CXX_FLAGS} -Wno-microsoft-unqualified-friend")
      if(HAVE_FMA)
        set(FMA_COMPILE_FLAGS "-mfma ${FMA_COMPILE_FLAGS}")
      endif(HAVE_FMA)
      if(HAVE_SSE4_1)
        set(SSE4_1_COMPILE_FLAGS "-msse4.1 ${SSE4_1_COMPILE_FLAGS}")
      endif(HAVE_SSE4_1)
    endif(CLANG)
  else() # if not MSVC
    check_cxx_compiler_flag("-mavx" HAVE_AVX)
    if(HAVE_AVX)
      set(AVX_COMPILE_FLAGS "-mavx")
      add_definitions("-DHAVE_AVX")
    endif(HAVE_AVX)

    check_cxx_compiler_flag("-mavx2" HAVE_AVX2)
    if(HAVE_AVX2)
      set(AVX2_COMPILE_FLAGS "-mavx2")
      add_definitions("-DHAVE_AVX2")
    endif()

    check_cxx_compiler_flag("-mavx512f" HAVE_AVX512F)
    if(HAVE_AVX512F)
      set(AVX512F_COMPILE_FLAGS "-mavx512f")
      add_definitions("-DHAVE_AVX512F")
    endif()

    check_cxx_compiler_flag("-mfma" HAVE_FMA)
    if(HAVE_FMA)
      set(FMA_COMPILE_FLAGS "-mfma")
      add_definitions("-DHAVE_FMA")
    endif()

    check_cxx_compiler_flag("-msse4.1" HAVE_SSE4_1)
    if(HAVE_SSE4_1)
      set(SSE4_1_COMPILE_FLAGS "-msse4.1")
      add_definitions("-DHAVE_SSE4_1")
    endif()

    check_cxx_compiler_flag("-fopenmp-simd" OPENMP_SIMD)
    if(OPENMP_SIMD)
      set(DOTPRODUCT_FLAGS "${DOTPRODUCT_FLAGS} -fopenmp-simd")
      add_definitions("-DOPENMP_SIMD")
    endif(OPENMP_SIMD)
  endif(MSVC)

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64.*|AARCH64.*")

  set(HAVE_AVX FALSE)
  set(HAVE_AVX2 FALSE)
  set(HAVE_AVX512F FALSE)
  set(HAVE_FMA FALSE)
  set(HAVE_SSE4_1 FALSE)
  set(HAVE_NEON TRUE)

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm.*")

  set(HAVE_AVX FALSE)
  set(HAVE_AVX2 FALSE)
  set(HAVE_AVX512F FALSE)
  set(HAVE_FMA FALSE)
  set(HAVE_SSE4_1 FALSE)

  check_cxx_compiler_flag("-mfpu=neon" HAVE_NEON)
  if(HAVE_NEON)
    set(NEON_COMPILE_FLAGS "-mfpu=neon")
  endif(HAVE_NEON)

else()

  set(HAVE_AVX FALSE)
  set(HAVE_AVX2 FALSE)
  set(HAVE_AVX512F FALSE)
  set(HAVE_FMA FALSE)
  set(HAVE_NEON FALSE)
  set(HAVE_SSE4_1 FALSE)

endif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|x86_64|AMD64|amd64|i386|i686")

if(HAVE_NEON)
  message(STATUS "LTO build is not supported on arm/RBPi.")
  set(ENABLE_LTO FALSE)  # enable LTO cause fatal error on arm/RBPi
endif()

# Compiler specific environment
if(CMAKE_COMPILER_IS_GNUCXX OR MINGW)
  set(CMAKE_CXX_FLAGS_DEBUG
      "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -pedantic -Og -Wno-unknown-pragmas")
elseif(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE) # strdup
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
  if(NOT CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif()
  # Hide some warnings for release target wd4244 'argument': conversion from
  # 'uint64_t' to 'unsigned int', possible loss of data wd4251 needs to have
  # dll-interface wd4267 return': conversion from 'size_t' to 'int', possible
  # loss of data wd4275 non dll-interface class wd4305 ...truncation from
  # 'double' to 'float'
  set(CMAKE_CXX_FLAGS_RELEASE
      "${CMAKE_CXX_FLAGS_RELEASE} /wd4244 /wd4305 /wd4267 /wd4251 /wd4275 /wd4005"
  )
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /wd4068")
  # Don't use /Wall because it generates too many warnings.
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W0 /bigobj")
  # MT flag
  if(WIN32_MT_BUILD)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    message(STATUS "Building with static CRT.")
  endif()
  # Workaround: When building on VS 2022 17.10 or newer, but using an older runtime,
  # mutexes can crash
  # https://stackoverflow.com/questions/78598141/first-stdmutexlock-crashes-in-application-built-with-latest-visual-studio
  add_definitions(-D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
endif()
if(CLANG) # clang all platforms
  set(CMAKE_CXX_FLAGS_RELEASE
      "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unused-command-line-argument")
  set(CMAKE_CXX_FLAGS_DEBUG
      "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -pedantic -O0")
endif()

if(OPENMP_BUILD
   AND MSVC
   AND "${MSVC_VERSION}" LESS 1929)
  set(OPENMP_BUILD OFF)
endif()
if(OPENMP_BUILD)
  if(MSVC)  # supported from cmake 3.30
    set(OpenMP_RUNTIME_MSVC "llvm")
  endif(MSVC)
  find_package(OpenMP)
  # https://stackoverflow.com/questions/12399422
  # how-to-set-linker-flags-for-openmp-in-cmakes-try-compile-function
  if(NOT OpenMP_FOUND
     AND CLANG
     AND WIN32)
    # workaround because find_package(OpenMP) does not work for clang-cl
    # https://gitlab.kitware.com/cmake/cmake/issues/19404
    check_include_file_cxx(omp.h HAVE_OMP_H_INCLUDE)
    find_library(OpenMP_LIBRARY NAMES omp libomp.lib)
    message(">> OpenMP_LIBRARY: ${OpenMP_LIBRARY}")
    if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
    endif()
    set(OpenMP_FOUND 1)
    # OpenMP 3.1 is fully supported from Clang 3.8.0
    add_definitions(-D_OPENMP=201107)
  endif()
  if(MSVC)
    # Note: -openmp:llvm is available for X64 from MSVC 16.9 from MSVC 16.10
    # Preview 2 there is support also for x86 and arm64
    # https://devblogs.microsoft.com/cppblog/openmp-updates-and-fixes-for-cpp-in-visual-studio-2019-16-10/
    if("${OpenMP_CXX_FLAGS}" STREQUAL "-openmp")
      set(OpenMP_CXX_FLAGS "-openmp:llvm")
    endif()
  endif()
  if(OpenMP_FOUND)
    message(">> OpenMP_FOUND ${OpenMP_FOUND} version: ${OpenMP_CXX_VERSION}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    if(NOT TARGET OpenMP::OpenMP_CXX)
      add_library(OpenMP::OpenMP_CXX IMPORTED INTERFACE)
    endif()
  endif()
endif()

if(CYGWIN)
  add_definitions(-D__CYGWIN__)
elseif(UNIX)
  if(NOT ANDROID)
    set(LIB_pthread pthread)
  endif()
elseif(WIN32)
  set(LIB_Ws2_32 Ws2_32)
endif()

add_definitions("-DCMAKE_BUILD")

# ##############################################################################
#
# packages
#
# ##############################################################################
include(CheckFunctions)

if(SW_BUILD)
  find_package(SW REQUIRED)
  if(BUILD_SHARED_LIBS)
    set(SW_BUILD_SHARED_LIBS 1)
  else()
    set(SW_BUILD_SHARED_LIBS 0)
  endif()
  sw_add_package(org.sw.demo.danbloomberg.leptonica
                 org.sw.demo.libarchive.libarchive)
  if(BUILD_TRAINING_TOOLS)
    sw_add_package(org.sw.demo.gnome.pango.pangocairo
                   org.sw.demo.unicode.icu.i18n)
  endif()
  sw_execute()
else()
  find_package(PkgConfig)
  # Check for required library. option -DLeptonica_DIR=path => cmake hint where
  # to find leptonica
  find_package(Leptonica ${MINIMUM_LEPTONICA_VERSION} CONFIG)
  if(NOT Leptonica_FOUND AND PKG_CONFIG_EXECUTABLE)
    pkg_check_modules(Leptonica lept>=${MINIMUM_LEPTONICA_VERSION})
    link_directories(${Leptonica_LIBRARY_DIRS})
  endif()
  if(NOT Leptonica_FOUND)
    message(FATAL_ERROR "Cannot find required library Leptonica. Quitting!")
  else()
    message(STATUS "Found leptonica version: ${Leptonica_VERSION}")
  endif(NOT Leptonica_FOUND)
  include_directories(${Leptonica_INCLUDE_DIRS})

  check_leptonica_tiff_support()
  if ((NOT LEPT_TIFF_RESULT EQUAL 0) AND LEPT_TIFF_COMPILE_SUCCESS)
    message(NOTICE "Leptonica was build without TIFF support! Disabling TIFF support...")
    set(DISABLE_TIFF ON)
  elseif(NOT ${CMAKE_VERSION} VERSION_LESS "3.25")
    message(STATUS "Leptonica was build with TIFF support.")
  endif()

  # Check for optional libraries.
  if(DISABLE_TIFF)
    set(HAVE_TIFFIO_H OFF)
    message(STATUS "TIFF support disabled.")
  else(DISABLE_TIFF)
    find_package(TIFF) # for tesseract
    if(NOT TIFF_FOUND AND PKG_CONFIG_EXECUTABLE)
      # try PKG_CONFIG to find libtiff if cmake failed
      pkg_check_modules(TIFF libtiff-4)
    endif()
    if(TIFF_FOUND)
      set(HAVE_TIFFIO_H ON)
      include_directories(${TIFF_INCLUDE_DIRS})
    endif(TIFF_FOUND)
  endif(DISABLE_TIFF)
  if(DISABLE_ARCHIVE)
    set(HAVE_LIBARCHIVE OFF)
    message(STATUS "LibArchive support disabled.")
  else(DISABLE_ARCHIVE)
    find_package(LibArchive)
    if(NOT LibArchive_FOUND AND PKG_CONFIG_EXECUTABLE)
      # try PKG_CONFIG to find libarchive if cmake failed
      pkg_check_modules(LibArchive libarchive)
    endif()
    if(LibArchive_FOUND)
      set(HAVE_LIBARCHIVE ON)
      include_directories(${LibArchive_INCLUDE_DIRS})
    endif(LibArchive_FOUND)
  endif(DISABLE_ARCHIVE)
  if(DISABLE_CURL)
    set(HAVE_LIBCURL OFF)
    message(STATUS "CURL support disabled.")
  else(DISABLE_CURL)
    find_package(CURL)
    if(NOT CURL_FOUND AND PKG_CONFIG_EXECUTABLE)
      # try PKG_CONFIG to find libcurl if cmake failed
      pkg_check_modules(CURL libcurl)
    endif()
    if(CURL_FOUND)
      set(HAVE_LIBCURL ON)
      include_directories(${CURL_INCLUDE_DIRS})
    endif(CURL_FOUND)
  endif(DISABLE_CURL)
endif()

# ##############################################################################
#
# configure
#
# ##############################################################################

if(MSVC)
  set(DOTPRODUCT_FLAGS "${DOTPRODUCT_FLAGS} /fp:fast")
else()
  set(DOTPRODUCT_FLAGS "${DOTPRODUCT_FLAGS} -O3 -ffast-math")
endif()

include (GNUInstallDirs)

set(AUTOCONFIG_SRC ${CMAKE_CURRENT_BINARY_DIR}/config_auto.h.in)
set(AUTOCONFIG ${CMAKE_CURRENT_BINARY_DIR}/config_auto.h)
add_definitions(-DHAVE_CONFIG_H)

if(GRAPHICS_DISABLED)
  message("ScrollView debugging disabled.")
endif()
set(CMAKE_REQUIRED_INCLUDES
    ${CMAKE_REQUIRED_INCLUDES} "${CMAKE_PREFIX_PATH}/include"
    ${CMAKE_INSTALL_INCLUDEDIR})
include(Configure)

configure_file(${AUTOCONFIG_SRC} ${AUTOCONFIG} @ONLY)

set(INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(LIBRARY_DIRS ${CMAKE_INSTALL_LIBDIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/tesseract/version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/tesseract/version.h @ONLY)

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

# show summary of configuration
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  set(COMPILER_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(${CMAKE_BUILD_TYPE} MATCHES Release)
  set(COMPILER_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
  if(LTO_SUPPORTED AND ENABLE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
  endif() # LTO_SUPPORTED
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(BUILD_ARCH "64 bits")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(BUILD_ARCH "32 bits")
endif()

message(STATUS)
message(STATUS "General configuration for Tesseract ${PACKAGE_VERSION}")
message(STATUS "--------------------------------------------------------")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE} ${BUILD_ARCH}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Used standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "CXX compiler options: ${COMPILER_FLAGS}")
get_directory_property(DirCompDefs COMPILE_DEFINITIONS)
message(STATUS "Compile definitions = ${DirCompDefs}")
message(STATUS "Linker options: ${CMAKE_EXE_LINKER_FLAGS} "
               "${CMAKE_EXE_LINKER_FLAGS_${CMAKE_BUILD_TYPE_UP}}")
message(STATUS "Install directory: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "HAVE_AVX: ${HAVE_AVX}")
message(STATUS "HAVE_AVX2: ${HAVE_AVX2}")
message(STATUS "HAVE_AVX512F: ${HAVE_AVX512F}")
message(STATUS "HAVE_FMA: ${HAVE_FMA}")
message(STATUS "HAVE_SSE4_1: ${HAVE_SSE4_1}")
message(STATUS "MARCH_NATIVE_OPT: ${MARCH_NATIVE_OPT}")
message(STATUS "HAVE_NEON: ${HAVE_NEON}")
message(STATUS "Link-time optimization: ${CMAKE_INTERPROCEDURAL_OPTIMIZATION}")
message(STATUS "--------------------------------------------------------")
message(STATUS "Build with sw [SW_BUILD]: ${SW_BUILD}")
message(STATUS "Build with openmp support [OPENMP_BUILD]: ${OPENMP_BUILD}")
message(STATUS "Build with libarchive support [HAVE_LIBARCHIVE]: "
               "${HAVE_LIBARCHIVE}")
message(STATUS "Build with libcurl support [HAVE_LIBCURL]: ${HAVE_LIBCURL}")
message(STATUS "Enable float for LSTM [FAST_FLOAT]: ${FAST_FLOAT}")
message(STATUS "Enable optimization for host CPU (could break HW compatibility)"
               " [ENABLE_NATIVE]: ${ENABLE_NATIVE}")
message(STATUS "Disable disable graphics (ScrollView) [GRAPHICS_DISABLED]: "
               "${GRAPHICS_DISABLED}")
message(STATUS "Disable the legacy OCR engine [DISABLED_LEGACY_ENGINE]: "
               "${DISABLED_LEGACY_ENGINE}")
message(STATUS "Build training tools [BUILD_TRAINING_TOOLS]: "
               "${BUILD_TRAINING_TOOLS}")
message(STATUS "Build tests [BUILD_TESTS]: ${BUILD_TESTS}")
message(STATUS "Use system ICU Library [USE_SYSTEM_ICU]: ${USE_SYSTEM_ICU}")
message(
  STATUS "Install tesseract configs [INSTALL_CONFIGS]: ${INSTALL_CONFIGS}")
message(STATUS "--------------------------------------------------------")
message(STATUS)

# ##############################################################################
#
# build
#
# ##############################################################################

include(BuildFunctions)
include(SourceGroups)

add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
if(ANDROID_TOOLCHAIN)
  include_directories(${ANDROID_TOOLCHAIN}/sysroot/usr/include)
  add_compile_definitions(__ANDROID_API_FUTURE__)
endif()

# ##############################################################################
# LIBRARY tesseract
# ##############################################################################

file(
  GLOB
  TESSERACT_SRC
  src/ccmain/*.cpp
  src/ccstruct/*.cpp
  src/ccutil/*.cpp
  src/classify/*.cpp
  src/cutil/*.cpp
  src/dict/*.cpp
  src/lstm/*.cpp
  src/textord/*.cpp
  src/viewer/*.cpp
  src/wordrec/*.cpp)

if(DISABLED_LEGACY_ENGINE)
  # prepend path to list of source files
  function(prepend_path srcs path)
    set(tmp, "")
    foreach(src IN LISTS ${srcs})
      list(APPEND tmp ${path}/${src})
    endforeach(src ${srcs})
    set(${srcs}
        ${tmp}
        PARENT_SCOPE)
  endfunction()

  set(TESSERACT_SRC_LEGACY
      src/ccmain/adaptions.cpp
      src/ccmain/docqual.cpp
      src/ccmain/equationdetect.cpp
      src/ccmain/fixspace.cpp
      src/ccmain/fixxht.cpp
      src/ccmain/osdetect.cpp
      src/ccmain/par_control.cpp
      src/ccmain/recogtraining.cpp
      src/ccmain/superscript.cpp
      src/ccmain/tessbox.cpp
      src/ccmain/tfacepp.cpp
      src/ccstruct/fontinfo.cpp
      src/ccstruct/params_training_featdef.cpp
      src/ccutil/ambigs.cpp
      src/ccutil/bitvector.cpp
      src/ccutil/indexmapbidi.cpp
      src/classify/adaptive.cpp
      src/classify/adaptmatch.cpp
      src/classify/blobclass.cpp
      src/classify/cluster.cpp
      src/classify/clusttool.cpp
      src/classify/cutoffs.cpp
      src/classify/featdefs.cpp
      src/classify/float2int.cpp
      src/classify/fpoint.cpp
      src/classify/intfeaturespace.cpp
      src/classify/intfx.cpp
      src/classify/intmatcher.cpp
      src/classify/intproto.cpp
      src/classify/kdtree.cpp
      src/classify/mf.cpp
      src/classify/mfoutline.cpp
      src/classify/mfx.cpp
      src/classify/normfeat.cpp
      src/classify/normmatch.cpp
      src/classify/ocrfeatures.cpp
      src/classify/outfeat.cpp
      src/classify/picofeat.cpp
      src/classify/protos.cpp
      src/classify/shapeclassifier.cpp
      src/classify/shapetable.cpp
      src/classify/tessclassifier.cpp
      src/classify/trainingsample.cpp
      src/dict/permdawg.cpp
      src/dict/hyphen.cpp
      src/wordrec/associate.cpp
      src/wordrec/chop.cpp
      src/wordrec/chopper.cpp
      src/wordrec/drawfx.cpp
      src/wordrec/findseam.cpp
      src/wordrec/gradechop.cpp
      src/wordrec/language_model.cpp
      src/wordrec/lm_consistency.cpp
      src/wordrec/lm_pain_points.cpp
      src/wordrec/lm_state.cpp
      src/wordrec/outlines.cpp
      src/wordrec/params_model.cpp
      src/wordrec/pieces.cpp
      src/wordrec/plotedges.cpp
      src/wordrec/render.cpp
      src/wordrec/segsearch.cpp
      src/wordrec/wordclass.cpp)
  prepend_path(TESSERACT_SRC_LEGACY "${CMAKE_CURRENT_SOURCE_DIR}")
  list(REMOVE_ITEM TESSERACT_SRC ${TESSERACT_SRC_LEGACY})
endif(DISABLED_LEGACY_ENGINE)

list(APPEND arch_files src/arch/dotproduct.cpp src/arch/simddetect.cpp
     src/arch/intsimdmatrix.cpp)

if(DOTPRODUCT_FLAGS)
  set_source_files_properties(src/arch/dotproduct.cpp
                              PROPERTIES COMPILE_FLAGS ${DOTPRODUCT_FLAGS})
endif(DOTPRODUCT_FLAGS)
if(HAVE_AVX)
  list(APPEND arch_files_opt src/arch/dotproductavx.cpp)
  set_source_files_properties(src/arch/dotproductavx.cpp
                              PROPERTIES COMPILE_FLAGS ${AVX_COMPILE_FLAGS})
endif(HAVE_AVX)
if(HAVE_AVX2)
  list(APPEND arch_files_opt src/arch/intsimdmatrixavx2.cpp
       src/arch/dotproductavx.cpp)
  set_source_files_properties(src/arch/intsimdmatrixavx2.cpp
                              PROPERTIES COMPILE_FLAGS ${AVX2_COMPILE_FLAGS})
endif(HAVE_AVX2)
if(HAVE_AVX512F)
  list(APPEND arch_files_opt src/arch/dotproductavx512.cpp)
  set_source_files_properties(src/arch/dotproductavx512.cpp
                              PROPERTIES COMPILE_FLAGS ${AVX512F_COMPILE_FLAGS})
endif(HAVE_AVX512F)
if(HAVE_FMA)
  list(APPEND arch_files_opt src/arch/dotproductfma.cpp)
  set_source_files_properties(src/arch/dotproductfma.cpp
                              PROPERTIES COMPILE_FLAGS ${FMA_COMPILE_FLAGS})
endif(HAVE_FMA)
if(HAVE_SSE4_1)
  list(APPEND arch_files_opt src/arch/dotproductsse.cpp
       src/arch/intsimdmatrixsse.cpp)
  set_source_files_properties(
    src/arch/dotproductsse.cpp src/arch/intsimdmatrixsse.cpp
    PROPERTIES COMPILE_FLAGS ${SSE4_1_COMPILE_FLAGS})
endif(HAVE_SSE4_1)
if(HAVE_NEON)
  list(APPEND arch_files_opt src/arch/dotproductneon.cpp
       src/arch/intsimdmatrixneon.cpp)
  if(NEON_COMPILE_FLAGS)
    set_source_files_properties(
      src/arch/dotproductneon.cpp src/arch/intsimdmatrixneon.cpp
      PROPERTIES COMPILE_FLAGS ${NEON_COMPILE_FLAGS})
  endif()
endif(HAVE_NEON)

file(
  GLOB_RECURSE
  TESSERACT_HDR
  include/*
  src/arch/*.h
  src/ccmain/*.h
  src/ccstruct/*.h
  src/ccutil/*.h
  src/classify/*.h
  src/cutil/*.h
  src/dict/*.h
  src/lstm/*.h
  src/textord/*.h
  src/viewer/*.h
  src/wordrec/*.h)

set(TESSERACT_SRC
    ${TESSERACT_SRC}
    src/api/baseapi.cpp
    src/api/capi.cpp
    src/api/renderer.cpp
    src/api/altorenderer.cpp
    src/api/pagerenderer.cpp
    src/api/hocrrenderer.cpp
    src/api/lstmboxrenderer.cpp
    src/api/pdfrenderer.cpp
    src/api/wordstrboxrenderer.cpp)

set(TESSERACT_CONFIGS
    tessdata/configs/alto
    tessdata/configs/ambigs.train
    tessdata/configs/api_config
    tessdata/configs/bazaar
    tessdata/configs/bigram
    tessdata/configs/box.train
    tessdata/configs/box.train.stderr
    tessdata/configs/digits
    tessdata/configs/get.images
    tessdata/configs/hocr
    tessdata/configs/inter
    tessdata/configs/kannada
    tessdata/configs/linebox
    tessdata/configs/logfile
    tessdata/configs/lstm.train
    tessdata/configs/lstmbox
    tessdata/configs/lstmdebug
    tessdata/configs/makebox
    tessdata/configs/page
    tessdata/configs/pdf
    tessdata/configs/quiet
    tessdata/configs/rebox
    tessdata/configs/strokewidth
    tessdata/configs/tsv
    tessdata/configs/txt
    tessdata/configs/unlv
    tessdata/configs/wordstrbox)

set(TESSERACT_TESSCONFIGS
    tessdata/tessconfigs/batch tessdata/tessconfigs/batch.nochop
    tessdata/tessconfigs/matdemo tessdata/tessconfigs/msdemo
    tessdata/tessconfigs/nobatch tessdata/tessconfigs/segdemo)

set(LIBTESSFILES ${TESSERACT_SRC} ${arch_files} ${arch_files_opt}
                 ${TESSERACT_HDR})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LIBTESSFILES})

add_library(libtesseract ${LIBTESSFILES})
target_include_directories(
  libtesseract BEFORE
  PRIVATE src
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/arch>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/ccmain>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/ccstruct>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/ccutil>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/classify>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/cutil>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/dict>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lstm>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/textord>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/viewer>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/wordrec>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/training>)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(
    libtesseract
    PRIVATE -DTESS_EXPORTS
    INTERFACE -DTESS_IMPORTS)
  # generate_export_header          (libtesseract EXPORT_MACRO_NAME TESS_API)
endif()
target_link_libraries(libtesseract PRIVATE ${LIB_Ws2_32} ${LIB_pthread})
if(OpenMP_CXX_FOUND)
  target_link_libraries(libtesseract PUBLIC OpenMP::OpenMP_CXX)
endif()
if(LibArchive_FOUND)
  target_link_libraries(libtesseract PUBLIC ${LibArchive_LIBRARIES})
endif(LibArchive_FOUND)
if(CURL_FOUND)
  if(NOT CURL_LIBRARIES)
    target_link_libraries(libtesseract PUBLIC CURL::libcurl)
  else()
    target_link_libraries(libtesseract PUBLIC ${CURL_LIBRARIES})
  endif()
endif(CURL_FOUND)

set_target_properties(
  libtesseract PROPERTIES VERSION
                          ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
set_target_properties(
  libtesseract PROPERTIES SOVERSION
                          ${VERSION_MAJOR}.${VERSION_MINOR})

set_target_properties(
  libtesseract
  PROPERTIES
    OUTPUT_NAME
    tesseract$<$<BOOL:${WIN32}>:${VERSION_MAJOR}${VERSION_MINOR}$<$<CONFIG:DEBUG>:d>>
)

if(SW_BUILD)
  target_link_libraries(libtesseract PUBLIC org.sw.demo.danbloomberg.leptonica
                                            org.sw.demo.libarchive.libarchive)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/TesseractTargets.cmake
       "include(${CMAKE_CURRENT_BINARY_DIR}/cppan.cmake)\n")
  export(
    TARGETS libtesseract
    APPEND
    FILE ${CMAKE_CURRENT_BINARY_DIR}/TesseractTargets.cmake
    NAMESPACE Tesseract::)
else()
  target_link_libraries(libtesseract PUBLIC ${Leptonica_LIBRARIES})
  export(
    TARGETS libtesseract
    FILE ${CMAKE_CURRENT_BINARY_DIR}/TesseractTargets.cmake
    NAMESPACE Tesseract::)
endif()

if(WIN32
   AND CLANG
   AND OPENMP_BUILD)
  # Workaround for "libomp.lib is not automatically added on Windows" see:
  # http://lists.llvm.org/pipermail/openmp-dev/2015-August/000857.html
  target_link_libraries(libtesseract PRIVATE ${OpenMP_LIBRARY})
endif()

if(ANDROID)
  add_definitions(-DANDROID)
  find_package(CpuFeaturesNdkCompat REQUIRED)
  target_include_directories(
    libtesseract
    PRIVATE "${CpuFeaturesNdkCompat_DIR}/../../../include/ndk_compat")
  target_link_libraries(libtesseract PRIVATE CpuFeatures::ndk_compat)
endif()

# ##############################################################################
# EXECUTABLE tesseract
# ##############################################################################

add_executable(tesseract src/tesseract.cpp)
target_link_libraries(tesseract libtesseract)
if(HAVE_TIFFIO_H AND WIN32)
  target_link_libraries(tesseract ${TIFF_LIBRARIES})
endif()

if(OPENMP_BUILD AND UNIX)
  target_link_libraries(tesseract pthread)
endif()

# ##############################################################################

if(BUILD_TESTS
   AND EXISTS
       ${CMAKE_CURRENT_SOURCE_DIR}/unittest/third_party/googletest/CMakeLists.txt
)
  enable_testing()
  add_subdirectory(unittest/third_party/googletest)
  add_subdirectory(unittest)
endif()

if(BUILD_TRAINING_TOOLS)
  add_subdirectory(src/training)
endif()

get_target_property(tesseract_NAME libtesseract NAME)
get_target_property(tesseract_VERSION libtesseract VERSION)
get_target_property(tesseract_OUTPUT_NAME libtesseract OUTPUT_NAME)

configure_file(tesseract.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/tesseract.pc.in
               @ONLY)
# to resolve generator expression in OUTPUT_NAME
file(
  GENERATE
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tesseract_$<CONFIG>.pc
  INPUT ${CMAKE_CURRENT_BINARY_DIR}/tesseract.pc.in)

configure_package_config_file(
  cmake/templates/TesseractConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/tesseract/TesseractConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tesseract
  PATH_VARS INCLUDE_DIR LIBRARY_DIRS)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/tesseract/TesseractConfigVersion.cmake
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY SameMajorVersion)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/tesseract_$<CONFIG>.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  RENAME tesseract.pc)
install(TARGETS tesseract DESTINATION bin)
if (MSVC)
  install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION bin OPTIONAL)
endif()
install(
  TARGETS libtesseract
  EXPORT TesseractTargets
  RUNTIME DESTINATION bin
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
if (MSVC AND BUILD_SHARED_LIBS)
  install(FILES $<TARGET_PDB_FILE:libtesseract> DESTINATION bin OPTIONAL)
endif()
install(
  EXPORT TesseractTargets
  NAMESPACE Tesseract::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tesseract)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
  FILES include/tesseract/baseapi.h
        include/tesseract/capi.h
        include/tesseract/renderer.h
        ${CMAKE_CURRENT_BINARY_DIR}/include/tesseract/version.h
        include/tesseract/ltrresultiterator.h
        include/tesseract/pageiterator.h
        include/tesseract/resultiterator.h
        include/tesseract/osdetect.h
        include/tesseract/publictypes.h
        include/tesseract/ocrclass.h
        include/tesseract/export.h
        include/tesseract/unichar.h
        # ${CMAKE_CURRENT_BINARY_DIR}/src/endianness.h
  DESTINATION include/tesseract)

if(INSTALL_CONFIGS)
  install(FILES ${TESSERACT_CONFIGS}
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/tessdata/configs)
  install(FILES ${TESSERACT_TESSCONFIGS}
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/tessdata/tessconfigs)
endif()

# ##############################################################################
# uninstall target
# ##############################################################################
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

  add_custom_target(
    uninstall
    COMMENT "Uninstall installed files"
    COMMAND ${CMAKE_COMMAND} -P
            ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

# ##############################################################################
