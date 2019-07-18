// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: renn
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include <cctype>
#include <climits>      // for CHAR_BIT
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>       // for std::numeric_limits

#include "scanutils.h"

enum Flags {
  FL_SPLAT  = 0x01,   // Drop the value, do not assign
  FL_INV    = 0x02,   // Character-set with inverse
  FL_WIDTH  = 0x04,   // Field width specified
  FL_MINUS  = 0x08,   // Negative number
};

enum Ranks {
  RANK_CHAR = -2,
  RANK_SHORT  = -1,
  RANK_INT  = 0,
  RANK_LONG = 1,
  RANK_LONGLONG = 2,
  RANK_PTR      = std::numeric_limits<int>::max() // Special value used for pointers
};

const enum Ranks kMinRank = RANK_CHAR;
const enum Ranks kMaxRank = RANK_LONGLONG;

const enum Ranks kIntMaxRank = RANK_LONGLONG;
const enum Ranks kSizeTRank = RANK_LONG;
const enum Ranks kPtrDiffRank = RANK_LONG;

enum Bail {
  BAIL_NONE = 0,    // No error condition
  BAIL_EOF,         // Hit EOF
  BAIL_ERR          // Conversion mismatch
};

// Helper functions ------------------------------------------------------------
inline size_t LongBit() {
  return CHAR_BIT * sizeof(long);
}

static inline int
SkipSpace(FILE *s) {
  int p;
  while (isascii(p = fgetc(s)) && isspace(p));
  ungetc(p, s);  // Make sure next char is available for reading
  return p;
}

static inline void
SetBit(unsigned long *bitmap, unsigned int bit) {
  bitmap[bit/LongBit()] |= 1UL << (bit%LongBit());
}

static inline int
TestBit(unsigned long *bitmap, unsigned int bit) {
  return static_cast<int>(bitmap[bit/LongBit()] >> (bit%LongBit())) & 1;
}

static inline int DigitValue(int ch, int base) {
  if (ch >= '0' && ch <= '9') {
    if (base >= 10 || ch <= '7')
      return ch-'0';
  } else if (ch >= 'A' && ch <= 'Z' && base == 16) {
    return ch-'A'+10;
  } else if (ch >= 'a' && ch <= 'z' && base == 16) {
    return ch-'a'+10;
  }
  return -1;
}

// IO (re-)implementations -----------------------------------------------------
static uintmax_t streamtoumax(FILE* s, int base) {
  int minus = 0;
  uintmax_t v = 0;
  int d, c = 0;

  for (c = fgetc(s); isascii(c) && isspace(c); c = fgetc(s));

  // Single optional + or -
  if (c == '-' || c == '+') {
    minus = (c == '-');
    c = fgetc(s);
  }

  // Assign correct base
  if (base == 0) {
    if (c == '0') {
      c = fgetc(s);
      if (c == 'x' || c == 'X') {
        base = 16;
        c = fgetc(s);
      } else {
        base = 8;
      }
    }
  } else if (base == 16) {
    if (c == '0') {
      c = fgetc(s);
      if (c == 'x' || c == 'X') c = fgetc(s);
    }
  }

  // Actual number parsing
  for (; (c != EOF) && (d = DigitValue(c, base)) >= 0; c = fgetc(s))
    v = v*base + d;

  ungetc(c, s);
  return minus ? -v : v;
}

static double streamtofloat(FILE* s) {
  bool minus = false;
  uint64_t v = 0;
  int d, c;
  uint64_t k = 1;
  uint64_t w = 0;

  for (c = fgetc(s); isascii(c) && isspace(c); c = fgetc(s));

  // Single optional + or -
  if (c == '-' || c == '+') {
    minus = (c == '-');
    c = fgetc(s);
  }

  // Actual number parsing
  for (; c != EOF && (d = DigitValue(c, 10)) >= 0; c = fgetc(s))
    v = v*10 + d;
  if (c == '.') {
    for (c = fgetc(s); c != EOF && (d = DigitValue(c, 10)) >= 0; c = fgetc(s)) {
      w = w*10 + d;
      k *= 10;
    }
  }
  double f = v + static_cast<double>(w) / k;
  if (c == 'e' || c == 'E') {
    c = fgetc(s);
    int expsign = 1;
    if (c == '-' || c == '+') {
      expsign = (c == '-') ? -1 : 1;
      c = fgetc(s);
    }
    int exponent = 0;
    for (; (c != EOF) && (d = DigitValue(c, 10)) >= 0; c = fgetc(s)) {
      exponent = exponent * 10 + d;
    }
    exponent *= expsign;
    f *= pow(10.0, static_cast<double>(exponent));
  }
  ungetc(c, s);

  return minus ? -f : f;
}

