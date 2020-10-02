#
# tesseract
#

###############################################################################
#
# cmake settings
#
###############################################################################

cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

# In-source builds are disabled.
if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    message(FATAL_ERROR
        "CMake generation is not possible within the source directory!"
        "\n Remove the CMakeCache.txt file and try again from another folder, e.g.:"
        "\n "
        "\n rm CMakeCache.txt"
        "\n mkdir build"
        "\n cd build"
        "\n cmake .."
    )
endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake Targets")

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.15.0")
    if (WIN32)
        cmake_policy(SET CMP0091 NEW)
        message(STATUS "Setting policy CMP0091 to NEW")
    endif()
endif()

###############################################################################
#
# project settings
#
###############################################################################

project(tesseract C CXX)

# Get version with components from VERSION file.
file(STRINGS "VERSION" VERSION_PLAIN)
string(REGEX REPLACE "^([^.]*)\\..*" "\\1" VERSION_MAJOR ${VERSION_PLAIN})
string(REGEX REPLACE "^[^.]*\\.([^.]*)\\..*" "\\1" VERSION_MINOR ${VERSION_PLAIN})
string(REGEX REPLACE "^[^.]*\\.[^.]*\\.([0-9]*).*" "\\1" VERSION_PATCH ${VERSION_PLAIN})
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
    execute_process(COMMAND git --git-dir ${CMAKE_CURRENT_SOURCE_DIR}/.git describe --abbrev=4
        OUTPUT_VARIABLE GIT_REV)
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

###############################################################################
#
# options
#
###############################################################################

message( "Configuring tesseract version ${PACKAGE_VERSION}...")

if (WIN32)
    option(SW_BUILD "Build with sw" ON)
else()
    option(SW_BUILD "Build with sw" OFF)
endif()
option(OPENMP_BUILD "Build with openmp support" OFF)  # see issue #1662
option(GRAPHICS_DISABLED "Disable disable graphics (ScrollView)" OFF)
option(DISABLED_LEGACY_ENGINE "Disable the legacy OCR engine" OFF)
option(ENABLE_LTO "Enable link-time optimization" OFF)
option(BUILD_TRAINING_TOOLS "Build training tools" ON)
option(BUILD_TESTS "Build tests" OFF)
option(USE_SYSTEM_ICU "Use system ICU" OFF)
if(NOT ${CMAKE_VERSION} VERSION_LESS "3.15.0")
    if(WIN32 AND MSVC)
        option(WIN32_MT_BUILD "Build with MT flag for MSVC" OFF)
    endif()
endif()

###############################################################################
#
# compiler and linker
#
###############################################################################

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CLANG 1)
endif()

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

include(CheckCXXCompilerFlag)

# Check for C++ standard to use
get_property(known_features GLOBAL PROPERTY CMAKE_CXX_KNOWN_FEATURES)
if (cxx_std_17 IN_LIST known_features)
    set(CMAKE_CXX_STANDARD 17)
elseif (cxx_std_14 IN_LIST known_features)
    set(CMAKE_CXX_STANDARD 14)
else()  # minimum required standard
    set(CMAKE_CXX_STANDARD 11)
endif()

if ("${CMAKE_CXX17_STANDARD_COMPILE_OPTION}" STREQUAL "")
    set(CMAKE_CXX_STANDARD 14)
endif()
if ("${CMAKE_CXX14_STANDARD_COMPILE_OPTION}" STREQUAL "")
    set(CMAKE_CXX_STANDARD 11)
endif()

# Avoid using experimental c++1y (c++1z) standard even if the compiler announces cxx14 (cxx17)
# in CMAKE_CXX_KNOWN_FEATURES and CMAKE_CXX_COMPILE_FEATURES
# It is the case of clang 3.9, 4.0 (announces c++1z) and gcc 4.8 (announces c++1y)
if ("${CMAKE_CXX17_STANDARD_COMPILE_OPTION}" STREQUAL "-std=c++1z")
  set(CMAKE_CXX_STANDARD 14)
endif()
if ("${CMAKE_CXX14_STANDARD_COMPILE_OPTION}" STREQUAL "-std=c++1y")
  set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LIBRARY_TYPE SHARED)
