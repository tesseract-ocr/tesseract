/******************************************************************************
 *
 * File:         helpers.h
 * Description:  General utility functions
 * Author:       Daria Antonova
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
 *****************************************************************************/

#ifndef TESSERACT_CCUTIL_HELPERS_H_
#define TESSERACT_CCUTIL_HELPERS_H_

#include <cassert>
#include <climits> // for INT_MIN, INT_MAX
#include <cmath> // std::isfinite
#include <cstdio>
#include <cstring>
#include <algorithm>  // for std::find
#include <functional>
#include <random>
#include <string>
#include <vector>

#include "serialis.h"

namespace tesseract {

// Copy a std::string to a newly allocated char *.
// TODO: Remove this function once the related code has been converted
// to use std::string.
inline char *copy_string(const std::string &from) {
  auto length = from.length();
  char *target_string = new char[length + 1];
  from.copy(target_string, length);
  target_string[length] = '\0';
  return target_string;
}

template <class T>
inline bool contains(const std::vector<T> &data, const T &value) {
  return std::find(data.begin(), data.end(), value) != data.end();
}

inline const std::vector<std::string> split(const std::string &s, char c) {
  std::string buff;
  std::vector<std::string> v;
  for (auto n : s) {
    if (n != c) {
      buff += n;
    } else if (n == c && !buff.empty()) {
      v.push_back(buff);
      buff.clear();
    }
  }
  if (!buff.empty()) {
    v.push_back(buff);
  }
  return v;
}

// A simple linear congruential random number generator.
class TRand {
public:
  // Sets the seed to the given value.
  void set_seed(uint64_t seed) {
    e.seed(seed);
  }
  // Sets the seed using a hash of a string.
  void set_seed(const std::string &str) {
    std::hash<std::string> hasher;
    set_seed(static_cast<uint64_t>(hasher(str)));
  }

