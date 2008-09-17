// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Local includes
#include "debugging.h"
#include "mathfunctions.h"

// C includes
#include <math.h>
#include <stdlib.h>

unsigned helium::SquaredRoot(unsigned x) {
  unsigned k = 0, c = 0;
  for(unsigned o = 1; o <= x; o += k) {
    k += 2;
    c++;
  }
  return c;
}

unsigned helium::Binomi(unsigned n, int k) {
  return ((n == k) || (k <= 1)) ? 1 : Binomi(n-1, k-1) + Binomi(n-1, k);
}

// This comparison function is required for qsort(...), which is used in
// the Median(...) function below.
int float_compare(const void* a, const void* b) {
  const float* f_a = reinterpret_cast<const float*>(a);
  const float* f_b = reinterpret_cast<const float*>(b);
  return (*f_a > *f_b) ? 1 : ((*f_a < *f_b) ? -1 : 0);
}

float helium::Median(float* values, unsigned size) {
  qsort(values, size, sizeof(float), float_compare);
  return values[size / 2];
}

// I feel the need to defend the rare case of using macros here:
// To avoid having to actually swap a row value-by-value in memory, an internal 
// list of n values is stored that map each index to the position of the row. 
// To access an element in the matrix, a look-up must be made to access the
// correct row. These macros should simplify this access, and make the code
// more readable.
#define ELEM_A(ROW, COL) A[rows[ROW] * n + COL]
#define ELEM_B(ROW) b[rows[ROW]]

bool helium::Gauss(float* A, float* b, float* x, unsigned n) {
  float tmp_x[n];
  unsigned rows[n];
  for (unsigned i = 0; i < n; i++) rows[i] = i;
  
  // Diagonalization
  for (unsigned i = 0; i < n; i++) {
    // Find pivot
    float max_val = fabs(ELEM_A(i, i));
    float cur_val = 0.0;
    
    unsigned r = i;
    for (unsigned k = i + 1; k < n; k++) { 
      cur_val = fabs(ELEM_A(k, i));
      if (cur_val > max_val) {
        max_val = cur_val;
        r = k;
      }
    }
    
    // Swap rows to make pivot the current row
    unsigned tmp = rows[r];
    rows[r] = rows[i];
    rows[i] = tmp;
    
    if (max_val == 0.0) return false; 
    
    // Scale row and subtract
    for (unsigned k = i + 1; k < n; k++) {
      float cur_factor =  ELEM_A(k, i) / max_val;
      for (unsigned j = i; j < n; j++) 
        ELEM_A(k, j) = ELEM_A(k, j) - ELEM_A(i, j) * cur_factor; 
      ELEM_B(k) = ELEM_B(k) - ELEM_B(i) * cur_factor;
    }
  }
  
  // Back substitution
  for (int i = n - 1; i >= 0; i--) {
    tmp_x[rows[i]] = ELEM_B(i);
    for (unsigned j = n - 1; j > i; j--) 
      tmp_x[rows[i]] -= tmp_x[rows[j]] * ELEM_A(i, j);
    tmp_x[rows[i]] /= ELEM_A(i, i);
  }
  
  // Swap back values
  for (int i = 0; i < n; i++) x[i] = tmp_x[rows[i]];
  
  return true;
}
