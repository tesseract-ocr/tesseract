#
# tesseract
#
if(NOT ${CMAKE_VERSION} VERSION_LESS "3.12.0")
  cmake_policy(SET CMP0074 NEW)
endif()

if(UNIX AND NOT ANDROID)
  set(LIB_pthread pthread)
endif()

if(SW_BUILD)
  set(ICU_FOUND 1)
else() # NOT SW_BUILD
  find_package(PkgConfig)
endif()

# experimental
# If PkgConfig is not present training tools will not be build,
# so it does not make sense to set ICU.
if(MSVC
   AND PKG_CONFIG_FOUND
   AND NOT SW_BUILD
   AND NOT USE_SYSTEM_ICU)
  include(CheckTypeSize)
  check_type_size("void *" SIZEOF_VOID_P)

  if(SIZEOF_VOID_P EQUAL 8)
    set(X64 1)
    set(ARCH_NAME 64)
  elseif(SIZEOF_VOID_P EQUAL 4)
    set(X86 1)
    set(ARCH_NAME 32)
  else()
    message(FATAL_ERROR "Cannot determine target architecture")
  endif()

  set(ICU_DIR "${CMAKE_CURRENT_BINARY_DIR}/icu")
  set(ICU_ARCHIVE "${ICU_DIR}/icu${ARCH_NAME}.zip")

  if(X86)
    set(ICU_HASH 45167a240b60e36b59a87eda23490ce4)
  else()
    set(ICU_HASH 480c72491576c048de1218c3c5519399)
  endif()

  message(STATUS "Downloading latest ICU binaries")
  set(COMPILER "msvc10")
  set(ICU_URL "https://github.com/unicode-org/icu/releases/download")
  set(ICU_R "56-1")
  set(ICU_V "56_1")
  file(
    DOWNLOAD
    "${ICU_URL}/release-${ICU_R}/icu4c-${ICU_V}-Win${ARCH_NAME}-${COMPILER}.zip"
    "${ICU_ARCHIVE}"
    SHOW_PROGRESS
    INACTIVITY_TIMEOUT 300 # seconds
    EXPECTED_HASH MD5=${ICU_HASH})
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xz "${ICU_ARCHIVE}"
    WORKING_DIRECTORY "${ICU_DIR}"
    RESULT_VARIABLE __result)
  if(NOT __result EQUAL 0)
    message(FATAL_ERROR "error ${__result}")
  endif()

  set(ICU_ROOT ${ICU_DIR}/icu)
endif()
# experimental

if(NOT SW_BUILD)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-uc icu-i18n)
  else()
    find_package(ICU 52.1 COMPONENTS uc i18n)
  endif()
  if(ICU_FOUND)
    message(">> ICU_FOUND ${ICU_FOUND} ${ICU_VERSION} ${ICU_LIBRARIES} ${ICU_INCLUDE_DIRS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ICU_CXX_FLAGS}")
  else()
    message(">> ICU not found!")
  endif()
endif()


# ##############################################################################
# LIBRARY common_training
# ##############################################################################

set(COMMON_TRAINING_SRC
    common/commandlineflags.cpp
    common/commandlineflags.h
    common/commontraining.cpp
    common/commontraining.h
    common/ctc.cpp
    common/ctc.h
    common/networkbuilder.cpp
    common/networkbuilder.h)

if(NOT DISABLED_LEGACY_ENGINE)
  list(
    APPEND
    COMMON_TRAINING_SRC
    common/errorcounter.cpp
    common/errorcounter.h
    common/intfeaturedist.cpp
    common/intfeaturedist.h
    common/intfeaturemap.cpp
    common/intfeaturemap.h
    common/mastertrainer.cpp
    common/mastertrainer.h
    common/sampleiterator.cpp
    common/sampleiterator.h
    common/trainingsampleset.cpp
    common/trainingsampleset.h)
endif()

