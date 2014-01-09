/* -*-C-*-
 ********************************************************************************
 *
 * File:         helpers.h
 * Description:  General utility functions
 * Author:       Daria Antonova
 * Created:      Wed Apr 8 14:37:00 2009
 * Language:     C++
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 2009, Google Inc.
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
 ********************************************************************************/

#ifndef TESSERACT_CCUTIL_HELPERS_H_
#define TESSERACT_CCUTIL_HELPERS_H_

#include <stdio.h>
#include <string.h>

// Remove newline (if any) at the end of the string.
inline void chomp_string(char *str) {
  int last_index = strlen(str) - 1;
  while (last_index >= 0 &&
         (str[last_index] == '\n' || str[last_index] == '\r')) {
    str[last_index--] = '\0';
  }
}

// Advance the current pointer of the file if it points to a newline character.
inline void SkipNewline(FILE *file) {
  if (fgetc(file) != '\n') fseek(file, -1, SEEK_CUR);
}

// Swaps the two args pointed to by the pointers.
// Operator= and copy constructor must work on T.
template<typename T> inline void Swap(T* p1, T* p2) {
  T tmp(*p2);
  *p2 = *p1;
  *p1 = tmp;
}

// qsort function to sort 2 floats.
inline int sort_floats(const void *arg1, const void *arg2) {
  float diff = *((float *) arg1) - *((float *) arg2);
  if (diff > 0) {
    return 1;
  } else if (diff < 0) {
    return -1;
  } else {
    return 0;
  }
}

// return the smallest multiple of block_size greater than or equal to n.
inline int RoundUp(int n, int block_size) {
  return block_size * ((n + block_size - 1) / block_size);
}

// Clip a numeric value to the interval [lower_bound, upper_bound].
template<typename T>
inline T ClipToRange(const T& x, const T& lower_bound, const T& upper_bound) {
  if (x < lower_bound)
    return lower_bound;
  if (x > upper_bound)
    return upper_bound;
  return x;
}

// Extend the range [lower_bound, upper_bound] to include x.
template<typename T1, typename T2>
inline void UpdateRange(const T1& x, T2* lower_bound, T2* upper_bound) {
  if (x < *lower_bound)
    *lower_bound = x;
  if (x > *upper_bound)
    *upper_bound = x;
}

// Decrease lower_bound to be <= x_lo AND increase upper_bound to be >= x_hi.
template<typename T1, typename T2>
inline void UpdateRange(const T1& x_lo, const T1& x_hi,
                        T2* lower_bound, T2* upper_bound) {
  if (x_lo < *lower_bound)
    *lower_bound = x_lo;
  if (x_hi > *upper_bound)
    *upper_bound = x_hi;
}

// Intersect the range [*lower2, *upper2] with the range [lower1, upper1],
// putting the result back in [*lower2, *upper2].
// If non-intersecting ranges are given, we end up with *lower2 > *upper2.
template<typename T>
inline void IntersectRange(const T& lower1, const T& upper1,
                           T* lower2, T* upper2) {
  if (lower1 > *lower2)
    *lower2 = lower1;
  if (upper1 < *upper2)
    *upper2 = upper1;
}

// Proper modulo arithmetic operator. Returns a mod b that works for -ve a.
// For any integer a and positive b, returns r : 0<=r<b and a=n*b + r for
// some integer n.
inline int Modulo(int a, int b) {
  return (a % b + b) % b;
}

// Integer division operator with rounding that works for negative input.
// Returns a divided by b, rounded to the nearest integer, without double
// counting at 0. With simple rounding 1/3 = 0, 0/3 = 0 -1/3 = 0, -2/3 = 0,
// -3/3 = 0 and -4/3 = -1.
// I want 1/3 = 0, 0/3 = 0, -1/3 = 0, -2/3 = -1, -3/3 = -1 and -4/3 = -1.
inline int DivRounded(int a, int b) {
  if (b < 0) return -DivRounded(a, -b);
  return a >= 0 ? (a + b / 2) / b : (a - b / 2) / b;
}

// Return a double cast to int with rounding.
inline int IntCastRounded(double x) {
  return x >= 0.0 ? static_cast<int>(x + 0.5) : -static_cast<int>(-x + 0.5);
}

// Reverse the order of bytes in a n byte quantity for big/little-endian switch.
inline void ReverseN(void* ptr, int num_bytes) {
  char *cptr = reinterpret_cast<char *>(ptr);
  int halfsize = num_bytes / 2;
  for (int i = 0; i < halfsize; ++i) {
    char tmp = cptr[i];
    cptr[i] = cptr[num_bytes - 1 - i];
    cptr[num_bytes - 1 - i] = tmp;
  }
}

// Reverse the order of bytes in a 16 bit quantity for big/little-endian switch.
inline void Reverse16(void *ptr) {
  ReverseN(ptr, 2);
}

// Reverse the order of bytes in a 32 bit quantity for big/little-endian switch.
inline void Reverse32(void *ptr) {
  ReverseN(ptr, 4);
}

// Reverse the order of bytes in a 64 bit quantity for big/little-endian switch.
inline void Reverse64(void* ptr) {
  ReverseN(ptr, 8);
}


#endif // TESSERACT_CCUTIL_HELPERS_H_
