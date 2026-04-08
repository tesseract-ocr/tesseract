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
#include <stdio.h>      // snprintf
#include <stdlib.h>     // _putenv_s, getenv
#include <string.h>     // strrchr

static char path[4096];

int main(int argc, char *argv[]) {
  if (argc > 1) {
    char *dir = argv[0];
    char *last = strrchr(dir, '\\');
    if (last != nullptr) {
      *last = '\0';
    }
    const char *env_path = getenv("PATH");
    if (env_path == nullptr) {
      env_path = "";
    }
    int result = snprintf(path, sizeof(path), "%s;%s", dir, env_path);
    if (result < 0 || result >= (int)sizeof(path)) {
      return 1;
    }
    _putenv_s("PATH", path);
    _spawnvp(_P_WAIT, argv[1], argv + 1);
    //~ _spawnvp(_P_OVERLAY, argv[1], argv + 1);
  }
  return 0;
}
