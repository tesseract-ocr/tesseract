#
# Find Leptonica
#
# Exported variables:
#    Leptonica_FOUND
#    Leptonica_INCLUDE_DIRS
#    Leptonica_LIBRARIES
#
#    Leptonica_VERSION
#    Leptonica_MAJOR_VERSION
#    Leptonica_MINOR_VERSION
#

find_path(Leptonica_INCLUDE_DIR leptonica/allheaders.h
    HINTS
    /usr/include
    /usr/local/include
    /opt/include
    /opt/local/include
)
if(NOT Leptonica_INCLUDE_DIR-NOTFOUND)
    set(Leptonica_INCLUDE_DIRS ${Leptonica_INCLUDE_DIR}/leptonica)
    file(STRINGS ${Leptonica_INCLUDE_DIRS}/allheaders.h Leptonica_MAJOR_VERSION REGEX "LIBLEPT_MAJOR_VERSION")
    file(STRINGS ${Leptonica_INCLUDE_DIRS}/allheaders.h Leptonica_MINOR_VERSION REGEX "LIBLEPT_MINOR_VERSION")
    string(REGEX MATCH "[0-9]+" Leptonica_MAJOR_VERSION ${Leptonica_MAJOR_VERSION})
    string(REGEX MATCH "[0-9]+" Leptonica_MINOR_VERSION ${Leptonica_MINOR_VERSION})
    set(Leptonica_VERSION ${Leptonica_MAJOR_VERSION}.${Leptonica_MINOR_VERSION})
endif()

find_library(Leptonica_LIBRARY NAMES lept liblept
    HINTS
    /usr/lib
    /usr/local/lib
    /opt/lib
    /opt/local/lib
)
set(Leptonica_LIBRARIES ${Leptonica_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Leptonica
    REQUIRED_VARS
        Leptonica_INCLUDE_DIRS
        Leptonica_LIBRARIES
    VERSION_VAR Leptonica_VERSION
)

mark_as_advanced(Leptonica_INCLUDE_DIRS Leptonica_LIBRARIES)

