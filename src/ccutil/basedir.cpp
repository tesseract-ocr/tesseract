/**********************************************************************
 * File:        basedir.cpp  (Formerly getpath.c)
 * Description: Find the directory location of the current executable using
 *PATH. Author:      Ray Smith Created:     Mon Jul 09 09:06:39 BST 1990
 *
 * (C) Copyright 1990, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include          "basedir.h"

#include          <cstdlib>

// Assuming that code_path is the name of some file in a desired directory,
// returns the given code_path stripped back to the last slash, leaving
// the last slash in place. If there is no slash, returns ./ assuming that
// the input was the name of something in the current directory.
// Useful for getting to the directory of argv[0], but does not search
// any paths.
TESS_API void truncate_path(const char *code_path, STRING* trunc_path) {
  int trunc_index = -1;
  if (code_path != nullptr) {
    const char* last_slash = strrchr(code_path, '/');
    if (last_slash != nullptr && last_slash + 1 - code_path > trunc_index)
      trunc_index = last_slash + 1 - code_path;
    last_slash = strrchr(code_path, '\\');
    if (last_slash != nullptr && last_slash + 1 - code_path > trunc_index)
      trunc_index = last_slash + 1 - code_path;
  }
  *trunc_path = code_path;
  if (trunc_index >= 0)
    trunc_path->truncate_at(trunc_index);
  else
    *trunc_path = "./";
}
