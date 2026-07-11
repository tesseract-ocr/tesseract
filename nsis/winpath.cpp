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

#include <filesystem>   // std::filesystem::path
#include <process.h>    // _spawnvp
#include <cstdlib>      // _putenv_s, getenv
#include <string>       // std::string

int main(int argc, char *argv[]) {
  if (argc > 1) {
    std::filesystem::path exe_path(argv[0]);
    std::string dir = exe_path.parent_path().string();

    const char *env_path = getenv("PATH");
    std::string new_path = dir + ";" + (env_path ? env_path : "");

    _putenv_s("PATH", new_path.c_str());
    return _spawnvp(_P_WAIT, argv[1], argv + 1);
    //~ _spawnvp(_P_OVERLAY, argv[1], argv + 1);
  }
  return 0;
}