  // Returns an integer in the range 0 to INT32_MAX.
  int32_t IntRand() {
    return e();
  }
  // Returns a floating point value in the range [-range, range].
  double SignedRand(double range) {
    return range * 2.0 * IntRand() / INT32_MAX - range;
  }
  // Returns a floating point value in the range [0, range].
  double UnsignedRand(double range) {
    return range * IntRand() / INT32_MAX;
  }

private:
  std::minstd_rand e;
};

// Remove newline (if any) at the end of the string.
inline void chomp_string(char *str) {
  int last_index = static_cast<int>(strlen(str)) - 1;
  while (last_index >= 0 && (str[last_index] == '\n' || str[last_index] == '\r')) {
    str[last_index--] = '\0';
  }
}

// return the smallest multiple of block_size greater than or equal to n.
inline int RoundUp(int n, int block_size) {
  return block_size * ((n + block_size - 1) / block_size);
}

// Clip a numeric value to the interval [lower_bound, upper_bound].
template <typename T>
inline T ClipToRange(const T &x, const T &lower_bound, const T &upper_bound) {
  if (x < lower_bound) {
    return lower_bound;
  }
  if (x > upper_bound) {
    return upper_bound;
  }
  return x;
}

// Extend the range [lower_bound, upper_bound] to include x.
template <typename T1, typename T2>
inline void UpdateRange(const T1 &x, T2 *lower_bound, T2 *upper_bound) {
  if (x < *lower_bound) {
    *lower_bound = x;
  }
  if (x > *upper_bound) {
    *upper_bound = x;
  }
}

// Decrease lower_bound to be <= x_lo AND increase upper_bound to be >= x_hi.
template <typename T1, typename T2>
inline void UpdateRange(const T1 &x_lo, const T1 &x_hi, T2 *lower_bound, T2 *upper_bound) {
  if (x_lo < *lower_bound) {
    *lower_bound = x_lo;
  }
  if (x_hi > *upper_bound) {
    *upper_bound = x_hi;
  }
}

// Intersect the range [*lower2, *upper2] with the range [lower1, upper1],
// putting the result back in [*lower2, *upper2].
// If non-intersecting ranges are given, we end up with *lower2 > *upper2.
template <typename T>
inline void IntersectRange(const T &lower1, const T &upper1, T *lower2, T *upper2) {
  if (lower1 > *lower2) {
    *lower2 = lower1;
  }
  if (upper1 < *upper2) {
    *upper2 = upper1;
  }
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
  if (b < 0) {
    return -DivRounded(a, -b);
  }
  return a >= 0 ? (a + b / 2) / b : (a - b / 2) / b;
}

// Return a double cast to int with rounding.
inline int IntCastRounded(double x) {
  assert(std::isfinite(x));
  assert(x < INT_MAX);
  assert(x > INT_MIN);
  return x >= 0.0 ? static_cast<int>(x + 0.5) : -static_cast<int>(-x + 0.5);
}

// Return a float cast to int with rounding.
inline int IntCastRounded(float x) {
  assert(std::isfinite(x));
  return x >= 0.0F ? static_cast<int>(x + 0.5F) : -static_cast<int>(-x + 0.5F);
}

// Reverse the order of bytes in a n byte quantity for big/little-endian switch.
inline void ReverseN(void *ptr, int num_bytes) {
  assert(num_bytes == 1 || num_bytes == 2 || num_bytes == 4 || num_bytes == 8);
  char *cptr = static_cast<char *>(ptr);
  int halfsize = num_bytes / 2;
  for (int i = 0; i < halfsize; ++i) {
    char tmp = cptr[i];
    cptr[i] = cptr[num_bytes - 1 - i];
    cptr[num_bytes - 1 - i] = tmp;
  }
}

// Reverse the order of bytes in a 32 bit quantity for big/little-endian switch.
inline void Reverse32(void *ptr) {
  ReverseN(ptr, 4);
}

// Reads a vector of simple types from the given file. Assumes that bitwise
// read/write will work with ReverseN according to sizeof(T).
// Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
template <typename T>
bool DeSerialize(bool swap, FILE *fp, std::vector<T> &data) {
  uint32_t size;
  if (fread(&size, sizeof(size), 1, fp) != 1) {
    return false;
  }
  if (swap) {
    Reverse32(&size);
  }
  // Arbitrarily limit the number of elements to protect against bad data.
  assert(size <= UINT16_MAX);
  if (size > UINT16_MAX) {
    return false;
  }
  // TODO: optimize.
  data.resize(size);
  if (size > 0) {
    if (fread(&data[0], sizeof(T), size, fp) != size) {
      return false;
    }
    if (swap) {
      for (uint32_t i = 0; i < size; ++i) {
        ReverseN(&data[i], sizeof(T));
      }
    }
  }
  return true;
}

// Writes a vector of simple types to the given file. Assumes that bitwise
// read/write of T will work. Returns false in case of error.
template <typename T>
bool Serialize(FILE *fp, const std::vector<T> &data) {
  uint32_t size = data.size();
  if (fwrite(&size, sizeof(size), 1, fp) != 1) {
    return false;
  } else if constexpr (std::is_class<T>::value) {
    // Serialize a tesseract class.
    for (auto &item : data) {
      if (!item.Serialize(fp)) {
        return false;
      }
    }
  } else if constexpr (std::is_pointer<T>::value) {
    // Serialize pointers.
    for (auto &item : data) {
      uint8_t non_null = (item != nullptr);
      if (!Serialize(fp, &non_null)) {
        return false;
      }
      if (non_null) {
        if (!item->Serialize(fp)) {
          return false;
        }
      }
    }
  } else if (size > 0) {
    if (fwrite(&data[0], sizeof(T), size, fp) != size) {
      return false;
    }
  }
  return true;
}

} // namespace tesseract

#endif // TESSERACT_CCUTIL_HELPERS_H_
