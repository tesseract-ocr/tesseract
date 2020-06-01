# - Check whether the C compiler supports a given flag.
# CHECK_C_COMPILER_FLAG(<flag> <var>)
#  <flag> - the compiler flag
#  <var>  - variable to store the result
# This internally calls the check_c_source_compiles macro.
# See help for CheckCSourceCompiles for a listing of variables
# that can modify the build.

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
# Copyright 2011-2013 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#  * Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
#  * The names of Kitware, Inc., the Insight Consortium, or the names of
#    any consortium members, or of any contributors, may not be used to
#    endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

INCLUDE(CheckCSourceCompiles)

MACRO (CHECK_C_COMPILER_FLAG _FLAG _RESULT)
   SET(SAFE_CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
   SET(CMAKE_REQUIRED_DEFINITIONS "${_FLAG}")
   if(${ARGC} GREATER 2)
      SET(TEST_SOURCE "${ARGV2}")
   else()
      SET(TEST_SOURCE "int main() { return 0;}")
   endif()
   CHECK_C_SOURCE_COMPILES("${TEST_SOURCE}" ${_RESULT}
     # Some compilers do not fail with a bad flag
     FAIL_REGEX "error: bad value (.*) for .* switch"       # GNU
     FAIL_REGEX "argument unused during compilation"        # clang
     FAIL_REGEX "is valid for .* but not for C"             # GNU
     FAIL_REGEX "unrecognized .*option"                     # GNU
     FAIL_REGEX "ignored for target"                        # GNU
     FAIL_REGEX "ignoring unknown option"                   # MSVC
     FAIL_REGEX "warning D9002"                             # MSVC
     FAIL_REGEX "[Uu]nknown option"                         # HP
     FAIL_REGEX "[Ww]arning: [Oo]ption"                     # SunPro
     FAIL_REGEX "command option .* is not recognized"       # XL
     FAIL_REGEX "WARNING: unknown flag:"                    # Open64
     FAIL_REGEX "command line error"                        # ICC
     FAIL_REGEX "command line warning"                      # ICC
     FAIL_REGEX "#10236:"                                   # ICC: File not found
     FAIL_REGEX " #10159: "                                 # ICC
     FAIL_REGEX " #10353: "                                 # ICC: option '-mfma' ignored, suggest using '-march=core-avx2'
     )
   SET (CMAKE_REQUIRED_DEFINITIONS "${SAFE_CMAKE_REQUIRED_DEFINITIONS}")
ENDMACRO (CHECK_C_COMPILER_FLAG)