add_library(common_training ${COMMON_TRAINING_SRC})
target_include_directories(common_training PUBLIC common
                                                  ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(common_training PUBLIC libtesseract)
install(
  TARGETS common_training
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
generate_export_header(common_training EXPORT_MACRO_NAME
                       TESS_COMMON_TRAINING_API)
if (MSVC AND BUILD_SHARED_LIBS)
  install(FILES $<TARGET_PDB_FILE:common_training> DESTINATION bin OPTIONAL)
endif()
project_group(common_training "Training Tools")

# ##############################################################################
# EXECUTABLE ambiguous_words
# ##############################################################################

if(NOT DISABLED_LEGACY_ENGINE)
  add_executable(ambiguous_words ambiguous_words.cpp)
  target_link_libraries(ambiguous_words common_training)
  project_group(ambiguous_words "Training Tools")
  install(
    TARGETS ambiguous_words
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  if (MSVC)
    install(FILES $<TARGET_PDB_FILE:ambiguous_words> DESTINATION bin OPTIONAL)
  endif()
endif()

# ##############################################################################
# EXECUTABLE classifier_tester
# ##############################################################################

if(NOT DISABLED_LEGACY_ENGINE)
  add_executable(classifier_tester classifier_tester.cpp)
  target_link_libraries(classifier_tester common_training)
  project_group(classifier_tester "Training Tools")
  install(
    TARGETS classifier_tester
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  if (MSVC)
    install(FILES $<TARGET_PDB_FILE:classifier_tester> DESTINATION bin OPTIONAL)
  endif()
endif()

# ##############################################################################
# EXECUTABLE combine_tessdata
# ##############################################################################

add_executable(combine_tessdata combine_tessdata.cpp)
target_link_libraries(combine_tessdata common_training)
project_group(combine_tessdata "Training Tools")
install(
  TARGETS combine_tessdata
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
if (MSVC)
  install(FILES $<TARGET_PDB_FILE:combine_tessdata> DESTINATION bin OPTIONAL)
endif()

# ##############################################################################
# EXECUTABLE cntraining
# ##############################################################################

if(NOT DISABLED_LEGACY_ENGINE)
  add_executable(cntraining cntraining.cpp)
  target_link_libraries(cntraining common_training)
  project_group(cntraining "Training Tools")
  install(
    TARGETS cntraining
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  if (MSVC)
    install(FILES $<TARGET_PDB_FILE:cntraining> DESTINATION bin OPTIONAL)
  endif()
endif()

# ##############################################################################
# EXECUTABLE dawg2wordlist
# ##############################################################################

add_executable(dawg2wordlist dawg2wordlist.cpp)
target_link_libraries(dawg2wordlist common_training)
project_group(dawg2wordlist "Training Tools")
install(
  TARGETS dawg2wordlist
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
if (MSVC)
  install(FILES $<TARGET_PDB_FILE:dawg2wordlist> DESTINATION bin OPTIONAL)
endif()

# ##############################################################################
# EXECUTABLE mftraining
# ##############################################################################

if(NOT DISABLED_LEGACY_ENGINE)
  add_executable(mftraining mftraining.cpp mergenf.cpp mergenf.h)
  target_link_libraries(mftraining common_training)
  project_group(mftraining "Training Tools")
  install(
    TARGETS mftraining
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  if (MSVC)
    install(FILES $<TARGET_PDB_FILE:mftraining> DESTINATION bin OPTIONAL)
  endif()
endif()

# ##############################################################################
# EXECUTABLE shapeclustering
# ##############################################################################

if(NOT DISABLED_LEGACY_ENGINE)
  add_executable(shapeclustering shapeclustering.cpp)
  target_link_libraries(shapeclustering common_training)
  project_group(shapeclustering "Training Tools")
  install(
    TARGETS shapeclustering
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  if (MSVC)
     install(FILES $<TARGET_PDB_FILE:shapeclustering> DESTINATION bin OPTIONAL)
  endif()
endif()

# ##############################################################################
# EXECUTABLE wordlist2dawg
# ##############################################################################

add_executable(wordlist2dawg wordlist2dawg.cpp)
target_link_libraries(wordlist2dawg common_training)
project_group(wordlist2dawg "Training Tools")
install(
  TARGETS wordlist2dawg
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
if (MSVC)
  install(FILES $<TARGET_PDB_FILE:wordlist2dawg> DESTINATION bin OPTIONAL)
endif()

if(ICU_FOUND)
  if(NOT SW_BUILD)
    include_directories(${ICU_INCLUDE_DIRS})
  endif()

  # ############################################################################
  # LIBRARY unicharset_training
  # ############################################################################

  file(GLOB unicharset_training_src unicharset/*)

  add_library(unicharset_training ${unicharset_training_src})
  if(SW_BUILD)
    target_link_libraries(unicharset_training
                          PUBLIC common_training org.sw.demo.unicode.icu.i18n)
  else()
    if(PKG_CONFIG_FOUND)
      target_link_libraries(unicharset_training PUBLIC common_training PkgConfig::ICU)
    else()
      target_link_libraries(unicharset_training PUBLIC common_training ${ICU_LIBRARIES})
    endif()
  endif()
  target_include_directories(unicharset_training
                             PUBLIC unicharset ${CMAKE_CURRENT_BINARY_DIR})
  install(
    TARGETS unicharset_training
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
  if (MSVC AND BUILD_SHARED_LIBS)
    install(FILES $<TARGET_PDB_FILE:unicharset_training> DESTINATION bin OPTIONAL)
  endif()
  generate_export_header(unicharset_training EXPORT_MACRO_NAME
                         TESS_UNICHARSET_TRAINING_API)
  project_group(unicharset_training "Training Tools")

  # ############################################################################
  # EXECUTABLE combine_lang_model
  # ############################################################################

  add_executable(combine_lang_model combine_lang_model.cpp)
  target_link_libraries(combine_lang_model unicharset_training)
  project_group(combine_lang_model "Training Tools")
  install(
    TARGETS combine_lang_model
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  if (MSVC)
    install(FILES $<TARGET_PDB_FILE:combine_lang_model> DESTINATION bin OPTIONAL)
  endif()

  # ############################################################################
  # EXECUTABLE lstmeval
  # ############################################################################

  add_executable(lstmeval lstmeval.cpp)
  target_link_libraries(lstmeval unicharset_training ${LIB_pthread})
  project_group(lstmeval "Training Tools")
  install(
    TARGETS lstmeval
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  if (MSVC)
    install(FILES $<TARGET_PDB_FILE:lstmeval> DESTINATION bin OPTIONAL)
  endif()

  # ############################################################################
  # EXECUTABLE lstmtraining
  # ############################################################################

  add_executable(lstmtraining lstmtraining.cpp)
  target_link_libraries(lstmtraining unicharset_training ${LIB_pthread})
  project_group(lstmtraining "Training Tools")
  install(
    TARGETS lstmtraining
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  if (MSVC)
    install(FILES $<TARGET_PDB_FILE:lstmtraining> DESTINATION bin OPTIONAL)
  endif()

  # ############################################################################
  # EXECUTABLE merge_unicharsets
  # ############################################################################

  add_executable(merge_unicharsets merge_unicharsets.cpp)
  target_link_libraries(merge_unicharsets common_training)
  project_group(merge_unicharsets "Training Tools")
  install(
    TARGETS merge_unicharsets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  if (MSVC)
    install(FILES $<TARGET_PDB_FILE:merge_unicharsets> DESTINATION bin OPTIONAL)
  endif()

  # ############################################################################
  # EXECUTABLE set_unicharset_properties
  # ############################################################################

  add_executable(set_unicharset_properties set_unicharset_properties.cpp)
  target_link_libraries(set_unicharset_properties unicharset_training)
  project_group(set_unicharset_properties "Training Tools")
  install(
    TARGETS set_unicharset_properties
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  if (MSVC)
    install(FILES $<TARGET_PDB_FILE:set_unicharset_properties> DESTINATION bin OPTIONAL)
  endif()

  # ############################################################################
  # EXECUTABLE unicharset_extractor
  # ############################################################################

  add_executable(unicharset_extractor unicharset_extractor.cpp)
  target_compile_features(unicharset_extractor PRIVATE cxx_std_17)
  target_link_libraries(unicharset_extractor unicharset_training)
  project_group(unicharset_extractor "Training Tools")
  install(
    TARGETS unicharset_extractor
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  if (MSVC)
    install(FILES $<TARGET_PDB_FILE:unicharset_extractor> DESTINATION bin OPTIONAL)
  endif()

  # ############################################################################

  if(PKG_CONFIG_FOUND OR SW_BUILD)

    if(PKG_CONFIG_FOUND)
      pkg_check_modules(
        PANGO
        REQUIRED
        IMPORTED_TARGET
        pango>=1.38.0
        cairo
        pangoft2
        pangocairo
        fontconfig)
    endif()

    # ##########################################################################
    # LIBRARY pango_training
    # ##########################################################################

    file(GLOB pango_training_src pango/*)

    add_library(pango_training ${pango_training_src})
    target_link_libraries(pango_training PUBLIC unicharset_training)
    if(SW_BUILD)
      target_link_libraries(pango_training
                            PUBLIC org.sw.demo.gnome.pango.pangocairo)
    else()
      if(PKG_CONFIG_FOUND)
        target_include_directories(pango_training BEFORE
                                   PUBLIC ${PANGO_INCLUDE_DIRS})
        target_compile_definitions(pango_training PUBLIC -DPANGO_ENABLE_ENGINE)
        target_link_libraries(pango_training PUBLIC PkgConfig::PANGO)
      endif()
    endif()
    target_include_directories(pango_training
                               PUBLIC pango ${CMAKE_CURRENT_BINARY_DIR})
    generate_export_header(pango_training EXPORT_MACRO_NAME
                           TESS_PANGO_TRAINING_API)
    project_group(pango_training "Training Tools")

    # ##########################################################################
    # EXECUTABLE text2image
    # ##########################################################################

    set(TEXT2IMAGE_SRC text2image.cpp degradeimage.cpp degradeimage.h)

    add_executable(text2image ${TEXT2IMAGE_SRC})
    target_link_libraries(text2image pango_training)
    project_group(text2image "Training Tools")
    install(
      TARGETS text2image
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib)
    if (MSVC)
      install(FILES $<TARGET_PDB_FILE:text2image> DESTINATION bin OPTIONAL)
    endif()
  endif()
endif(ICU_FOUND)

# ##############################################################################
