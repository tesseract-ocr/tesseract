// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
#include "debugging.h"

// C includes
#include <stdio.h>
#include <stdlib.h>

void helium::AssertionFunction(bool assertion, 
                               const char* assertion_string, 
                               unsigned line, 
                               const char* file) {
  if (!assertion) {
    printf("%s, %d] ASSERTION (%s) FAILED!\n", file, line, assertion_string);
    exit(-1);
  }
}

void helium::LogMessageFunction(const char* message, 
                                unsigned line, 
                                const char* file) {
  printf("%s, %d] %s\n", file, line, message);
}

void helium::ErrorMessageFunction(const char* message, 
                                  unsigned line, 
                                  const char* file) {
  fprintf(stderr, "%s, %d] %s\n", file, line, message);
}
