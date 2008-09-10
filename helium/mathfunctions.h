// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file declares all mathematical functions, that are used in Helium.
// They range from extremely simple (Square(int) function) to moderately
// complex (Gauss(...) linear equation solver).
//
#ifndef HELIUM_MATHFUNCTIONS_H__
#define HELIUM_MATHFUNCTIONS_H__

namespace helium {

// The Square function is mainly used for cosmetic purposes on large 
// expressions that need to be multiplied with themselves. Note that this
// implementation works only on 'int' to avoid overflow errors when passing
// a more limited type.
inline int Square(int x) {
  return x * x;
}

// A floating point version of the Square function.
inline float Square(float x) {
  return x * x;
}

// Returns the minumum of the two values. This function is mainly
// available for readability.
template<typename T>
inline T Min(T a, T b) {
  return (a < b) ? a : b;
}

// Returns the maximum of the two values. This function is mainly
// available for readability.
template<typename T>
inline T Max(T a, T b) {
  return (a > b) ? a : b;
}

// Returns the minumum of the three values.
inline int Min3(int a, int b, int c) {
  return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
}

// Returns the maximum of the three values.
inline int Max3(int a, int b, int c) {
  return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}

// This function calculates the squared-root of the given integer argument,
// using only additions. For floating point calculations, use the sqrt(...)
// family of functions, defined in <math.h>.
unsigned SquaredRoot(unsigned x);

// Typical recursive function to calculate the binomial coefficient of
// the natural number n and the integer k.
unsigned Binomi(unsigned n, int k);

// Calculates the median value of the given values. Note that the values will
// be sorted after calling this function!
float Median(float* values, unsigned size);

// Solves the linear system A * x = b, where A is an n x n matrix and x, b
// are vectors of size n. All vector and matrix values must be passed as simple
// 1-dimensional C-arrays. The array x must be large enough to hold n output
// values. Gauss with pivoting is used to enhance robustness.
bool Gauss(float* A, float* b, float* x, unsigned n);

} // namespace

#endif  // HELIUM_MATHFUNCTIONS_H__