if (STATIC)
    set(LIBRARY_TYPE)
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

CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(MARCH_NATIVE_FLAGS "${MARCH_NATIVE_FLAGS} -march=native -mtune=native")
    set(MARCH_NATIVE_OPT ON)
endif()

CHECK_CXX_COMPILER_FLAG("-mavx" HAVE_AVX)
if(HAVE_AVX)
    set(AVX_COMPILE_FLAGS "-mavx")
    add_definitions("-DHAVE_AVX")
endif(HAVE_AVX)

CHECK_CXX_COMPILER_FLAG("-mavx2" HAVE_AVX2)
if(HAVE_AVX2)
    set(AVX2_COMPILE_FLAGS "-mavx2")
    add_definitions("-DHAVE_AVX2")
endif()

CHECK_CXX_COMPILER_FLAG("-mfma" HAVE_FMA)
if(HAVE_FMA)
    set(FMA_COMPILE_FLAGS "-mfma")
    add_definitions("-DHAVE_FMA")
endif()

CHECK_CXX_COMPILER_FLAG("-msse4.1" HAVE_SSE4_1)
if(HAVE_SSE4_1)
    set(SSE4_1_COMPILE_FLAGS "-msse4.1")
    add_definitions("-DHAVE_SSE4_1")
endif()

if(NOT APPLE)
    # NEON support relies on getauxval, which is not available on OSX, only on Linux and Android
    CHECK_CXX_COMPILER_FLAG("-mfpu=neon" HAVE_NEON)
    if(HAVE_NEON)
        set(NEON_COMPILE_FLAGS "-mfpu=neon")
        add_definitions("-DHAVE_NEON")
    endif()
endif(NOT APPLE)

if(MSVC)
    if(NOT HAVE_AVX)
        set(AVX_COMPILE_FLAGS "/arch:AVX")
        set(HAVE_AVX ON)
        add_definitions("-DHAVE_AVX")
    endif()

    if(NOT HAVE_AVX2)
        set(AVX2_COMPILE_FLAGS "/arch:AVX2")
        set(HAVE_AVX2 ON)
        add_definitions("-DHAVE_AVX2")
        set(FMA_COMPILE_FLAGS "-D__FMA__")
        set(HAVE_FMA ON)
        add_definitions("-DHAVE_FMA")
    endif()

    if(NOT HAVE_SSE4_1)
        set(SSE4_1_COMPILE_FLAGS "-D__SSE4_1__")
        set(HAVE_SSE4_1 ON)
        add_definitions("-DHAVE_SSE4_1")
    endif()
endif(MSVC)

# auto optimize - used only for information about available vectors
include(OptimizeForArchitecture)
OptimizeForArchitecture()
# remove global definition to eliminate effect on build
foreach(_flag ${_enable_vector_unit_list})
    string(TOUPPER "${_flag}" _flag)
    string(REPLACE "." "_" _flag "__${_flag}__")
    remove_definitions("-D${_flag}")
endforeach(_flag)
foreach(flag ${Vc_ARCHITECTURE_FLAGS})
    set(Vc_CXX_FLAGS "${Vc_CXX_FLAGS} ${flag}")
endforeach()

