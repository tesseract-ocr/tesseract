# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# This module can find the International Components for Unicode (ICU) Library
#
# Requirements:
# - CMake >= 2.8.3 (for new version of find_package_handle_standard_args)
#
# The following variables will be defined for your use:
#   - ICU_FOUND             : were all of your specified components found (include dependencies)?
#   - ICU_INCLUDE_DIRS      : ICU include directory
#   - ICU_LIBRARIES         : ICU libraries
#   - ICU_VERSION           : complete version of ICU (x.y.z)
#   - ICU_MAJOR_VERSION     : major version of ICU
#   - ICU_MINOR_VERSION     : minor version of ICU
#   - ICU_PATCH_VERSION     : patch version of ICU
#   - ICU_<COMPONENT>_FOUND : were <COMPONENT> found? (FALSE for non specified component if it is not a dependency)
#
# For windows or non standard installation, define ICU_ROOT variable to point to the root installation of ICU. Two ways:
#   - run cmake with -DICU_ROOT=<PATH>
#   - define an environment variable with the same name before running cmake
# With cmake-gui, before pressing "Configure":
#   1) Press "Add Entry" button
#   2) Add a new entry defined as:
#     - Name: ICU_ROOT
#     - Type: choose PATH in the selection list
#     - Press "..." button and select the root installation of ICU
#
# Example Usage:
#
#   1. Copy this file in the root of your project source directory
#   2. Then, tell CMake to search this non-standard module in your project directory by adding to your CMakeLists.txt:
#     set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
#   3. Finally call find_package() once, here are some examples to pick from
#
#   Require ICU 4.4 or later
#     find_package(ICU 4.4 REQUIRED)
#
#   if(ICU_FOUND)
#      include_directories(${ICU_INCLUDE_DIRS})
#      add_executable(myapp myapp.c)
#      target_link_libraries(myapp ${ICU_LIBRARIES})
#   endif(ICU_FOUND)

#=============================================================================
# Copyright (c) 2011-2013, julp
#
# Distributed under the OSI-approved BSD License
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#=============================================================================

find_package(PkgConfig QUIET)

########## Private ##########
if(NOT DEFINED ICU_PUBLIC_VAR_NS)
    set(ICU_PUBLIC_VAR_NS "ICU")                          # Prefix for all ICU relative public variables
endif(NOT DEFINED ICU_PUBLIC_VAR_NS)
if(NOT DEFINED ICU_PRIVATE_VAR_NS)
    set(ICU_PRIVATE_VAR_NS "_${ICU_PUBLIC_VAR_NS}")       # Prefix for all ICU relative internal variables
endif(NOT DEFINED ICU_PRIVATE_VAR_NS)
if(NOT DEFINED PC_ICU_PRIVATE_VAR_NS)
    set(PC_ICU_PRIVATE_VAR_NS "_PC${ICU_PRIVATE_VAR_NS}") # Prefix for all pkg-config relative internal variables
endif(NOT DEFINED PC_ICU_PRIVATE_VAR_NS)

function(icudebug _VARNAME)
    if(${ICU_PUBLIC_VAR_NS}_DEBUG)
        if(DEFINED ${ICU_PUBLIC_VAR_NS}_${_VARNAME})
            message("${ICU_PUBLIC_VAR_NS}_${_VARNAME} = ${${ICU_PUBLIC_VAR_NS}_${_VARNAME}}")
        else(DEFINED ${ICU_PUBLIC_VAR_NS}_${_VARNAME})
            message("${ICU_PUBLIC_VAR_NS}_${_VARNAME} = <UNDEFINED>")
        endif(DEFINED ${ICU_PUBLIC_VAR_NS}_${_VARNAME})
    endif(${ICU_PUBLIC_VAR_NS}_DEBUG)
endfunction(icudebug)

set(${ICU_PRIVATE_VAR_NS}_ROOT "")
if(DEFINED ENV{ICU_ROOT})
    set(${ICU_PRIVATE_VAR_NS}_ROOT "$ENV{ICU_ROOT}")
endif(DEFINED ENV{ICU_ROOT})
if (DEFINED ICU_ROOT)
    set(${ICU_PRIVATE_VAR_NS}_ROOT "${ICU_ROOT}")
