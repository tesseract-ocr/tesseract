// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Local includes
#include "cstringutils.h"

// C includes
#include <stdlib.h>
#include <string.h>

char* helium::StringConcat(const char* string_a, const char* string_b) {
  char* out = new char[strlen(string_a) + strlen(string_b) + 1];
  if (strlen(string_a) > 0) strcpy(out, string_a);
  if (strlen(string_b) > 0) strcat(out, string_b);
  return out;
}
  
char* helium::StringChopTail(const char* string, char delimiter) {
  for (int index = strlen(string) - 1; index >= 0; index--) {
    if (string[index] == delimiter) {
      char* out = new char[index + 2];
      memcpy(out, string, index + 1);
      out[index + 1] = '\0';
      return out;
    }
  }
  return NULL;
}