# Compiler specific environments
if(CMAKE_COMPILER_IS_GNUCXX OR MINGW)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -pedantic -Og")
elseif(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    if (NOT CLANG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /wd4244 /wd4305 /wd4267")
    # Don't use /Wall because it generates too many warnings.
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 /bigobj")
    # MT flag
    if(WIN32_MT_BUILD)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        message (STATUS "Building with static CRT.")
    endif()
endif()
if(CLANG)  # clang all platforms
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unused-command-line-argument")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -pedantic -O0")
endif()

if (OPENMP_BUILD)
    find_package(OpenMP QUIET)
    if (OpenMP_FOUND)
        message(">> ${OpenMP_FOUND} ${OpenMP_VERSION}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        if(${CMAKE_VERSION} VERSION_LESS "3.9.0")
            add_library(OpenMP::OpenMP_CXX IMPORTED INTERFACE)
        endif()
    endif()
    # https://stackoverflow.com/questions/12399422/how-to-set-linker-flags-for-openmp-in-cmakes-try-compile-function
    if (NOT OpenMP_FOUND AND CLANG AND WIN32)
        # workaroung because find_package(OpenMP) does not work for clang-cl
        # https://gitlab.kitware.com/cmake/cmake/issues/19404
        check_include_file_cxx(omp.h HAVE_OMP_H_INCLUDE)
        find_library(OpenMP_LIBRARY NAMES omp libomp.lib)
        message(">> OpenMP_LIBRARY: ${OpenMP_LIBRARY}")
        if (MSVC)
            set(OpenMP_CXX_FLAGS "${OpenMP_CXX_FLAGS} /openmp")
        else()
            set(OpenMP_CXX_FLAGS "${OpenMP_CXX_FLAGS} -fopenmp")
        endif()
        set(OpenMP_FOUND 1)
        add_definitions(-D_OPENMP=201107)  # 3.1 version is supported from Clang 3.8.0
    endif()
endif()


if (CYGWIN)
    add_definitions(-D__CYGWIN__)
elseif(UNIX)
    if (NOT ANDROID)
        set(LIB_pthread pthread)
    endif()
elseif(WIN32)
    set(LIB_Ws2_32 Ws2_32)
endif()

###############################################################################
#
# packages
#
###############################################################################

if (SW_BUILD)
    find_package(SW REQUIRED)
    if (STATIC)
        set(SW_BUILD_SHARED_LIBS 0)
    else()
        set(SW_BUILD_SHARED_LIBS 1)
    endif()
    sw_add_package(
        org.sw.demo.danbloomberg.leptonica
        org.sw.demo.libarchive.libarchive
    )
    if (BUILD_TRAINING_TOOLS)
        sw_add_package(
            org.sw.demo.gnome.pango.pangocairo
            org.sw.demo.unicode.icu.i18n
        )
    endif()
    sw_execute()
else()
    find_package(PkgConfig)
    if(PKG_CONFIG_EXECUTABLE AND NOT Leptonica_DIR)
        pkg_check_modules(Leptonica REQUIRED lept>=${MINIMUM_LEPTONICA_VERSION})
        link_directories(${Leptonica_LIBRARY_DIRS})
    else()
        find_package(Leptonica ${MINIMUM_LEPTONICA_VERSION} REQUIRED CONFIG)
    endif()
    if (NOT Leptonica_FOUND)
        message(FATAL_ERROR "Cannot find required library Leptonica. Quitting!")
    endif(NOT Leptonica_FOUND)

    find_package(LibArchive)
    if(LibArchive_FOUND)
        set(HAVE_LIBARCHIVE ON)
    endif()
endif()

find_package(OpenCL QUIET)


###############################################################################
#
# configure
#
###############################################################################

if (NOT MSVC)
   set(MARCH_NATIVE_FLAGS "${MARCH_NATIVE_FLAGS} -O3 -ffast-math")
endif()

set(AUTOCONFIG_SRC ${CMAKE_CURRENT_BINARY_DIR}/config_auto.h.in)
set(AUTOCONFIG ${CMAKE_CURRENT_BINARY_DIR}/config_auto.h)
add_definitions(-DHAVE_CONFIG_H)

if(GRAPHICS_DISABLED)
    message("ScrollView debugging disabled.")
endif()
set (CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} "${CMAKE_PREFIX_PATH}/include" "${CMAKE_INSTALL_PREFIX}/include")
include(Configure)

configure_file(${AUTOCONFIG_SRC} ${AUTOCONFIG} @ONLY)

set(INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include" "${CMAKE_INSTALL_PREFIX}/include/tesseract")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tesseract/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/tesseract/version.h @ONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vs2010/tesseract/tesseract.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/vs2010/tesseract/tesseract.rc @ONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vs2010/tesseract/libtesseract.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/vs2010/tesseract/libtesseract.rc @ONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/TesseractConfig-version.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/TesseractConfig-version.cmake @ONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/TesseractConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/TesseractConfig.cmake @ONLY)

# show summary of configuration
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    set(COMPILER_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(${CMAKE_BUILD_TYPE} MATCHES Release)
    set(COMPILER_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
    if (LTO_SUPPORTED AND ENABLE_LTO)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
    endif()  # LTO_SUPPORTED
endif()
message( STATUS )
message( STATUS "General configuration for Tesseract ${PACKAGE_VERSION}")
message( STATUS "--------------------------------------------------------")
message( STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message( STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message( STATUS "Used standard: C++${CMAKE_CXX_STANDARD}")
message( STATUS "CXX compiler options: ${COMPILER_FLAGS}")
get_directory_property( DirCompDefs COMPILE_DEFINITIONS)
message( STATUS "Compile definitions = ${DirCompDefs}")
message( STATUS "Linker options: ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_${CMAKE_BUILD_TYPE_UP}}")
message( STATUS "Install directory: ${CMAKE_INSTALL_PREFIX}")
message( STATUS "Architecture flags: ${Vc_ARCHITECTURE_FLAGS}")
message( STATUS "Vector unit list: ${_enable_vector_unit_list}")
message( STATUS "HAVE_AVX: ${HAVE_AVX}")
message( STATUS "HAVE_AVX2: ${HAVE_AVX2}")
message( STATUS "HAVE_FMA: ${HAVE_FMA}")
message( STATUS "HAVE_SSE4_1: ${HAVE_SSE4_1}")
message( STATUS "MARCH_NATIVE_OPT: ${MARCH_NATIVE_OPT}")
message( STATUS "HAVE_NEON: ${HAVE_NEON}")
message( STATUS "Link-time optimization: ${CMAKE_INTERPROCEDURAL_OPTIMIZATION}")
message( STATUS "--------------------------------------------------------")
message( STATUS "Build with sw [SW_BUILD]: ${SW_BUILD}")
message( STATUS "Build with openmp support [OPENMP_BUILD]: ${OPENMP_BUILD}")
message( STATUS "Disable disable graphics (ScrollView) [GRAPHICS_DISABLED]: ${GRAPHICS_DISABLED}")
message( STATUS "Disable the legacy OCR engine [DISABLED_LEGACY_ENGINE]: ${DISABLED_LEGACY_ENGINE}")
message( STATUS "Build training tools [BUILD_TRAINING_TOOLS]: ${BUILD_TRAINING_TOOLS}")
message( STATUS "Build tests [BUILD_TESTS]: ${BUILD_TESTS}")
message( STATUS "Use system ICU Library [USE_SYSTEM_ICU]: ${USE_SYSTEM_ICU}")
message( STATUS "--------------------------------------------------------")
message( STATUS )

###############################################################################
#
# build
#
###############################################################################

include(BuildFunctions)
include(SourceGroups)

add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1)

include_directories(${Leptonica_INCLUDE_DIRS})
include_directories(${LibArchive_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
include_directories(include)
include_directories(src/arch)
include_directories(src/ccmain)
include_directories(src/ccstruct)
include_directories(src/ccutil)
include_directories(src/classify)
include_directories(src/cutil)
include_directories(src/dict)
include_directories(src/lstm)
include_directories(src/opencl)
include_directories(src/textord)
include_directories(src/viewer)
include_directories(src/wordrec)
include_directories(src/training)
if(ANDROID_TOOLCHAIN)
  include_directories(${ANDROID_TOOLCHAIN}/sysroot/usr/include)
  add_compile_definitions(__ANDROID_API_FUTURE__)
endif()

########################################
# LIBRARY tesseract
########################################

file(GLOB tesseract_src
    src/ccmain/*.cpp
    src/ccstruct/*.cpp
    src/ccutil/*.cpp
    src/classify/*.cpp
    src/cutil/*.cpp
    src/dict/*.cpp
    src/lstm/*.cpp
    src/opencl/*.cpp
    src/textord/*.cpp
    src/viewer/*.cpp
    src/wordrec/*.cpp
)

if (DISABLED_LEGACY_ENGINE)
    function(prepend_path srcs path)
        set(tmp, "")
        foreach(src IN LISTS ${srcs})
            list(APPEND tmp ${path}/${src})
        endforeach(src ${srcs})
        set(${srcs} ${tmp} PARENT_SCOPE)
    endfunction()

    SET(tesseract_src_legacy
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
        src/ccutil/universalambigs.cpp
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
        src/classify/mfdefs.cpp
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
        src/wordrec/wordclass.cpp
    )
    prepend_path(tesseract_src_legacy "${CMAKE_CURRENT_SOURCE_DIR}")
    list(REMOVE_ITEM tesseract_src ${tesseract_src_legacy})
endif(DISABLED_LEGACY_ENGINE)

list(APPEND arch_files
    src/arch/dotproduct.cpp
    src/arch/simddetect.cpp
    src/arch/intsimdmatrix.cpp
)

if(MARCH_NATIVE_FLAGS)
    set_source_files_properties(src/arch/dotproduct.cpp
                                PROPERTIES COMPILE_FLAGS ${MARCH_NATIVE_FLAGS})
endif(MARCH_NATIVE_FLAGS)
if(HAVE_AVX)
    list(APPEND arch_files_opt src/arch/dotproductavx.cpp)
    set_source_files_properties(src/arch/dotproductavx.cpp
                                PROPERTIES COMPILE_FLAGS ${AVX_COMPILE_FLAGS})
endif(HAVE_AVX)
if(HAVE_AVX2)
    list(APPEND arch_files_opt src/arch/intsimdmatrixavx2.cpp src/arch/dotproductavx.cpp)
    set_source_files_properties(src/arch/intsimdmatrixavx2.cpp
                                PROPERTIES COMPILE_FLAGS ${AVX2_COMPILE_FLAGS})
endif(HAVE_AVX2)
if(HAVE_FMA)
    list(APPEND arch_files_opt src/arch/dotproductfma.cpp)
    set_source_files_properties(src/arch/dotproductfma.cpp
                                PROPERTIES COMPILE_FLAGS ${FMA_COMPILE_FLAGS})
endif(HAVE_FMA)
if(HAVE_SSE4_1)
    list(APPEND arch_files_opt src/arch/dotproductsse.cpp src/arch/intsimdmatrixsse.cpp)
    set_source_files_properties(src/arch/dotproductsse.cpp src/arch/intsimdmatrixsse.cpp
                                PROPERTIES COMPILE_FLAGS ${SSE4_1_COMPILE_FLAGS})
endif(HAVE_SSE4_1)
if(HAVE_NEON)
   list(APPEND arch_files_opt src/arch/intsimdmatrixneon.cpp)
   set_source_files_properties(src/arch/intsimdmatrixneon.cpp
                               PROPERTIES COMPILE_FLAGS ${NEON_COMPILE_FLAGS})
endif(HAVE_NEON)

file(GLOB tesseract_hdr
    src/arch/*.h
    src/ccmain/*.h
    src/ccstruct/*.h
    src/ccutil/*.h
    src/classify/*.h
    src/cutil/*.h
    src/dict/*.h
    src/lstm/*.h
    src/opencl/*.h
    src/textord/*.h
    src/viewer/*.h
    src/wordrec/*.h
)

set(tesseract_src ${tesseract_src}
    src/api/baseapi.cpp
    src/api/capi.cpp
    src/api/renderer.cpp
    src/api/altorenderer.cpp
    src/api/hocrrenderer.cpp
    src/api/lstmboxrenderer.cpp
    src/api/pdfrenderer.cpp
    src/api/wordstrboxrenderer.cpp
)

if (WIN32)
    if (MSVC)
        include_directories(src/vs2010/tesseract)
        set(tesseract_hdr
            ${tesseract_hdr}
            ${CMAKE_CURRENT_SOURCE_DIR}/src/vs2010/tesseract/resource.h)
        set(tesseract_rsc ${CMAKE_CURRENT_BINARY_DIR}/vs2010/tesseract/libtesseract.rc)
    endif()  # MSVC
endif()

add_library                     (libtesseract ${LIBRARY_TYPE} ${tesseract_src} ${arch_files}
    ${arch_files_opt} ${tesseract_hdr} ${tesseract_rsc}
    )
if (NOT STATIC)
target_compile_definitions      (libtesseract
    PRIVATE -DTESS_EXPORTS
    INTERFACE -DTESS_IMPORTS
)
set_target_properties           (libtesseract PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS True)
endif()
target_link_libraries           (libtesseract PRIVATE ${LIB_Ws2_32} ${LIB_pthread})
if(OpenMP_CXX_FOUND)
    target_link_libraries(libtesseract PUBLIC OpenMP::OpenMP_CXX)
endif()
set_target_properties           (libtesseract PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
set_target_properties           (libtesseract PROPERTIES SOVERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
if (WIN32)
set_target_properties           (libtesseract PROPERTIES OUTPUT_NAME tesseract${VERSION_MAJOR}${VERSION_MINOR})
set_target_properties           (libtesseract PROPERTIES DEBUG_OUTPUT_NAME tesseract${VERSION_MAJOR}${VERSION_MINOR}d)
else()
set_target_properties           (libtesseract PROPERTIES OUTPUT_NAME tesseract)
endif()

if (SW_BUILD)
    target_link_libraries       (libtesseract PUBLIC
        org.sw.demo.danbloomberg.leptonica
        org.sw.demo.libarchive.libarchive
    )
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/TesseractTargets.cmake "include(${CMAKE_CURRENT_BINARY_DIR}/cppan.cmake)\n")
    export(TARGETS libtesseract APPEND FILE ${CMAKE_CURRENT_BINARY_DIR}/TesseractTargets.cmake)
else()
    target_link_libraries       (libtesseract PUBLIC
        ${Leptonica_LIBRARIES}
        ${LibArchive_LIBRARIES}
    )
    export(TARGETS libtesseract FILE ${CMAKE_CURRENT_BINARY_DIR}/TesseractTargets.cmake)
endif()

if (WIN32 AND CLANG AND OPENMP_BUILD)
    # Workaround for "libomp.lib is not automatically added on Windows"
    # see: http://lists.llvm.org/pipermail/openmp-dev/2015-August/000857.html
    target_link_libraries (libtesseract PRIVATE ${OpenMP_LIBRARY})
endif()

########################################
# EXECUTABLE tesseractmain
########################################

set(tesseractmain_src src/api/tesseractmain.cpp)
if (MSVC)
    set(tesseractmain_rsc ${CMAKE_CURRENT_BINARY_DIR}/vs2010/tesseract/tesseract.rc)
endif()

add_executable                  (tesseract ${tesseractmain_src} ${tesseractmain_rsc})
target_link_libraries           (tesseract libtesseract)
if (HAVE_TIFFIO_H)
    target_link_libraries(tesseract tiff)
endif()

if (OPENMP_BUILD AND UNIX)
target_link_libraries           (tesseract pthread)
endif()

########################################

if (BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/googletest/CMakeLists.txt)
    add_subdirectory(googletest)
endif()

if (BUILD_TRAINING_TOOLS)
add_subdirectory(src/training)
endif()

get_target_property(tesseract_NAME libtesseract NAME)
get_target_property(tesseract_VERSION libtesseract VERSION)
get_target_property(tesseract_OUTPUT_NAME libtesseract OUTPUT_NAME)
configure_file(tesseract.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/tesseract.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tesseract.pc DESTINATION lib/pkgconfig)
install(TARGETS tesseract RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(TARGETS libtesseract EXPORT TesseractTargets RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(EXPORT TesseractTargets DESTINATION lib/tesseract)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cmake TYPE LIB)

install(FILES
    # from api/makefile.am
    include/tesseract/apitypes.h
    include/tesseract/baseapi.h
    include/tesseract/capi.h
    include/tesseract/renderer.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/tesseract/version.h

    #from ccmain/makefile.am
    include/tesseract/thresholder.h
    include/tesseract/ltrresultiterator.h
    include/tesseract/pageiterator.h
    include/tesseract/resultiterator.h
    include/tesseract/osdetect.h

    #from ccstruct/makefile.am
    include/tesseract/publictypes.h

    #from ccutil/makefile.am
    include/tesseract/genericvector.h
    include/tesseract/helpers.h
    include/tesseract/ocrclass.h
    include/tesseract/platform.h
    include/tesseract/serialis.h
    include/tesseract/strngs.h
    include/tesseract/unichar.h

    #${CMAKE_CURRENT_BINARY_DIR}/src/endianness.h
    DESTINATION include/tesseract)

########################################
# uninstall target
########################################
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

###############################################################################
