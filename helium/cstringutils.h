// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains convenience functions for C string processing. They
// are more light-weight than the String class (see stringutils.h).
//
#ifndef HELIUM_CSTRINGUTILS_H__
#define HELIUM_CSTRINGUTILS_H__

namespace helium {
  
  // Returns an allocated string that has string_b concatenated to string_b.
  char* StringConcat(const char* string_a, const char* string_b);
  
  // Returns a string that contains all words, but the last. The given 
  // delimeter specifies how words are separated. This is mainly used for
  // separating path components.
  char* StringChopTail(const char* string, char delimiter);

} // namespace

#endif  // HELIUM_CSTRINGUTILS_H__
