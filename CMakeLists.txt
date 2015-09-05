#
# tesseract
#

###############################################################################
#
# cmake settings
#
###############################################################################

cmake_minimum_required(VERSION 2.8.12)

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

find_package(Leptonica 1.72 REQUIRED)

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
        add_definitions(-D_SCL_SECURE_NO_WARNINGS)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)        
        
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
    endif()
endif()

###############################################################################
#
# configure
#
###############################################################################

set(AUTOCONFIG_SRC ${CMAKE_BINARY_DIR}/config_auto.h.in)
set(AUTOCONFIG ${CMAKE_BINARY_DIR}/src/config_auto.h)

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

########################################
# FUNCTION build_dir
########################################
function(build_dir target_name)
    if (${ARGC} GREATER 1)
        set(dir ${ARGV1})
    else()
        set(dir ${target_name})
    endif()
    file(GLOB ${dir}_src "${dir}/*.cpp")
    file(GLOB ${dir}_hdr "${dir}/*.h")
    add_library(${target_name} ${${dir}_src} ${${dir}_hdr})
endfunction(build_dir)
########################################

add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1)
add_definitions(-DUSE_STD_NAMESPACE=1)
add_definitions(-DWINDLLNAME="libtesseract${VERSION_MAJOR}${VERSION_MINOR}.dll")
add_definitions(-DTESS_EXPORTS)

include_directories(${Leptonica_INCLUDE_DIRS})

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
# LIBRARY api
########################################

set(api_src
    api/baseapi.cpp
    api/capi.cpp
    api/renderer.cpp
    api/pdfrenderer.cpp
)
file(GLOB api_hdr "api/*.h")
add_library(api ${api_src} ${api_hdr})


########################################

########################################
# LIBRARIES tesseract
########################################

build_dir(main ccmain)
build_dir(struct ccstruct)
build_dir(ccutil)
build_dir(classify)
build_dir(cube)
build_dir(cutil)
build_dir(dict)
build_dir(neural neural_networks/runtime)
build_dir(opencl)
build_dir(textord)
build_dir(viewer)
build_dir(port vs2010/port)
build_dir(wordrec)


########################################
# LIBRARY tesseract
########################################

add_library                     (tesseract ${LIBRARY_TYPE} ${tesseract_src} ${tesseract_hdr})
target_link_libraries           (tesseract
    PRIVATE
        main
        struct
        ccutil
        classify
        cube
        cutil
        dict
        neural
        opencl
        textord
        viewer
        port
        wordrec

    PUBLIC
        ${Leptonica_LIBRARIES}
        Ws2_32
)
set_target_properties           (tesseract PROPERTIES OUTPUT_NAME libtesseract${VERSION_MAJOR}${VERSION_MINOR})
set_target_properties           (tesseract PROPERTIES DEBUG_OUTPUT_NAME libtesseract${VERSION_MAJOR}${VERSION_MINOR}d)
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

###############################################################################
