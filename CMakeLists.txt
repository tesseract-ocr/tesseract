#
# tesseract
#

###############################################################################
#
# cmake settings
#
###############################################################################

cmake_minimum_required(VERSION 2.8.11)

# In-source builds are disabled.
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
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

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake")

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

# Provide also same macro names as autoconf (see configure.ac).
set(GENERIC_MAJOR_VERSION ${VERSION_MAJOR})
set(GENERIC_MINOR_VERSION ${VERSION_MINOR})
set(GENERIC_MICRO_VERSION ${VERSION_PATCH})

set(MINIMUM_LEPTONICA_VERSION 1.74)

###############################################################################
#
# compiler and linker
#
###############################################################################

set(LIBRARY_TYPE SHARED)
if (STATIC)
    set(LIBRARY_TYPE)
endif()

if (WIN32)
    if (MSVC)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_definitions(-DNOMINMAX)

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /openmp /utf-8")
    endif()

    set(LIB_Ws2_32 Ws2_32)
endif()

if (CYGWIN)
    add_definitions(-D__CYGWIN__)
endif()

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

    set(LIB_pthread pthread)
endif()

###############################################################################
#
# packages
#
###############################################################################

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/.cppan)
    if (NOT Leptonica_DIR AND NOT MSVC)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(Leptonica REQUIRED lept>=${MINIMUM_LEPTONICA_VERSION})
        link_directories(${Leptonica_LIBRARY_DIRS})
    else()
        find_package(Leptonica ${MINIMUM_LEPTONICA_VERSION} REQUIRED CONFIG)
    endif()
else()
    if (STATIC)
        set(CPPAN_BUILD_SHARED_LIBS 0)
    else()
        set(CPPAN_BUILD_SHARED_LIBS 1)
    endif()
    add_subdirectory(.cppan)
endif()

find_package(OpenCL QUIET)

option(BUILD_TRAINING_TOOLS "Build training tools" ON)
option(BUILD_TESTS "Build tests" OFF)

###############################################################################
#
# configure
#
###############################################################################

set(AUTOCONFIG_SRC ${CMAKE_BINARY_DIR}/config_auto.h.in)
set(AUTOCONFIG ${CMAKE_BINARY_DIR}/config_auto.h)

include(Configure)

configure_file(${AUTOCONFIG_SRC} ${AUTOCONFIG} @ONLY)

set(INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include" "${CMAKE_INSTALL_PREFIX}/include/tesseract")

configure_file(
    ${CMAKE_SOURCE_DIR}/src/api/tess_version.h.in
    ${CMAKE_BINARY_DIR}/api/tess_version.h @ONLY)
configure_file(
    ${CMAKE_SOURCE_DIR}/src/vs2010/tesseract/tesseract.rc.in
    ${CMAKE_BINARY_DIR}/vs2010/tesseract/tesseract.rc @ONLY)
configure_file(
    ${CMAKE_SOURCE_DIR}/src/vs2010/tesseract/libtesseract.rc.in
    ${CMAKE_BINARY_DIR}/vs2010/tesseract/libtesseract.rc @ONLY)
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/templates/TesseractConfig-version.cmake.in
    ${CMAKE_BINARY_DIR}/TesseractConfig-version.cmake @ONLY)
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/templates/TesseractConfig.cmake.in
    ${CMAKE_BINARY_DIR}/TesseractConfig.cmake @ONLY)

###############################################################################
#
# build
#
###############################################################################

include(BuildFunctions)
include(SourceGroups)

add_definitions(-DHAVE_CONFIG_H)
add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1)
add_definitions(-DWINDLLNAME="libtesseract${VERSION_MAJOR}${VERSION_MINOR}.dll")

include_directories(${Leptonica_INCLUDE_DIRS})

include_directories(${CMAKE_BINARY_DIR})

include_directories(src/api)
include_directories(${CMAKE_BINARY_DIR}/api)
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
include_directories(src/vs2010/port)
include_directories(src/viewer)
include_directories(src/wordrec)

########################################
# LIBRARY tesseract
########################################

