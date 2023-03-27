# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################
#
# configure
#
################################################################################

########################################
# FUNCTION check_includes
########################################
function(check_includes files)
    foreach(F ${${files}})
        set(name ${F})
        string(REPLACE "-" "_" name ${name})
        string(REPLACE "." "_" name ${name})
        string(REPLACE "/" "_" name ${name})
        string(TOUPPER ${name} name)
        check_include_files(${F} HAVE_${name})
        file(APPEND ${AUTOCONFIG_SRC} "/* Define to 1 if you have the <${F}> header file. */\n")
        file(APPEND ${AUTOCONFIG_SRC} "#cmakedefine HAVE_${name} 1\n")
        file(APPEND ${AUTOCONFIG_SRC} "\n")
    endforeach()
endfunction(check_includes)

########################################
# FUNCTION check_functions
########################################
function(check_functions functions)
    foreach(F ${${functions}})
        set(name ${F})
        string(TOUPPER ${name} name)
        check_function_exists(${F} HAVE_${name})
        file(APPEND ${AUTOCONFIG_SRC} "/* Define to 1 if you have the `${F}' function. */\n")
        file(APPEND ${AUTOCONFIG_SRC} "#cmakedefine HAVE_${name} 1\n")
        file(APPEND ${AUTOCONFIG_SRC} "\n")
    endforeach()
endfunction(check_functions)

########################################
# FUNCTION check_types
########################################
function(check_types types)
    foreach(T ${${types}})
        set(name ${T})
        string(REPLACE " " "_" name ${name})
        string(REPLACE "-" "_" name ${name})
        string(REPLACE "." "_" name ${name})
        string(REPLACE "/" "_" name ${name})
        string(TOUPPER ${name} name)
        check_type_size(${T} HAVE_${name})
        file(APPEND ${AUTOCONFIG_SRC} "/* Define to 1 if the system has the type `${T}'. */\n")
        file(APPEND ${AUTOCONFIG_SRC} "#cmakedefine HAVE_${name} 1\n")
        file(APPEND ${AUTOCONFIG_SRC} "\n")
    endforeach()
endfunction(check_types)

########################################

file(WRITE ${AUTOCONFIG_SRC})

include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckPrototypeDefinition)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(CheckTypeSize)
include(TestBigEndian)

set(include_files_list
    dlfcn.h
    inttypes.h
    memory.h
    stdint.h
    stdlib.h
    string.h
    sys/stat.h
    sys/types.h
    unistd.h

    cairo/cairo-version.h
    CL/cl.h
    OpenCL/cl.h
    pango-1.0/pango/pango-features.h
    unicode/uchar.h
)
# check_includes(include_files_list)

set(types_list
    "long long int"
    wchar_t
)
# check_types(types_list)

list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
list(APPEND CMAKE_REQUIRED_LIBRARIES -lm)
set(functions_list
    feenableexcept
)
check_functions(functions_list)

file(APPEND ${AUTOCONFIG_SRC} "
/* Version number */
#cmakedefine PACKAGE_VERSION \"${PACKAGE_VERSION}\"
#cmakedefine GRAPHICS_DISABLED ${GRAPHICS_DISABLED}
#cmakedefine FAST_FLOAT ${FAST_FLOAT}
#cmakedefine DISABLED_LEGACY_ENGINE ${DISABLED_LEGACY_ENGINE}
#cmakedefine HAVE_TIFFIO_H ${HAVE_TIFFIO_H}
#cmakedefine HAVE_NEON ${HAVE_NEON}
#cmakedefine HAVE_LIBARCHIVE ${HAVE_LIBARCHIVE}
#cmakedefine HAVE_LIBCURL ${HAVE_LIBCURL}
#cmakedefine USE_OPENCL ${USE_OPENCL}
")

if(TESSDATA_PREFIX)
 file(APPEND ${AUTOCONFIG_SRC} "
#cmakedefine TESSDATA_PREFIX \"${TESSDATA_PREFIX}\"
")
endif()

########################################

################################################################################
