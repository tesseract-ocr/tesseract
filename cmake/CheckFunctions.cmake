# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by
# applicable law or agreed to in writing, software distributed under the License
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the License for the specific language
# governing permissions and limitations under the License.
# ##############################################################################
#
# macros and functions
#
# ##############################################################################

# ##############################################################################
# FUNCTION check_leptonica_tiff_support
# ##############################################################################
function(check_leptonica_tiff_support)
  # check if leptonica was build with tiff support set result to
  # LEPT_TIFF_RESULT
  set(TIFF_TEST
  "#include \"leptonica/allheaders.h\"\n"
  "int main() {\n"
  "  l_uint8 *data = NULL;\n"
  "  size_t size = 0;\n"
  "  PIX* pix = pixCreate(3, 3, 4);\n"
  "  l_int32 ret_val = pixWriteMemTiff(&data, &size, pix, IFF_TIFF_G3);\n"
  "  pixDestroy(&pix);\n"
  "  lept_free(data);\n"
  "  return ret_val;}\n")
  if(${CMAKE_VERSION} VERSION_LESS "3.25")
    message(STATUS "Testing TIFF support in Leptonica is available with CMake >= 3.25 (you have ${CMAKE_VERSION}))")
  else()
    set(CMAKE_TRY_COMPILE_CONFIGURATION ${CMAKE_BUILD_TYPE})
    try_run(
      LEPT_TIFF_RESULT
      LEPT_TIFF_COMPILE_SUCCESS
      SOURCE_FROM_CONTENT tiff_test.cpp "${TIFF_TEST}"
      CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${Leptonica_INCLUDE_DIRS}"
      LINK_LIBRARIES ${Leptonica_LIBRARIES}
      COMPILE_OUTPUT_VARIABLE
      COMPILE_OUTPUT)
    if(NOT LEPT_TIFF_COMPILE_SUCCESS)
      message(STATUS "COMPILE_OUTPUT: ${COMPILE_OUTPUT}")
      message(STATUS "Leptonica_INCLUDE_DIRS: ${Leptonica_INCLUDE_DIRS}")
      message(STATUS "Leptonica_LIBRARIES: ${Leptonica_LIBRARIES}")
      message(STATUS "LEPT_TIFF_RESULT: ${LEPT_TIFF_RESULT}")
      message(STATUS "LEPT_TIFF_COMPILE: ${LEPT_TIFF_COMPILE}")
      message(WARNING "Failed to compile test")
    endif()
  endif()
endfunction(check_leptonica_tiff_support)

# ##############################################################################