file(GLOB tesseract_src
    src/arch/*.cpp
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
if (WIN32)
    file(GLOB tesseract_win32_src "src/vs2010/port/*.cpp")
    file(GLOB tesseract_win32_hdr "src/vs2010/port/*.h")
    set(tesseract_src ${tesseract_src} ${tesseract_win32_src})
    set(tesseract_hdr ${tesseract_hdr} ${tesseract_win32_hdr})
endif()

set(tesseract_src ${tesseract_src}
    src/api/baseapi.cpp
    src/api/capi.cpp
    src/api/renderer.cpp
    src/api/pdfrenderer.cpp
)

if (WIN32)
    if (MSVC)
        include_directories(src/vs2010/tesseract)
        set(tesseract_hdr
            ${tesseract_hdr}
            ${CMAKE_CURRENT_SOURCE_DIR}/src/vs2010/tesseract/resource.h)
        set(tesseract_rsc ${CMAKE_BINARY_DIR}/vs2010/tesseract/libtesseract.rc)
        set_source_files_properties(
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/dotproductsse.cpp
            PROPERTIES COMPILE_DEFINITIONS __SSE4_1__)
        set_source_files_properties(
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/intsimdmatrixsse.cpp
            PROPERTIES COMPILE_DEFINITIONS __SSE4_1__)
        set_source_files_properties(
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/dotproductavx.cpp
            PROPERTIES COMPILE_FLAGS "/arch:AVX")
        set_source_files_properties(
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/intsimdmatrixavx2.cpp
            PROPERTIES COMPILE_FLAGS "/arch:AVX2")
    endif()
else()
    set_source_files_properties(
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/dotproductsse.cpp
            PROPERTIES COMPILE_FLAGS "-msse4.1")
    set_source_files_properties(
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/intsimdmatrixsse.cpp
            PROPERTIES COMPILE_FLAGS "-msse4.1")
    set_source_files_properties(
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/dotproductavx.cpp
            PROPERTIES COMPILE_FLAGS "-mavx")
    set_source_files_properties(
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/intsimdmatrixavx2.cpp
            PROPERTIES COMPILE_FLAGS "-mavx2")
endif()

add_library                     (libtesseract ${LIBRARY_TYPE} ${tesseract_src} ${tesseract_hdr}
    ${tesseract_rsc}
    )
if (NOT STATIC)
target_compile_definitions      (libtesseract
    PRIVATE -DTESS_EXPORTS
    INTERFACE -DTESS_IMPORTS
)
set_target_properties           (libtesseract PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS True)
endif()
target_link_libraries           (libtesseract ${LIB_Ws2_32} ${LIB_pthread})
set_target_properties           (libtesseract PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
set_target_properties           (libtesseract PROPERTIES SOVERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
if (WIN32)
set_target_properties           (libtesseract PROPERTIES OUTPUT_NAME tesseract${VERSION_MAJOR}${VERSION_MINOR})
set_target_properties           (libtesseract PROPERTIES DEBUG_OUTPUT_NAME tesseract${VERSION_MAJOR}${VERSION_MINOR}d)
else()
set_target_properties           (libtesseract PROPERTIES OUTPUT_NAME tesseract)
endif()

if (NOT CPPAN_BUILD)
    target_link_libraries       (libtesseract ${Leptonica_LIBRARIES})
    export(TARGETS libtesseract FILE ${CMAKE_BINARY_DIR}/TesseractTargets.cmake)
else()
    target_link_libraries       (libtesseract pvt.cppan.demo.danbloomberg.leptonica)
    file(WRITE ${CMAKE_BINARY_DIR}/TesseractTargets.cmake "include(${CMAKE_BINARY_DIR}/cppan.cmake)\n")
    export(TARGETS libtesseract APPEND FILE ${CMAKE_BINARY_DIR}/TesseractTargets.cmake)
endif()

########################################
# EXECUTABLE tesseractmain
########################################

set(tesseractmain_src src/api/tesseractmain.cpp)
if (MSVC)
    set(tesseractmain_rsc ${CMAKE_BINARY_DIR}/vs2010/tesseract/tesseract.rc)
endif()

add_executable                  (tesseract ${tesseractmain_src} ${tesseractmain_rsc})
target_link_libraries           (tesseract libtesseract)

########################################

if (BUILD_TESTS AND EXISTS ${PROJECT_SOURCE_DIR}/googletest/CMakeLists.txt)
    add_subdirectory(googletest)
    add_executable(tesseract_tests tests/tesseracttests.cpp)
    target_link_libraries(tesseract_tests gtest_main)
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
    ${CMAKE_BINARY_DIR}/TesseractConfig.cmake
    ${CMAKE_BINARY_DIR}/TesseractConfig-version.cmake
    DESTINATION cmake)

install(FILES
    # from api/makefile.am
    src/api/apitypes.h
    src/api/baseapi.h
    src/api/capi.h
    src/api/renderer.h
    ${CMAKE_CURRENT_BINARY_DIR}/api/tess_version.h

    #from arch/makefile.am
    src/arch/dotproductavx.h
    src/arch/dotproductsse.h
    src/arch/intsimdmatrix.h
    src/arch/intsimdmatrixavx2.h
    src/arch/intsimdmatrixsse.h
    src/arch/simddetect.h

    #from ccmain/makefile.am
    src/ccmain/thresholder.h
    src/ccmain/ltrresultiterator.h
    src/ccmain/pageiterator.h
    src/ccmain/resultiterator.h
    src/ccmain/osdetect.h

    #from ccstruct/makefile.am
    src/ccstruct/publictypes.h

    #from ccutil/makefile.am
    src/ccutil/basedir.h
    src/ccutil/errcode.h
    src/ccutil/fileerr.h
    src/ccutil/genericvector.h
    src/ccutil/helpers.h
    src/ccutil/host.h
    src/ccutil/params.h
    src/ccutil/ocrclass.h
    src/ccutil/platform.h
    src/ccutil/serialis.h
    src/ccutil/strngs.h
    src/ccutil/tesscallback.h
    src/ccutil/unichar.h
    src/ccutil/unicharcompress.h
    src/ccutil/unicharmap.h
    src/ccutil/unicharset.h

    #from lstm/makefile.am
    src/lstm/convolve.h
    src/lstm/ctc.h
    src/lstm/fullyconnected.h
    src/lstm/functions.h
    src/lstm/input.h
    src/lstm/lstm.h
    src/lstm/lstmrecognizer.h
    src/lstm/lstmtrainer.h
    src/lstm/maxpool.h
    src/lstm/networkbuilder.h
    src/lstm/network.h
    src/lstm/networkio.h
    src/lstm/networkscratch.h
    src/lstm/parallel.h
    src/lstm/plumbing.h
    src/lstm/recodebeam.h
    src/lstm/reconfig.h
    src/lstm/reversed.h
    src/lstm/series.h
    src/lstm/static_shape.h
    src/lstm/stridemap.h
    src/lstm/tfnetwork.h
    src/lstm/weightmatrix.h

    #${CMAKE_BINARY_DIR}/src/endianness.h
    DESTINATION include/tesseract)


###############################################################################
