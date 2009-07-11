/**********************************************************************
 * File:        basedir.c  (Formerly getpath.c)
 * Description: Find the directory location of the current executable using PATH.
 * Author:      Ray Smith
 * Created:     Mon Jul 09 09:06:39 BST 1990
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

#include          "mfcpch.h"     //precompiled headers
#include          "strngs.h"
#ifdef __UNIX__
#include          <unistd.h>
#include                    <fcntl.h>
#endif
#include          <stdlib.h>
#include          "basedir.h"
#include          "varable.h"
#include          "notdll.h"     //must be last include

#ifdef __MSW32__
STRING_VAR(tessedit_module_name, "tessdll.dll",
        "Module colocated with tessdata dir");
#endif

/**********************************************************************
 * getpath
 *
 * Find the directory of the given executable using the usual path rules.
 * This enables data to be located relative to the code.
 **********************************************************************/

DLLSYM inT8 getpath(                   //get dir name of code
                    const char *code,  //executable to locate
                    STRING &path       //output path name
                   ) {
  char directory[MAX_PATH];      //main directory
  #ifdef __UNIX__
  inT16 dirind;                  //index in directory
  register char *pathlist;       //$PATH
  int fd;                        //file descriptor

  strcpy(directory, code);  //get demo directory
  dirind = strlen (directory);
  while (dirind > 0 && directory[dirind - 1] != '/')
    dirind--;                    //look back for dirname
  directory[dirind] = '\0';      //end at directory
  if (dirind != 0) {
    path = directory;            //had it in arg
    return 0;
  }
  pathlist = getenv ("PATH");    //find search path
  while (pathlist != NULL && *pathlist) {
    for (dirind = 0; *pathlist != '\0' && *pathlist != ':';)
                                 //copy a directory
      directory[dirind++] = *pathlist++;
    if (*pathlist == ':')
      pathlist++;
    if (dirind == 0)
      continue;
    if (directory[dirind - 1] != '/');
    directory[dirind++] = '/';   //add ending slash
    directory[dirind++] = '\0';
    path = directory;            //try this path
    strcat(directory, code);
    fd = open (directory, 0);
    if (fd >= 0) {
      close(fd);  //found it
      return 0;
    }
  }
  strcpy (directory, "./");
  path = directory;              //in current?
  strcat(directory, code);
  fd = open (directory, 0);
  if (fd >= 0) {
    close(fd);
    return 0;                    //in current after all
  }
  return -1;
  #endif
  #ifdef __MSW32__
  char *path_end;                //end of dir

  if (code == NULL) {
    // Attempt to get the path of the most relevant module. If the dll
    // is being used, this will be the dll. Otherwise GetModuleHandle will
    // return NULL and default to the path of the executable.
    if (GetModuleFileName(GetModuleHandle(tessedit_module_name.string()),
                          directory, MAX_PATH - 1) == 0) {
      return -1;
    }
  } else {
    strncpy(directory, code, MAX_PATH - 1);
  }
  while ((path_end = strchr (directory, '\\')) != NULL)
    *path_end = '/';
  path_end = strrchr (directory, '/');
  if (path_end != NULL)
    path_end[1] = '\0';
  else
    strcpy (directory, "./");
  path = directory;
  return 0;
  #endif
}
