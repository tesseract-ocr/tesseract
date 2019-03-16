// Copyright (C) 2024 Stefan Weil
//
// SPDX-License-Identifier: Apache-2.0
//
// winpath - run a Windows program with extended PATH
//
// Usage:
//
//     winpath [CMD [ARGUMENT ...]]
//
// Example:
//
//     winpath cmd
//
// This will start a Windows command line with PATH extended by
// the location of the winpath executable.

#include <process.h>    // _spawnvp
#include <stdlib.h>     // _putenv_s
#include <string.h>     // strcpy, strcat

static char path[4096];

int main(int argc, char *argv[]) {
  if (argc > 1) {
    char *dir = argv[0];
    char *last = strrchr(dir, '\\');
    if (last != nullptr) {
      *last = '\0';
    }
    strcpy(path, dir);
    strcat(path, ";");
    strcat(path, getenv("PATH"));
    _putenv_s("PATH", path);
    _spawnvp(_P_WAIT, argv[1], argv + 1);
    //~ _spawnvp(_P_OVERLAY, argv[1], argv + 1);
  }
  return 0;
}
