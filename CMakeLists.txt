#
# tesseract
#

###############################################################################
#
# cmake settings
#
###############################################################################

cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

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

option(CPPAN_BUILD "Build with cppan" ON)
option(SW_BUILD "Build with sw" OFF)
option(OPENMP_BUILD "Build with openmp support" OFF)  # see issue #1662
option(AUTO_OPTIMIZE "Usage of cmake auto optimize macros (not suitable for portable build)" ON)
option(GRAPHICS_DISABLED "Disable disable graphics (ScrollView)" OFF)
option(DISABLED_LEGACY_ENGINE "Disable the legacy OCR engine" OFF)
option(BUILD_TRAINING_TOOLS "Build training tools" ON)
option(BUILD_TESTS "Build tests" OFF)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.cppan OR SW_BUILD)
    set(CPPAN_BUILD OFF)
endif()

###############################################################################
#
# compiler and linker
#
###############################################################################

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

# auto optimize
if (AUTO_OPTIMIZE)
    include(OptimizeForArchitecture)
    AutodetectHostArchitecture()
    OptimizeForArchitecture()
endif()
# Compiler specific environments
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CLANG 1)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR MINGW)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -pedantic -Og")
elseif(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    if (NOT CLANG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()
    # Don't use /Wall because it generates too many warnings.
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 /bigobj")
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
        add_library(OpenMP::OpenMP_CXX IMPORTED INTERFACE)
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

if(CPPAN_BUILD)
    if (STATIC)
        set(CPPAN_BUILD_SHARED_LIBS 0)
    else()
        set(CPPAN_BUILD_SHARED_LIBS 1)
    endif()
    add_subdirectory(.cppan)
elseif (SW_BUILD)
    find_package(SW REQUIRED)
    if (STATIC)
        set(SW_BUILD_SHARED_LIBS 0)
    else()
        set(SW_BUILD_SHARED_LIBS 1)
    endif()
    sw_add_package(
        org.sw.demo.danbloomberg.leptonica-master
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

foreach(flag ${Vc_ARCHITECTURE_FLAGS})
    set(Vc_CXX_FLAGS "${Vc_CXX_FLAGS} ${flag}")
endforeach()

# add definition as expected in src/arch/simddetect.cpp
set(AVX_OPT OFF)
set(AVX2_OPT OFF)
set(FMA_OPT OFF)
set(SSE41_OPT OFF)
set(MARCH_NATIVE_OPT OFF)
foreach(flag ${_enable_vector_unit_list})  # from OptimizeForArchitecture()
    string(TOUPPER "${flag}" flag)
    string(REPLACE "\." "_" flag "${flag}")
    set(simd_flags "${simd_flags} -D${flag}")
    string(REPLACE "_" "" flag "${flag}")
    if("${flag}" MATCHES "AVX|AVX2|FMA|SSE41")
        set("${flag}_OPT" ON)
    endif()
endforeach(flag)
if (NOT MSVC)
   set(MARCH_NATIVE_FLAGS "${MARCH_NATIVE_FLAGS} -O3 -ffast-math")
endif()
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(MARCH_NATIVE_FLAGS "${MARCH_NATIVE_FLAGS} -march=native -mtune=native")
    set(MARCH_NATIVE_OPT ON)
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/tess_version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/api/tess_version.h @ONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vs2010/tesseract/tesseract.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/vs2010/tesseract/tesseract.rc @ONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vs2010/tesseract/libtesseract.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/vs2010/tesseract/libtesseract.rc @ONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/TesseractConfig-version.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/TesseractConfig-version.cmake @ONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/TesseractConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/TesseractConfig.cmake @ONLY)

# show summary of configuration
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    set(COMPILER_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(${CMAKE_BUILD_TYPE} MATCHES Release)
    set(COMPILER_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
endif()
message( STATUS )
message( STATUS "General configuration for Tesseract ${PACKAGE_VERSION}")
message( STATUS "--------------------------------------------------------")
message( STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message( STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message( STATUS "Used standard: C++${CMAKE_CXX_STANDARD}")
message( STATUS "CXX compiler options: ${COMPILER_FLAGS}")
message( STATUS "Linker options: ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_${CMAKE_BUILD_TYPE_UP}}")
message( STATUS "Install directory: ${CMAKE_INSTALL_PREFIX}")
message( STATUS "Architecture flags: ${Vc_ARCHITECTURE_FLAGS}")
message( STATUS "Vector unit list: ${_enable_vector_unit_list}")
message( STATUS "AVX_OPT: ${AVX_OPT}")
message( STATUS "AVX2_OPT: ${AVX2_OPT}")
message( STATUS "FMA_OPT: ${FMA_OPT}")
message( STATUS "SSE41_OPT: ${SSE41_OPT}")
message( STATUS "MARCH_NATIVE_OPT: ${MARCH_NATIVE_OPT}")
message( STATUS "simd_flags: ${simd_flags}")
message( STATUS "--------------------------------------------------------")
message( STATUS "Build with cppan [CPPAN_BUILD]: ${CPPAN_BUILD}")
if (CPPAN_BUILD)
message( STATUS "##################################################################################")
message(  "!! CPPAN is depreciated! Please consider switching to SW Build.\n"
          "   More details: https://github.com/tesseract-ocr/tesseract/wiki/Compiling#windows")
message( STATUS "##################################################################################")
endif()
message( STATUS "Build with sw [SW_BUILD]: ${SW_BUILD}")
message( STATUS "Build with openmp support [OPENMP_BUILD]: ${OPENMP_BUILD}")
message( STATUS "Disable disable graphics (ScrollView) [GRAPHICS_DISABLED]: ${GRAPHICS_DISABLED}")
message( STATUS "Disable the legacy OCR engine [DISABLED_LEGACY_ENGINE]: ${DISABLED_LEGACY_ENGINE}")
message( STATUS "Build training tools [BUILD_TRAINING_TOOLS]: ${BUILD_TRAINING_TOOLS}")
message( STATUS "Build tests [BUILD_TESTS]: ${BUILD_TESTS}")
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
include_directories(src/api)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/api)
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

list(APPEND arch_files
    src/arch/dotproduct.cpp
    src/arch/simddetect.cpp
    src/arch/intsimdmatrix.cpp
)
set_source_files_properties(${arch_files} PROPERTIES COMPILE_FLAGS "${simd_flags}")
set_source_files_properties(src/arch/dotproduct.cpp PROPERTIES COMPILE_FLAGS "${MARCH_NATIVE_FLAGS} ${Vc_CXX_FLAGS}")
if(AVX_OPT)
   list(APPEND arch_files_opt src/arch/dotproductavx.cpp)
   set_source_files_properties(src/arch/dotproductavx.cpp PROPERTIES COMPILE_FLAGS "-DAVX")
endif(AVX_OPT)
if(AVX2_OPT)
   list(APPEND arch_files_opt src/arch/intsimdmatrixavx2.cpp)
   set_source_files_properties(src/arch/intsimdmatrixavx2.cpp PROPERTIES COMPILE_FLAGS "-DAVX2")
endif(AVX2_OPT)
if(FMA_OPT)
   list(APPEND arch_files_opt src/arch/dotproductfma.cpp)
   set_source_files_properties(src/arch/dotproductfma.cpp PROPERTIES COMPILE_FLAGS "-mfma")
endif(FMA_OPT)
if(SSE41_OPT)
   list(APPEND arch_files_opt src/arch/dotproductsse.cpp src/arch/intsimdmatrixsse.cpp)
   set_source_files_properties(src/arch/dotproductsse.cpp src/arch/intsimdmatrixsse.cpp PROPERTIES COMPILE_FLAGS "-DSSE4_1 -msse4.1")
endif(SSE41_OPT)
set_source_files_properties(${arch_files_opt} PROPERTIES COMPILE_FLAGS "${Vc_CXX_FLAGS}")

file(GLOB tesseract_hdr
    src/api/*.h
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
set_target_properties           (libtesseract PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
set_target_properties           (libtesseract PROPERTIES SOVERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
if (WIN32)
set_target_properties           (libtesseract PROPERTIES OUTPUT_NAME tesseract${VERSION_MAJOR}${VERSION_MINOR})
set_target_properties           (libtesseract PROPERTIES DEBUG_OUTPUT_NAME tesseract${VERSION_MAJOR}${VERSION_MINOR}d)
else()
set_target_properties           (libtesseract PROPERTIES OUTPUT_NAME tesseract)
endif()

if (CPPAN_BUILD)
    target_link_libraries       (libtesseract PUBLIC
        pvt.cppan.demo.danbloomberg.leptonica
        pvt.cppan.demo.libarchive.libarchive
    )
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/TesseractTargets.cmake "include(${CMAKE_CURRENT_BINARY_DIR}/cppan.cmake)\n")
    export(TARGETS libtesseract APPEND FILE ${CMAKE_CURRENT_BINARY_DIR}/TesseractTargets.cmake)
elseif (SW_BUILD)
    target_link_libraries       (libtesseract PUBLIC
        org.sw.demo.danbloomberg.leptonica-master
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
install(EXPORT TesseractTargets DESTINATION cmake)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/TesseractConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/TesseractConfig-version.cmake
    DESTINATION cmake)

install(FILES
    # from api/makefile.am
    src/api/apitypes.h
    src/api/baseapi.h
    src/api/capi.h
    src/api/renderer.h
    ${CMAKE_CURRENT_BINARY_DIR}/api/tess_version.h

    #from ccmain/makefile.am
    src/ccmain/thresholder.h
    src/ccmain/ltrresultiterator.h
    src/ccmain/pageiterator.h
    src/ccmain/resultiterator.h
    src/ccmain/osdetect.h

    #from ccstruct/makefile.am
    src/ccstruct/publictypes.h

    #from ccutil/makefile.am
    src/ccutil/genericvector.h
    src/ccutil/helpers.h
    src/ccutil/ocrclass.h
    src/ccutil/platform.h
    src/ccutil/serialis.h
    src/ccutil/strngs.h
    src/ccutil/tesscallback.h
    src/ccutil/unichar.h
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