endif(DEFINED ICU_ROOT)

set(${ICU_PRIVATE_VAR_NS}_BIN_SUFFIXES )
set(${ICU_PRIVATE_VAR_NS}_LIB_SUFFIXES )
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    list(APPEND ${ICU_PRIVATE_VAR_NS}_BIN_SUFFIXES "bin64")
    list(APPEND ${ICU_PRIVATE_VAR_NS}_LIB_SUFFIXES "lib64")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
list(APPEND ${ICU_PRIVATE_VAR_NS}_BIN_SUFFIXES "bin")
list(APPEND ${ICU_PRIVATE_VAR_NS}_LIB_SUFFIXES "lib")

set(${ICU_PRIVATE_VAR_NS}_COMPONENTS )
# <icu component name> <library name 1> ... <library name N>
macro(icu_declare_component _NAME)
    list(APPEND ${ICU_PRIVATE_VAR_NS}_COMPONENTS ${_NAME})
    set("${ICU_PRIVATE_VAR_NS}_COMPONENTS_${_NAME}" ${ARGN})
endmacro(icu_declare_component)

icu_declare_component(data icudata)
icu_declare_component(uc   icuuc)         # Common and Data libraries
icu_declare_component(i18n icui18n icuin) # Internationalization library
icu_declare_component(io   icuio ustdio)  # Stream and I/O Library
icu_declare_component(le   icule)         # Layout library
icu_declare_component(lx   iculx)         # Paragraph Layout library

