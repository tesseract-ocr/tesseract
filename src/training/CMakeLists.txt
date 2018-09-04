#
# tesseract
#

if (NOT CPPAN_BUILD AND NOT (WIN32 OR CYGWIN))
    return()
endif()

if (CPPAN_BUILD)
    set(ICU_FOUND 1)
endif()

# experimental
if (MSVC AND NOT CPPAN_BUILD AND NOT USE_SYSTEM_ICU)
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
        INACTIVITY_TIMEOUT 60 # seconds
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
endif()
# experimental

if (NOT CPPAN_BUILD)
    find_package(ICU COMPONENTS uc i18n)
endif()

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
    commandlineflags.h
    commontraining.cpp
    commontraining.h
)
add_library                 (common_training ${common_training_src})
target_link_libraries       (common_training libtesseract tessopt)
project_group               (common_training "Training Tools")


########################################
# EXECUTABLE ambiguous_words
########################################

add_executable              (ambiguous_words ambiguous_words.cpp)
target_link_libraries       (ambiguous_words libtesseract)
project_group               (ambiguous_words "Training Tools")
install                     (TARGETS ambiguous_words RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


########################################
# EXECUTABLE classifier_tester
########################################

add_executable              (classifier_tester classifier_tester.cpp)
target_link_libraries       (classifier_tester common_training)
project_group               (classifier_tester "Training Tools")
install                     (TARGETS classifier_tester RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


########################################
# EXECUTABLE combine_tessdata
########################################

add_executable              (combine_tessdata combine_tessdata.cpp)
target_link_libraries       (combine_tessdata libtesseract)
project_group               (combine_tessdata "Training Tools")
install                     (TARGETS combine_tessdata RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


########################################
# EXECUTABLE cntraining
########################################

add_executable              (cntraining cntraining.cpp)
target_link_libraries       (cntraining common_training)
project_group               (cntraining "Training Tools")
install                     (TARGETS cntraining RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


########################################
# EXECUTABLE dawg2wordlist
########################################

add_executable              (dawg2wordlist dawg2wordlist.cpp)
target_link_libraries       (dawg2wordlist libtesseract)
project_group               (dawg2wordlist "Training Tools")
install                     (TARGETS dawg2wordlist RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


########################################
# EXECUTABLE mftraining
########################################

add_executable              (mftraining mftraining.cpp mergenf.cpp mergenf.h)
target_link_libraries       (mftraining common_training)
project_group               (mftraining "Training Tools")
install                     (TARGETS mftraining RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


########################################
# EXECUTABLE shapeclustering
########################################

add_executable              (shapeclustering shapeclustering.cpp)
target_link_libraries       (shapeclustering common_training)
project_group               (shapeclustering "Training Tools")
install                     (TARGETS shapeclustering RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


########################################
# EXECUTABLE wordlist2dawg
########################################

add_executable              (wordlist2dawg wordlist2dawg.cpp)
target_link_libraries       (wordlist2dawg libtesseract)
project_group               (wordlist2dawg "Training Tools")
install                     (TARGETS wordlist2dawg RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


if (ICU_FOUND)

if (NOT CPPAN_BUILD)
include_directories(${ICU_INCLUDE_DIRS})
endif()

########################################
# LIBRARY unicharset_training
########################################

set(unicharset_training_src
    icuerrorcode.cpp
    icuerrorcode.h
    lang_model_helpers.cpp
    lang_model_helpers.h
    lstmtester.cpp
    lstmtester.h
    normstrngs.cpp
    normstrngs.h
    unicharset_training_utils.cpp
    unicharset_training_utils.h

    validate_grapheme.h validate_indic.h validate_khmer.h
    validate_javanese.h validate_myanmar.h validator.h
    validate_grapheme.cpp validate_indic.cpp validate_khmer.cpp
    validate_javanese.cpp validate_myanmar.cpp validator.cpp

)
add_library                 (unicharset_training ${unicharset_training_src})
if (NOT CPPAN_BUILD)
target_link_libraries       (unicharset_training common_training ${ICU_LIBRARIES})
else()
target_link_libraries       (unicharset_training common_training pvt.cppan.demo.unicode.icu.i18n)
endif()
project_group               (unicharset_training "Training Tools")


########################################
# EXECUTABLE combine_lang_model
########################################

add_executable              (combine_lang_model combine_lang_model.cpp)
target_link_libraries       (combine_lang_model unicharset_training)
project_group               (combine_lang_model "Training Tools")
install                     (TARGETS combine_lang_model RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


########################################
# EXECUTABLE lstmeval
########################################

add_executable              (lstmeval lstmeval.cpp)
target_link_libraries       (lstmeval unicharset_training)
project_group               (lstmeval "Training Tools")
install                     (TARGETS lstmeval RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


########################################
# EXECUTABLE lstmtraining
########################################

add_executable              (lstmtraining lstmtraining.cpp)
target_link_libraries       (lstmtraining unicharset_training)
project_group               (lstmtraining "Training Tools")
install                     (TARGETS lstmtraining RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


########################################
# EXECUTABLE set_unicharset_properties
########################################

add_executable              (set_unicharset_properties set_unicharset_properties.cpp)
target_link_libraries       (set_unicharset_properties unicharset_training)
project_group               (set_unicharset_properties "Training Tools")
install                     (TARGETS set_unicharset_properties RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


########################################
# EXECUTABLE unicharset_extractor
########################################

add_executable              (unicharset_extractor unicharset_extractor.cpp)
target_link_libraries       (unicharset_extractor libtesseract tessopt unicharset_training)
project_group               (unicharset_extractor "Training Tools")
install                     (TARGETS unicharset_extractor RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


########################################
# EXECUTABLE text2image
########################################

if (NOT CPPAN_BUILD)
find_package(PkgConfig)
endif()

if (PKG_CONFIG_FOUND OR CPPAN_BUILD)

if (PKG_CONFIG_FOUND)
pkg_check_modules(Pango REQUIRED pango)
pkg_check_modules(Cairo REQUIRED cairo)
pkg_check_modules(PangoFt2 REQUIRED pangoft2)
pkg_check_modules(PangoCairo REQUIRED pangocairo)
pkg_check_modules(FontConfig REQUIRED fontconfig)
endif()

set(text2image_src
    text2image.cpp
    boxchar.cpp
    boxchar.h
    degradeimage.cpp
    degradeimage.h
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

add_executable              (text2image ${text2image_src})
target_link_libraries       (text2image unicharset_training)
if (PKG_CONFIG_FOUND)
target_include_directories  (text2image BEFORE PRIVATE ${Cairo_INCLUDE_DIRS} ${Pango_INCLUDE_DIRS})
target_compile_definitions  (text2image PRIVATE -DPANGO_ENABLE_ENGINE)
target_link_libraries       (text2image
    ${Pango_LIBRARIES}
    ${Cairo_LIBRARIES}
    ${PangoCairo_LIBRARIES}
    ${PangoFt2_LIBRARIES}
    ${FontConfig_LIBRARIES}
)
endif()
if (CPPAN_BUILD)
target_link_libraries       (text2image pvt.cppan.demo.gnome.pango.pangocairo)
endif()
project_group               (text2image "Training Tools")
install                     (TARGETS text2image RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

endif()
endif(ICU_FOUND)

###############################################################################
