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
    limits.h
    malloc.h
    memory.h
    stdbool.h
    stdint.h
    stdlib.h
    strings.h
    string.h
    sys/ipc.h
    sys/shm.h
    sys/stat.h
    sys/types.h
    sys/wait.h
    tiffio.h
    unistd.h
    
    cairo/cairo-version.h
    CL/cl.h
    OpenCL/cl.h
    pango-1.0/pango/pango-features.h
    unicode/uchar.h
)
check_includes(include_files_list)

set(functions_list
    getline
    snprintf
)
check_functions(functions_list)

set(types_list
    "long long int"
    off_t
    mbstate_t
    wchar_t
    _Bool
)
check_types(types_list)

check_c_source_compiles("#include <sys/time.h>\n#include <time.h>\nmain(){}" TIME_WITH_SYS_TIME)

test_big_endian(WORDS_BIGENDIAN)

set(STDC_HEADERS 1)

file(APPEND ${AUTOCONFIG_SRC} "
/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS 1

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#cmakedefine WORDS_BIGENDIAN 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#cmakedefine TIME_WITH_SYS_TIME 1
")

########################################

################################################################################
