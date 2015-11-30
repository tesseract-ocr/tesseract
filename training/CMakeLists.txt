#
# tesseract
#

if (STATIC OR NOT (WIN32 OR CYGWIN))

# experimental
if (MSVC)

include(CheckTypeSize)
check_type_size("void *" SIZEOF_VOID_P)

if (SIZEOF_VOID_P EQUAL 8)
    set(X64 1)
    set(ARCH_DIR_NAME 64)
elseif (SIZEOF_VOID_P EQUAL 4)
    set(X86 1)
    set(ARCH_DIR_NAME 32)
else()
    message(FATAL_ERROR "Cannot determine target architecture")
endif()

set(icu_dir "${CMAKE_CURRENT_BINARY_DIR}/icu")
set(icu_archive "${icu_dir}/icu${ARCH_DIR_NAME}.zip")

if (X86)
    set(icu_hash 45167a240b60e36b59a87eda23490ce4)
else()
    set(icu_hash 480c72491576c048de1218c3c5519399)
endif()

message(STATUS "Downloading latest ICU binaries")

file(DOWNLOAD
    "http://download.icu-project.org/files/icu4c/56.1/icu4c-56_1-Win${ARCH_DIR_NAME}-msvc10.zip"
    "${icu_archive}"
    SHOW_PROGRESS
    EXPECTED_HASH MD5=${icu_hash}
)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xz "${icu_archive}"
    WORKING_DIRECTORY "${icu_dir}"
    RESULT_VARIABLE __result
)
if(NOT __result EQUAL 0)
    message(FATAL_ERROR "error ${__result}")
endif()

set(ICU_ROOT ${icu_dir}/icu)

endif(MSVC)
# experimental

find_package(ICU COMPONENTS uc i18n)

########################################
# LIBRARY tessopt
########################################

add_library                 (tessopt tessopt.cpp tessopt.h)
project_group               (tessopt "Training Tools")


########################################
# LIBRARY common_training
########################################

set(common_training_src
    commandlineflags.cpp
    commontraining.cpp
)
set(common_training_hdr
    commandlineflags.h
    commontraining.h
)
add_library                 (common_training ${common_training_src} ${common_training_hdr})
target_link_libraries       (common_training tesseract tessopt)
project_group               (common_training "Training Tools")


########################################
# EXECUTABLE ambiguous_words
########################################

add_executable              (ambiguous_words ambiguous_words.cpp)
target_link_libraries       (ambiguous_words tesseract)
project_group               (ambiguous_words "Training Tools")


########################################
# EXECUTABLE classifier_tester
########################################

add_executable              (classifier_tester classifier_tester.cpp)
target_link_libraries       (classifier_tester common_training)
project_group               (classifier_tester "Training Tools")


########################################
# EXECUTABLE combine_tessdata
########################################

add_executable              (combine_tessdata combine_tessdata.cpp)
target_link_libraries       (combine_tessdata tesseract)
project_group               (combine_tessdata "Training Tools")


########################################
# EXECUTABLE cntraining
########################################

add_executable              (cntraining cntraining.cpp)
target_link_libraries       (cntraining common_training)
project_group               (cntraining "Training Tools")


########################################
# EXECUTABLE dawg2wordlist
########################################

add_executable              (dawg2wordlist dawg2wordlist.cpp)
target_link_libraries       (dawg2wordlist tesseract)
project_group               (dawg2wordlist "Training Tools")


########################################
# EXECUTABLE mftraining
########################################

add_executable              (mftraining mftraining.cpp mergenf.cpp mergenf.h)
target_link_libraries       (mftraining common_training)
project_group               (mftraining "Training Tools")


########################################
# EXECUTABLE shapeclustering
########################################

add_executable              (shapeclustering shapeclustering.cpp)
target_link_libraries       (shapeclustering common_training)
project_group               (shapeclustering "Training Tools")


########################################
# EXECUTABLE unicharset_extractor
########################################

add_executable              (unicharset_extractor unicharset_extractor.cpp)
target_link_libraries       (unicharset_extractor tesseract tessopt)
project_group               (unicharset_extractor "Training Tools")


########################################
# EXECUTABLE wordlist2dawg
########################################

add_executable              (wordlist2dawg wordlist2dawg.cpp)
target_link_libraries       (wordlist2dawg tesseract)
project_group               (wordlist2dawg "Training Tools")


########################################
# EXECUTABLE set_unicharset_properties
########################################

if (ICU_FOUND)

include_directories(${ICU_INCLUDE_DIRS})

add_executable              (set_unicharset_properties
    set_unicharset_properties.cpp
    unicharset_training_utils.cpp
    unicharset_training_utils.h
    fileio.cpp
    fileio.h
    normstrngs.cpp
    normstrngs.h
    icuerrorcode.h
)
target_link_libraries       (set_unicharset_properties common_training ${ICU_LIBRARIES})
project_group               (set_unicharset_properties "Training Tools")


########################################
# EXECUTABLE text2image
########################################

if (PKG_CONFIG_FOUND)

pkg_check_modules(Pango REQUIRED pango)
pkg_check_modules(Cairo REQUIRED cairo)
pkg_check_modules(PangoFt2 REQUIRED pangoft2)
pkg_check_modules(PangoCairo REQUIRED pangocairo)
pkg_check_modules(FontConfig REQUIRED fontconfig)

set(text2image_src
    text2image.cpp
    boxchar.cpp
    boxchar.h
    degradeimage.cpp
    degradeimage.h
    fileio.cpp
    fileio.h
    ligature_table.cpp
    ligature_table.h
    normstrngs.cpp
    normstrngs.h
    pango_font_info.cpp
    pango_font_info.h
    stringrenderer.cpp
    stringrenderer.h
    tlog.cpp
    tlog.h
    util.h
    icuerrorcode.h
)
if (CYGWIN)
set(text2image_src ${text2image_src} ../vs2010/port/strcasestr.cpp)
endif()

add_executable              (text2image ${text2image_src})
target_include_directories  (text2image BEFORE PRIVATE ${Cairo_INCLUDE_DIRS} ${Pango_INCLUDE_DIRS})
target_compile_definitions  (text2image PRIVATE -DPANGO_ENABLE_ENGINE)
target_link_libraries       (text2image tesseract common_training
    ${ICU_LIBRARIES}
    ${Pango_LIBRARIES}
    ${Cairo_LIBRARIES}
    ${PangoCairo_LIBRARIES}
    ${PangoFt2_LIBRARIES}
    ${FontConfig_LIBRARIES}
)
project_group               (text2image "Training Tools")

endif(PKG_CONFIG_FOUND)
endif(ICU_FOUND)
endif(STATIC OR NOT (WIN32 OR CYGWIN))

###############################################################################