static int tvfscanf(FILE* stream, const char *format, va_list ap);

int tfscanf(FILE* stream, const char *format, ...) {
  va_list ap;
  int rv;

  va_start(ap, format);
  rv = tvfscanf(stream, format, ap);
  va_end(ap);

  return rv;
}

static int tvfscanf(FILE* stream, const char *format, va_list ap) {
  const char *p = format;
  char ch;
  int q = 0;
  uintmax_t val = 0;
  int rank = RANK_INT;    // Default rank
  unsigned int width = UINT_MAX;
  int base;
  int flags = 0;
  enum {
    ST_NORMAL,        // Ground state
    ST_FLAGS,         // Special flags
    ST_WIDTH,         // Field width
    ST_MODIFIERS,     // Length or conversion modifiers
    ST_MATCH_INIT,    // Initial state of %[ sequence
    ST_MATCH,         // Main state of %[ sequence
    ST_MATCH_RANGE,   // After - in a %[ sequence
  } state = ST_NORMAL;
  char *sarg = nullptr;    // %s %c or %[ string argument
  enum Bail bail = BAIL_NONE;
  int converted = 0;    // Successful conversions
  unsigned long matchmap[((1 << CHAR_BIT)+(CHAR_BIT * sizeof(long) - 1)) /
      (CHAR_BIT * sizeof(long))];
  int matchinv = 0;   // Is match map inverted?
  unsigned char range_start = 0;
  auto start_off = std::ftell(stream);

  // Skip leading spaces
  SkipSpace(stream);

  while ((ch = *p++) && !bail) {
    switch (state) {
      case ST_NORMAL:
        if (ch == '%') {
          state = ST_FLAGS;
          flags = 0; rank = RANK_INT; width = UINT_MAX;
        } else if (isascii(ch) && isspace(ch)) {
          SkipSpace(stream);
        } else {
          if (fgetc(stream) != ch)
            bail = BAIL_ERR;  // Match failure
        }
        break;

      case ST_FLAGS:
        if (ch == '*') {
          flags |= FL_SPLAT;
        } else if ('0' <= ch && ch <= '9') {
          width = (ch-'0');
          state = ST_WIDTH;
          flags |= FL_WIDTH;
        } else {
          state = ST_MODIFIERS;
          p--;      // Process this character again
        }
      break;

      case ST_WIDTH:
        if (ch >= '0' && ch <= '9') {
          width = width*10+(ch-'0');
        } else {
          state = ST_MODIFIERS;
          p--;      // Process this character again
        }
      break;

      case ST_MODIFIERS:
        switch (ch) {
          // Length modifiers - nonterminal sequences
          case 'h':
            rank--;     // Shorter rank
          break;
          case 'l':
            rank++;     // Longer rank
          break;
          case 'j':
            rank = kIntMaxRank;
          break;
          case 'z':
            rank = kSizeTRank;
          break;
          case 't':
            rank = kPtrDiffRank;
          break;
          case 'L':
          case 'q':
            rank = RANK_LONGLONG; // long double/long long
          break;

          default:
            // Output modifiers - terminal sequences
            state = ST_NORMAL;  // Next state will be normal
            if (rank < kMinRank)  // Canonicalize rank
              rank = kMinRank;
            else if (rank > kMaxRank)
              rank = kMaxRank;

          switch (ch) {
            case 'P':   // Upper case pointer
            case 'p':   // Pointer
              rank = RANK_PTR;
              base = 0;
              goto scan_int;

            case 'i':   // Base-independent integer
              base = 0;
              goto scan_int;

            case 'd':   // Decimal integer
              base = 10;
              goto scan_int;

            case 'o':   // Octal integer
              base = 8;
              goto scan_int;

            case 'u':   // Unsigned decimal integer
              base = 10;
              goto scan_int;

            case 'x':   // Hexadecimal integer
            case 'X':
              base = 16;
              goto scan_int;

            case 'n':   // Number of characters consumed
              val = std::ftell(stream) - start_off;
            goto set_integer;

            scan_int:
              q = SkipSpace(stream);
              if (q <= 0) {
                bail = BAIL_EOF;
                break;
              }
              val = streamtoumax(stream, base);
              // fall through

            set_integer:
              if (!(flags & FL_SPLAT)) {
                converted++;
                switch(rank) {
                  case RANK_CHAR:
                    *va_arg(ap, unsigned char *)
                      = static_cast<unsigned char>(val);
                  break;
                  case RANK_SHORT:
                    *va_arg(ap, unsigned short *)
                      = static_cast<unsigned short>(val);
                  break;
                  case RANK_INT:
                    *va_arg(ap, unsigned int *)
                      = static_cast<unsigned int>(val);
                  break;
                  case RANK_LONG:
                    *va_arg(ap, unsigned long *)
                      = static_cast<unsigned long>(val);
                  break;
                  case RANK_LONGLONG:
                    *va_arg(ap, unsigned long long *)
                      = static_cast<unsigned long long>(val);
                  break;
                  case RANK_PTR:
                    *va_arg(ap, void **)
                      = reinterpret_cast<void *>(static_cast<uintptr_t>(val));
                  break;
                }
              }
            break;

            case 'f':   // Preliminary float value parsing
            case 'g':
            case 'G':
            case 'e':
            case 'E':
              q = SkipSpace(stream);
              if (q <= 0) {
                bail = BAIL_EOF;
                break;
              }

              {
              double fval = streamtofloat(stream);
              if (!(flags & FL_SPLAT)) {
                if (rank == RANK_INT)
                  *va_arg(ap, float *) = static_cast<float>(fval);
                else if (rank == RANK_LONG)
                  *va_arg(ap, double *) = static_cast<double>(fval);
                converted++;
              }
              }
            break;

            case 'c':               // Character
              width = (flags & FL_WIDTH) ? width : 1; // Default width == 1
              sarg = va_arg(ap, char *);
              while (width--) {
                if ((q = fgetc(stream)) <= 0) {
                  bail = BAIL_EOF;
                  break;
                }
                if (!(flags & FL_SPLAT)) {
                  *sarg++ = q;
                  converted++;
                }
              }
            break;

            case 's':               // String
            {
              if (!(flags & FL_SPLAT)) {
                sarg = va_arg(ap, char *);
              }
              unsigned length = 0;
              while (width--) {
                q = fgetc(stream);
                if ((isascii(q) && isspace(q)) || (q <= 0)) {
                  ungetc(q, stream);
                  break;
                }
                if (!(flags & FL_SPLAT)) {
                  sarg[length] = q;
                }
                length++;
              }
              if (length == 0) {
                bail = BAIL_EOF;
              } else if (!(flags & FL_SPLAT)) {
                sarg[length] = '\0'; // Terminate output
                converted++;
              }
            }
            break;

            case '[':   // Character range
              sarg = va_arg(ap, char *);
              state = ST_MATCH_INIT;
              matchinv = 0;
              memset(matchmap, 0, sizeof matchmap);
            break;

            case '%':   // %% sequence
              if (fgetc(stream) != '%')
                bail = BAIL_ERR;
            break;

            default:    // Anything else
              bail = BAIL_ERR;  // Unknown sequence
            break;
          }
        }
      break;

      case ST_MATCH_INIT:   // Initial state for %[ match
        if (ch == '^' && !(flags & FL_INV)) {
          matchinv = 1;
        } else {
          SetBit(matchmap, static_cast<unsigned char>(ch));
          state = ST_MATCH;
        }
      break;

      case ST_MATCH:    // Main state for %[ match
        if (ch == ']') {
          goto match_run;
        } else if (ch == '-') {
          range_start = static_cast<unsigned char>(ch);
          state = ST_MATCH_RANGE;
        } else {
          SetBit(matchmap, static_cast<unsigned char>(ch));
        }
      break;

      case ST_MATCH_RANGE:    // %[ match after -
        if (ch == ']') {
          SetBit(matchmap, static_cast<unsigned char>('-'));
          goto match_run;
        } else {
          int i;
          for (i = range_start ; i < (static_cast<unsigned char>(ch)) ; i++)
          SetBit(matchmap, i);
          state = ST_MATCH;
        }
      break;

      match_run:      // Match expression finished
        char* oarg = sarg;
        while (width) {
          q = fgetc(stream);
          auto qc = static_cast<unsigned char>(q);
          if (q <= 0 || !(TestBit(matchmap, qc)^matchinv)) {
            ungetc(q, stream);
            break;
          }
          if (!(flags & FL_SPLAT)) *sarg = q;
          sarg++;
        }
        if (oarg == sarg) {
          bail = (q <= 0) ? BAIL_EOF : BAIL_ERR;
        } else if (!(flags & FL_SPLAT)) {
          *sarg = '\0';
          converted++;
        }
      break;
    }
  }

  if (bail == BAIL_EOF && !converted)
    converted = -1;   // Return EOF (-1)

  return converted;
}
