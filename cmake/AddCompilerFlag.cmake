# - Add a given compiler flag to flags variables.
# AddCompilerFlag(<flag> [<var>])
# or
# AddCompilerFlag(<flag> [C_FLAGS <var>] [CXX_FLAGS <var>] [C_RESULT <var>]
#                        [CXX_RESULT <var>])

#=============================================================================
# Copyright 2010-2015 Matthias Kretz <kretz@kde.org>
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
#  * Neither the names of contributing organizations nor the
#    names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
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

get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

macro(AddCompilerFlag _flag)
   string(REGEX REPLACE "[-.+/:= ]" "_" _flag_esc "${_flag}")

   set(_c_flags "CMAKE_C_FLAGS")
   set(_cxx_flags "CMAKE_CXX_FLAGS")
   set(_c_result tmp)
   set(_cxx_result tmp)
   if(${ARGC} EQUAL 2)
      message(WARNING "Deprecated use of the AddCompilerFlag macro.")
      unset(_c_result)
      set(_cxx_result ${ARGV1})
   elseif(${ARGC} GREATER 2)
      set(state 0)
      unset(_c_flags)
      unset(_cxx_flags)
      unset(_c_result)
      unset(_cxx_result)
      foreach(_arg ${ARGN})
         if("x${_arg}" STREQUAL "xC_FLAGS")
            set(state 1)
            if(NOT DEFINED _c_result)
               set(_c_result tmp0)
            endif()
         elseif("x${_arg}" STREQUAL "xCXX_FLAGS")
            set(state 2)
            if(NOT DEFINED _cxx_result)
               set(_cxx_result tmp1)
            endif()
         elseif("x${_arg}" STREQUAL "xC_RESULT")
            set(state 3)
         elseif("x${_arg}" STREQUAL "xCXX_RESULT")
            set(state 4)
         elseif(state EQUAL 1)
            set(_c_flags "${_arg}")
         elseif(state EQUAL 2)
            set(_cxx_flags "${_arg}")
         elseif(state EQUAL 3)
            set(_c_result "${_arg}")
         elseif(state EQUAL 4)
            set(_cxx_result "${_arg}")
         else()
            message(FATAL_ERROR "Syntax error for AddCompilerFlag")
         endif()
      endforeach()
   endif()

   set(_c_code "int main() { return 0; }")
   set(_cxx_code "int main() { return 0; }")
   if("${_flag}" STREQUAL "-mfma")
      # Compiling with FMA3 support may fail only at the assembler level.
      # In that case we need to have such an instruction in the test code
      set(_c_code "#include <immintrin.h>
      __m128 foo(__m128 x) { return _mm_fmadd_ps(x, x, x); }
      int main() { return 0; }")
      set(_cxx_code "${_c_code}")
   elseif("${_flag}" STREQUAL "-stdlib=libc++")
      # Compiling with libc++ not only requires a compiler that understands it, but also
      # the libc++ headers itself
      set(_cxx_code "#include <iostream>
      #include <cstdio>
      int main() { return 0; }")
   else()
      set(_cxx_code "#include <cstdio>
      int main() { return 0; }")
   endif()

   if(DEFINED _c_result)
      check_c_compiler_flag("${_flag}" check_c_compiler_flag_${_flag_esc} "${_c_code}")
      set(${_c_result} ${check_c_compiler_flag_${_flag_esc}})
   endif()
   if(DEFINED _cxx_result)
      check_cxx_compiler_flag("${_flag}" check_cxx_compiler_flag_${_flag_esc} "${_cxx_code}")
      set(${_cxx_result} ${check_cxx_compiler_flag_${_flag_esc}})
   endif()

   macro(my_append _list _flag _special)
      if("x${_list}" STREQUAL "x${_special}")
         set(${_list} "${${_list}} ${_flag}")
      else()
         list(APPEND ${_list} "${_flag}")
      endif()
   endmacro()

   if(check_c_compiler_flag_${_flag_esc} AND DEFINED _c_flags)
      my_append(${_c_flags} "${_flag}" CMAKE_C_FLAGS)
   endif()
   if(check_cxx_compiler_flag_${_flag_esc} AND DEFINED _cxx_flags)
      my_append(${_cxx_flags} "${_flag}" CMAKE_CXX_FLAGS)
   endif()
endmacro(AddCompilerFlag)
