# find_package(GTest REQUIRED)
include(GoogleTest) # Todo install GoogleTests?

# Set common include directories
set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}/../src/training
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ccutil
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ccstruct
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/viewer
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/training/unicharset
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/training/common
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest/googlemock/include)

if (MSVC)
    set(TESSBIN_DIR ${EXECUTABLE_OUTPUT_PATH}/$<CONFIG>)
else()
    set(TESSBIN_DIR ${EXECUTABLE_OUTPUT_PATH})
endif()

# Set common compile definitions
set(COMMON_COMPILE_DEFINITIONS
    "-DTESTING_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../test/testing\""
    "-DTESSDATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../tessdata\""
    "-DTESSBIN_DIR=\"${TESSBIN_DIR}\""
    "-DTESTDATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../test/testdata\""
    "-DLANGDATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../langdata_lstm\"")

file(
  GLOB TEST_SOURCES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "*.cc")

set(COMMON_LINK_LIBS libtesseract GTest::gtest_main common_training
                     unicharset_training)

set(TRAINING_TESTS
    commandlineflags_test.cc
    dawg_test.cc
    lstm_recode_test.cc
    lstm_squashed_test.cc
    lstm_test.cc
    lstm_test.cc
    normstrngs_test.cc
    unichar_test.cc
    unicharcompress_test.cc
    unicharset_test.cc
    validate_grapheme_test.cc
    validate_indic_test.cc
    validate_khmer_test.cc
    validate_myanmar_test.cc
    validator_test.cc)

set(PANGO_TESTS ligature_table_test.cc pango_font_info_test.cc
                pango_font_info_test.cc stringrenderer_test.cc)

set(LEGACY_TESTS
    applybox_test.cc
    bitvector_test.cc
    equationdetect_test.cc
    indexmapbidi_test.cc
    intfeaturemap_test.cc
    mastertrainer_test.cc
    osd_test.cc
    params_model_test.cc
    shapetable_test.cc)

if(BUILD_TRAINING_TOOLS AND PANGO_FOUND)
  list(APPEND COMMON_INCLUDE_DIRS
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/training/pango ${PANGO_INCLUDE_DIRS})

else()
  list(REMOVE_ITEM TEST_SOURCES ${PANGO_TESTS})
endif()

if(DISABLED_LEGACY_ENGINE)
  list(REMOVE_ITEM TEST_SOURCES ${LEGACY_TESTS})
endif()

if(NOT BUILD_TRAINING_TOOLS)
  list(REMOVE_ITEM TEST_SOURCES ${TRAINING_TESTS})
endif()

set(TATWEEL_TEST_EXTRA_SRC util/utf8/unilib.cc util/utf8/unicodetext.cc
                           third_party/utf/rune.c)

message(STATUS "Enabled tests: ${TEST_SOURCES}")

foreach(test_source IN LISTS TEST_SOURCES)
  get_filename_component(test_name ${test_source} NAME_WE)
  if(${test_source} IN_LIST PANGO_TESTS)
    list(APPEND COMMON_LINK_LIBS pango_training ${PANGO_LIBRARIES})
  endif()
  if(${test_name} MATCHES "tatweel_test")
    list(APPEND test_source ${TATWEEL_TEST_EXTRA_SRC})
    list(APPEND COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/util/utf8)
  endif()
  add_executable(${test_name} ${test_source})
  if(${test_name} MATCHES "progress_test")
    target_link_libraries(${test_name} PRIVATE GTest::gmock)
  endif()
  target_compile_definitions(${test_name} PRIVATE ${COMMON_COMPILE_DEFINITIONS})
  target_include_directories(${test_name} PRIVATE ${COMMON_INCLUDE_DIRS})
  target_link_libraries(${test_name} PRIVATE ${COMMON_LINK_LIBS})
  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# Discover tests gtest_discover_tests(apiexample_test baseapi_test
# baseapi_thread_test) add_test(baseapi_gtests baseapi_test.cc)