########## Public ##########
set(${ICU_PUBLIC_VAR_NS}_FOUND TRUE)
set(${ICU_PUBLIC_VAR_NS}_LIBRARIES )
set(${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS )
set(${ICU_PUBLIC_VAR_NS}_C_FLAGS "")
set(${ICU_PUBLIC_VAR_NS}_CXX_FLAGS "")
set(${ICU_PUBLIC_VAR_NS}_CPP_FLAGS "")
set(${ICU_PUBLIC_VAR_NS}_C_SHARED_FLAGS "")
set(${ICU_PUBLIC_VAR_NS}_CXX_SHARED_FLAGS "")
set(${ICU_PUBLIC_VAR_NS}_CPP_SHARED_FLAGS "")
foreach(${ICU_PRIVATE_VAR_NS}_COMPONENT ${${ICU_PRIVATE_VAR_NS}_COMPONENTS})
    string(TOUPPER "${${ICU_PRIVATE_VAR_NS}_COMPONENT}" ${ICU_PRIVATE_VAR_NS}_UPPER_COMPONENT)
    set("${ICU_PUBLIC_VAR_NS}_${${ICU_PRIVATE_VAR_NS}_UPPER_COMPONENT}_FOUND" FALSE) # may be done in the icu_declare_component macro
endforeach(${ICU_PRIVATE_VAR_NS}_COMPONENT)

# Check components
if(NOT ${ICU_PUBLIC_VAR_NS}_FIND_COMPONENTS) # uc required at least
    set(${ICU_PUBLIC_VAR_NS}_FIND_COMPONENTS uc)
else(NOT ${ICU_PUBLIC_VAR_NS}_FIND_COMPONENTS)
    list(APPEND ${ICU_PUBLIC_VAR_NS}_FIND_COMPONENTS uc)
    list(REMOVE_DUPLICATES ${ICU_PUBLIC_VAR_NS}_FIND_COMPONENTS)
    foreach(${ICU_PRIVATE_VAR_NS}_COMPONENT ${${ICU_PUBLIC_VAR_NS}_FIND_COMPONENTS})
        if(NOT DEFINED ${ICU_PRIVATE_VAR_NS}_COMPONENTS_${${ICU_PRIVATE_VAR_NS}_COMPONENT})
            message(FATAL_ERROR "Unknown ICU component: ${${ICU_PRIVATE_VAR_NS}_COMPONENT}")
        endif(NOT DEFINED ${ICU_PRIVATE_VAR_NS}_COMPONENTS_${${ICU_PRIVATE_VAR_NS}_COMPONENT})
    endforeach(${ICU_PRIVATE_VAR_NS}_COMPONENT)
endif(NOT ${ICU_PUBLIC_VAR_NS}_FIND_COMPONENTS)

# Includes
find_path(
    ${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS
    NAMES unicode/utypes.h utypes.h
    HINTS ${${ICU_PRIVATE_VAR_NS}_ROOT}
    PATH_SUFFIXES "include"
    DOC "Include directories for ICU"
)

if(${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS)
    ########## <part to keep synced with tests/version/CMakeLists.txt> ##########
    if(EXISTS "${${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS}/unicode/uvernum.h") # ICU >= 4
        file(READ "${${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS}/unicode/uvernum.h" ${ICU_PRIVATE_VAR_NS}_VERSION_HEADER_CONTENTS)
    elseif(EXISTS "${${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS}/unicode/uversion.h") # ICU [2;4[
        file(READ "${${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS}/unicode/uversion.h" ${ICU_PRIVATE_VAR_NS}_VERSION_HEADER_CONTENTS)
    elseif(EXISTS "${${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS}/unicode/utypes.h") # ICU [1.4;2[
        file(READ "${${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS}/unicode/utypes.h" ${ICU_PRIVATE_VAR_NS}_VERSION_HEADER_CONTENTS)
    elseif(EXISTS "${${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS}/utypes.h") # ICU 1.3
        file(READ "${${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS}/utypes.h" ${ICU_PRIVATE_VAR_NS}_VERSION_HEADER_CONTENTS)
    else()
        message(FATAL_ERROR "ICU version header not found")
    endif()

    if(${ICU_PRIVATE_VAR_NS}_VERSION_HEADER_CONTENTS MATCHES ".*# *define *ICU_VERSION *\"([0-9]+)\".*") # ICU 1.3
        # [1.3;1.4[ as #define ICU_VERSION "3" (no patch version, ie all 1.3.X versions will be detected as 1.3.0)
        set(${ICU_PUBLIC_VAR_NS}_MAJOR_VERSION "1")
        set(${ICU_PUBLIC_VAR_NS}_MINOR_VERSION "${CMAKE_MATCH_1}")
        set(${ICU_PUBLIC_VAR_NS}_PATCH_VERSION "0")
    elseif(${ICU_PRIVATE_VAR_NS}_VERSION_HEADER_CONTENTS MATCHES ".*# *define *U_ICU_VERSION_MAJOR_NUM *([0-9]+).*")
        #
        # Since version 4.9.1, ICU release version numbering was totaly changed, see:
        # - http://site.icu-project.org/download/49
        # - http://userguide.icu-project.org/design#TOC-Version-Numbers-in-ICU
        #
        set(${ICU_PUBLIC_VAR_NS}_MAJOR_VERSION "${CMAKE_MATCH_1}")
        string(REGEX REPLACE ".*# *define *U_ICU_VERSION_MINOR_NUM *([0-9]+).*" "\\1" ${ICU_PUBLIC_VAR_NS}_MINOR_VERSION "${${ICU_PRIVATE_VAR_NS}_VERSION_HEADER_CONTENTS}")
        string(REGEX REPLACE ".*# *define *U_ICU_VERSION_PATCHLEVEL_NUM *([0-9]+).*" "\\1" ${ICU_PUBLIC_VAR_NS}_PATCH_VERSION "${${ICU_PRIVATE_VAR_NS}_VERSION_HEADER_CONTENTS}")
    elseif(${ICU_PRIVATE_VAR_NS}_VERSION_HEADER_CONTENTS MATCHES ".*# *define *U_ICU_VERSION *\"(([0-9]+)(\\.[0-9]+)*)\".*") # ICU [1.4;1.8[
        # [1.4;1.8[ as #define U_ICU_VERSION "1.4.1.2" but it seems that some 1.4.1(?:\.\d)? have releasing error and appears as 1.4.0
        set(${ICU_PRIVATE_VAR_NS}_FULL_VERSION "${CMAKE_MATCH_1}") # copy CMAKE_MATCH_1, no longer valid on the following if
        if(${ICU_PRIVATE_VAR_NS}_FULL_VERSION MATCHES "^([0-9]+)\\.([0-9]+)$")
            set(${ICU_PUBLIC_VAR_NS}_MAJOR_VERSION "${CMAKE_MATCH_1}")
            set(${ICU_PUBLIC_VAR_NS}_MINOR_VERSION "${CMAKE_MATCH_2}")
            set(${ICU_PUBLIC_VAR_NS}_PATCH_VERSION "0")
        elseif(${ICU_PRIVATE_VAR_NS}_FULL_VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)")
            set(${ICU_PUBLIC_VAR_NS}_MAJOR_VERSION "${CMAKE_MATCH_1}")
            set(${ICU_PUBLIC_VAR_NS}_MINOR_VERSION "${CMAKE_MATCH_2}")
            set(${ICU_PUBLIC_VAR_NS}_PATCH_VERSION "${CMAKE_MATCH_3}")
        endif()
    else()
        message(FATAL_ERROR "failed to detect ICU version")
    endif()
    set(${ICU_PUBLIC_VAR_NS}_VERSION "${${ICU_PUBLIC_VAR_NS}_MAJOR_VERSION}.${${ICU_PUBLIC_VAR_NS}_MINOR_VERSION}.${${ICU_PUBLIC_VAR_NS}_PATCH_VERSION}")
    ########## </part to keep synced with tests/version/CMakeLists.txt> ##########

    # Check dependencies (implies pkg-config)
    if(PKG_CONFIG_FOUND)
        set(${ICU_PRIVATE_VAR_NS}_COMPONENTS_DUP ${${ICU_PUBLIC_VAR_NS}_FIND_COMPONENTS})
        foreach(${ICU_PRIVATE_VAR_NS}_COMPONENT ${${ICU_PRIVATE_VAR_NS}_COMPONENTS_DUP})
            pkg_check_modules(PC_ICU_PRIVATE_VAR_NS "icu-${${ICU_PRIVATE_VAR_NS}_COMPONENT}" QUIET)

            if(${PC_ICU_PRIVATE_VAR_NS}_FOUND)
                foreach(${PC_ICU_PRIVATE_VAR_NS}_LIBRARY ${PC_ICU_LIBRARIES})
                    string(REGEX REPLACE "^icu" "" ${PC_ICU_PRIVATE_VAR_NS}_STRIPPED_LIBRARY ${${PC_ICU_PRIVATE_VAR_NS}_LIBRARY})
                    list(APPEND ${ICU_PUBLIC_VAR_NS}_FIND_COMPONENTS ${${PC_ICU_PRIVATE_VAR_NS}_STRIPPED_LIBRARY})
                endforeach(${PC_ICU_PRIVATE_VAR_NS}_LIBRARY)
            endif(${PC_ICU_PRIVATE_VAR_NS}_FOUND)
        endforeach(${ICU_PRIVATE_VAR_NS}_COMPONENT)
        list(REMOVE_DUPLICATES ${ICU_PUBLIC_VAR_NS}_FIND_COMPONENTS)
    endif(PKG_CONFIG_FOUND)

    # Check libraries
    foreach(${ICU_PRIVATE_VAR_NS}_COMPONENT ${${ICU_PUBLIC_VAR_NS}_FIND_COMPONENTS})
        set(${ICU_PRIVATE_VAR_NS}_POSSIBLE_RELEASE_NAMES )
        set(${ICU_PRIVATE_VAR_NS}_POSSIBLE_DEBUG_NAMES )
        foreach(${ICU_PRIVATE_VAR_NS}_BASE_NAME ${${ICU_PRIVATE_VAR_NS}_COMPONENTS_${${ICU_PRIVATE_VAR_NS}_COMPONENT}})
            list(APPEND ${ICU_PRIVATE_VAR_NS}_POSSIBLE_RELEASE_NAMES "${${ICU_PRIVATE_VAR_NS}_BASE_NAME}")
            list(APPEND ${ICU_PRIVATE_VAR_NS}_POSSIBLE_DEBUG_NAMES "${${ICU_PRIVATE_VAR_NS}_BASE_NAME}d")
            list(APPEND ${ICU_PRIVATE_VAR_NS}_POSSIBLE_RELEASE_NAMES "${${ICU_PRIVATE_VAR_NS}_BASE_NAME}${ICU_MAJOR_VERSION}${ICU_MINOR_VERSION}")
            list(APPEND ${ICU_PRIVATE_VAR_NS}_POSSIBLE_DEBUG_NAMES "${${ICU_PRIVATE_VAR_NS}_BASE_NAME}${ICU_MAJOR_VERSION}${ICU_MINOR_VERSION}d")
        endforeach(${ICU_PRIVATE_VAR_NS}_BASE_NAME)

        find_library(
            ${ICU_PRIVATE_VAR_NS}_LIB_RELEASE_${${ICU_PRIVATE_VAR_NS}_COMPONENT}
            NAMES ${${ICU_PRIVATE_VAR_NS}_POSSIBLE_RELEASE_NAMES}
            HINTS ${${ICU_PRIVATE_VAR_NS}_ROOT}
            PATH_SUFFIXES ${_ICU_LIB_SUFFIXES}
            DOC "Release libraries for ICU"
        )
        find_library(
            ${ICU_PRIVATE_VAR_NS}_LIB_DEBUG_${${ICU_PRIVATE_VAR_NS}_COMPONENT}
            NAMES ${${ICU_PRIVATE_VAR_NS}_POSSIBLE_DEBUG_NAMES}
            HINTS ${${ICU_PRIVATE_VAR_NS}_ROOT}
            PATH_SUFFIXES ${_ICU_LIB_SUFFIXES}
            DOC "Debug libraries for ICU"
        )

        string(TOUPPER "${${ICU_PRIVATE_VAR_NS}_COMPONENT}" ${ICU_PRIVATE_VAR_NS}_UPPER_COMPONENT)
        if(NOT ${ICU_PRIVATE_VAR_NS}_LIB_RELEASE_${${ICU_PRIVATE_VAR_NS}_COMPONENT} AND NOT ${ICU_PRIVATE_VAR_NS}_LIB_DEBUG_${${ICU_PRIVATE_VAR_NS}_COMPONENT}) # both not found
            set("${ICU_PUBLIC_VAR_NS}_${${ICU_PRIVATE_VAR_NS}_UPPER_COMPONENT}_FOUND" FALSE)
            set("${ICU_PUBLIC_VAR_NS}_FOUND" FALSE)
        else(NOT ${ICU_PRIVATE_VAR_NS}_LIB_RELEASE_${${ICU_PRIVATE_VAR_NS}_COMPONENT} AND NOT ${ICU_PRIVATE_VAR_NS}_LIB_DEBUG_${${ICU_PRIVATE_VAR_NS}_COMPONENT}) # one or both found
            set("${ICU_PUBLIC_VAR_NS}_${${ICU_PRIVATE_VAR_NS}_UPPER_COMPONENT}_FOUND" TRUE)
            if(NOT ${ICU_PRIVATE_VAR_NS}_LIB_RELEASE_${${ICU_PRIVATE_VAR_NS}_COMPONENT}) # release not found => we are in debug
                set(${ICU_PRIVATE_VAR_NS}_LIB_${${ICU_PRIVATE_VAR_NS}_COMPONENT} "${${ICU_PRIVATE_VAR_NS}_LIB_DEBUG_${${ICU_PRIVATE_VAR_NS}_COMPONENT}}")
            elseif(NOT ${ICU_PRIVATE_VAR_NS}_LIB_DEBUG_${${ICU_PRIVATE_VAR_NS}_COMPONENT}) # debug not found => we are in release
                set(${ICU_PRIVATE_VAR_NS}_LIB_${${ICU_PRIVATE_VAR_NS}_COMPONENT} "${${ICU_PRIVATE_VAR_NS}_LIB_RELEASE_${${ICU_PRIVATE_VAR_NS}_COMPONENT}}")
            else() # both found
                set(
                    ${ICU_PRIVATE_VAR_NS}_LIB_${${ICU_PRIVATE_VAR_NS}_COMPONENT}
                    optimized ${${ICU_PRIVATE_VAR_NS}_LIB_RELEASE_${${ICU_PRIVATE_VAR_NS}_COMPONENT}}
                    debug ${${ICU_PRIVATE_VAR_NS}_LIB_DEBUG_${${ICU_PRIVATE_VAR_NS}_COMPONENT}}
                )
            endif()
            list(APPEND ${ICU_PUBLIC_VAR_NS}_LIBRARIES ${${ICU_PRIVATE_VAR_NS}_LIB_${${ICU_PRIVATE_VAR_NS}_COMPONENT}})
        endif(NOT ${ICU_PRIVATE_VAR_NS}_LIB_RELEASE_${${ICU_PRIVATE_VAR_NS}_COMPONENT} AND NOT ${ICU_PRIVATE_VAR_NS}_LIB_DEBUG_${${ICU_PRIVATE_VAR_NS}_COMPONENT})
    endforeach(${ICU_PRIVATE_VAR_NS}_COMPONENT)

    # Try to find out compiler flags
    find_program(${ICU_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE icu-config HINTS ${${ICU_PRIVATE_VAR_NS}_ROOT})
    if(${ICU_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE)
        execute_process(COMMAND ${${ICU_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE} --cflags OUTPUT_VARIABLE ${ICU_PUBLIC_VAR_NS}_C_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${${ICU_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE} --cxxflags OUTPUT_VARIABLE ${ICU_PUBLIC_VAR_NS}_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${${ICU_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE} --cppflags OUTPUT_VARIABLE ${ICU_PUBLIC_VAR_NS}_CPP_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)

        execute_process(COMMAND ${${ICU_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE} --cflags-dynamic OUTPUT_VARIABLE ${ICU_PUBLIC_VAR_NS}_C_SHARED_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${${ICU_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE} --cxxflags-dynamic OUTPUT_VARIABLE ${ICU_PUBLIC_VAR_NS}_CXX_SHARED_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${${ICU_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE} --cppflags-dynamic OUTPUT_VARIABLE ${ICU_PUBLIC_VAR_NS}_CPP_SHARED_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif(${ICU_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE)

    # Check find_package arguments
    include(FindPackageHandleStandardArgs)
    if(${ICU_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${ICU_PUBLIC_VAR_NS}_FIND_QUIETLY)
        find_package_handle_standard_args(
            ${ICU_PUBLIC_VAR_NS}
            REQUIRED_VARS ${ICU_PUBLIC_VAR_NS}_LIBRARIES ${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS
            VERSION_VAR ${ICU_PUBLIC_VAR_NS}_VERSION
        )
    else(${ICU_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${ICU_PUBLIC_VAR_NS}_FIND_QUIETLY)
        find_package_handle_standard_args(${ICU_PUBLIC_VAR_NS} "ICU not found" ${ICU_PUBLIC_VAR_NS}_LIBRARIES ${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS)
    endif(${ICU_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${ICU_PUBLIC_VAR_NS}_FIND_QUIETLY)
else(${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS)
    set("${ICU_PUBLIC_VAR_NS}_FOUND" FALSE)
    if(${ICU_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${ICU_PUBLIC_VAR_NS}_FIND_QUIETLY)
        message(FATAL_ERROR "Could not find ICU include directory")
    endif(${ICU_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${ICU_PUBLIC_VAR_NS}_FIND_QUIETLY)
endif(${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS)

mark_as_advanced(
    ${ICU_PUBLIC_VAR_NS}_INCLUDE_DIRS
    ${ICU_PUBLIC_VAR_NS}_LIBRARIES
)

# IN (args)
icudebug("FIND_COMPONENTS")
icudebug("FIND_REQUIRED")
icudebug("FIND_QUIETLY")
icudebug("FIND_VERSION")
# OUT
# Found
icudebug("FOUND")
icudebug("UC_FOUND")
icudebug("I18N_FOUND")
icudebug("IO_FOUND")
icudebug("LE_FOUND")
icudebug("LX_FOUND")
icudebug("DATA_FOUND")
# Flags
icudebug("C_FLAGS")
icudebug("CPP_FLAGS")
icudebug("CXX_FLAGS")
icudebug("C_SHARED_FLAGS")
icudebug("CPP_SHARED_FLAGS")
icudebug("CXX_SHARED_FLAGS")
# Linking
icudebug("INCLUDE_DIRS")
icudebug("LIBRARIES")
# Version
icudebug("MAJOR_VERSION")
icudebug("MINOR_VERSION")
icudebug("PATCH_VERSION")
icudebug("VERSION")
