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

set(VERSION_MAJOR 3)
set(VERSION_MINOR 05)
set(VERSION_PLAIN ${VERSION_MAJOR}.${VERSION_MINOR})

set(MINIMUM_LEPTONICA_VERSION 1.71)

if (NOT Leptonica_BUILD_DIR)
find_package(Leptonica ${MINIMUM_LEPTONICA_VERSION} REQUIRED)
else()
set(Leptonica_DIR ${Leptonica_BUILD_DIR})
find_package(Leptonica ${MINIMUM_LEPTONICA_VERSION} REQUIRED CONFIG)
endif()

find_package(OpenCL QUIET)
find_package(PkgConfig)

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
        
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
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
# configure
#
###############################################################################

set(AUTOCONFIG_SRC ${CMAKE_BINARY_DIR}/config_auto.h.in)
set(AUTOCONFIG ${CMAKE_BINARY_DIR}/config_auto.h)

include(Configure)

configure_file(${AUTOCONFIG_SRC} ${AUTOCONFIG} @ONLY)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/api)

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
add_definitions(-DUSE_STD_NAMESPACE=1)
add_definitions(-DWINDLLNAME="libtesseract${VERSION_MAJOR}${VERSION_MINOR}.dll")

include_directories(${Leptonica_INCLUDE_DIRS})

include_directories(${CMAKE_BINARY_DIR})

include_directories(api)
include_directories(ccmain)
include_directories(ccstruct)
include_directories(ccutil)
include_directories(classify)
include_directories(cube)
include_directories(cutil)
include_directories(dict)
include_directories(neural_networks/runtime)
include_directories(opencl)
include_directories(textord)
include_directories(vs2010/port)
include_directories(viewer)
include_directories(wordrec)

########################################
# LIBRARY tesseract
########################################

string(SUBSTRING ${VERSION_MINOR} 0 1 VERSION_MINOR_0)
string(SUBSTRING ${VERSION_MINOR} 1 1 VERSION_MINOR_1)

file(GLOB tesseract_src
    "ccmain/*.cpp"
    "ccstruct/*.cpp"
    "ccutil/*.cpp"
    "classify/*.cpp"
    "cube/*.cpp"
    "cutil/*.cpp"
    "dict/*.cpp"
    "neural_networks/runtime/*.cpp"
    "opencl/*.cpp"
    "textord/*.cpp"
    "viewer/*.cpp"
    "wordrec/*.cpp"
)
file(GLOB tesseract_hdr
    "api/*.h"
    "ccmain/*.h"
    "ccstruct/*.h"
    "ccutil/*.h"
    "classify/*.h"
    "cube/*.h"
    "cutil/*.h"
    "dict/*.h"
    "neural_networks/runtime/*.h"
    "opencl/*.h"
    "textord/*.h"
    "viewer/*.h"
    "wordrec/*.h"
)
if (WIN32)
    file(GLOB tesseract_win32_src "vs2010/port/*.cpp")
    file(GLOB tesseract_win32_hdr "vs2010/port/*.h")
    set(tesseract_src ${tesseract_src} ${tesseract_win32_src})
    set(tesseract_hdr ${tesseract_hdr} ${tesseract_win32_hdr})
endif()

set(tesseract_src ${tesseract_src}
    api/baseapi.cpp
    api/capi.cpp
    api/renderer.cpp
    api/pdfrenderer.cpp
)

add_library                     (tesseract ${LIBRARY_TYPE} ${tesseract_src} ${tesseract_hdr})
if (NOT STATIC)
target_compile_definitions      (tesseract PUBLIC -DTESS_EXPORTS)
endif()
target_link_libraries           (tesseract ${Leptonica_LIBRARIES} ${LIB_Ws2_32} ${LIB_pthread})
set_target_properties           (tesseract PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR_0}.${VERSION_MINOR_1})
set_target_properties           (tesseract PROPERTIES SOVERSION ${VERSION_MAJOR}.${VERSION_MINOR_0}.${VERSION_MINOR_1})
if (WIN32)
set_target_properties           (tesseract PROPERTIES OUTPUT_NAME tesseract${VERSION_MAJOR}${VERSION_MINOR})
set_target_properties           (tesseract PROPERTIES DEBUG_OUTPUT_NAME tesseract${VERSION_MAJOR}${VERSION_MINOR}d)
endif()
export(TARGETS tesseract FILE ${CMAKE_BINARY_DIR}/TesseractTargets.cmake)


########################################
# EXECUTABLE tesseractmain
########################################

set(tesseractmain_src
    api/tesseractmain.cpp
    vs2010/tesseract/resource.h
    vs2010/tesseract/tesseract.rc
)
add_executable                  (tesseractmain ${tesseractmain_src})
target_link_libraries           (tesseractmain tesseract)
set_target_properties           (tesseractmain PROPERTIES OUTPUT_NAME tesseract)

########################################

add_subdirectory(training)

###############################################################################
