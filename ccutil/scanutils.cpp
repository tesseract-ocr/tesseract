// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: renn
//
// The fscanf, vfscanf and creat functions are implemented so that their
// functionality is mostly like their stdio counterparts. However, currently
// these functions do not use any buffering, making them rather slow.
// File streams are thus processed one character at a time.
// Although the implementations of the scanf functions do lack a few minor
// features, they should be sufficient for their use in tesseract.
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

#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "scanutils.h"
#include "tprintf.h"

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
  RANK_PTR      = INT_MAX // Special value used for pointers
//  RANK_PTR      = 3 // Special value used for pointers
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
SkipSpace(FILE *s)
{
  int p;
  while (isspace(p = fgetc(s)));
  ungetc(p, s);  // Make sure next char is available for reading
  return p;
}

static inline void
SetBit(unsigned long *bitmap, unsigned int bit)
{
  bitmap[bit/LongBit()] |= 1UL << (bit%LongBit());
}

static inline int
TestBit(unsigned long *bitmap, unsigned int bit)
{
  return static_cast<int>(bitmap[bit/LongBit()] >> (bit%LongBit())) & 1;
}

static inline int DigitValue(int ch)
{
  if (ch >= '0' && ch <= '9') {
    return ch-'0';
  } else if (ch >= 'A' && ch <= 'Z') {
    return ch-'A'+10;
  } else if (ch >= 'a' && ch <= 'z') {
    return ch-'a'+10;
  } else {
    return -1;
  }
}

// IO (re-)implementations -----------------------------------------------------
uintmax_t streamtoumax(FILE* s, int base)
{
  int minus = 0;
  uintmax_t v = 0;
  int d, c = 0;

  for (c = fgetc(s);
    isspace(static_cast<unsigned char>(c)) && (c != EOF);
    c = fgetc(s))

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
      if (c == 'x' && c == 'X') c = fgetc(s);
    }
  }

  // Actual number parsing
  for (; (c != EOF) && (d = DigitValue(c)) >= 0 && d < base; c = fgetc(s))
    v = v*base + d;

  ungetc(c, s);
  return minus ? -v : v;
}

double streamtofloat(FILE* s)
{
  int minus = 0;
  int v = 0;
  int d, c = 0;
  int k = 1;
  int w = 0;

  for (c = fgetc(s);
    isspace(static_cast<unsigned char>(c)) && (c != EOF);
    c = fgetc(s));

  // Single optional + or -
  if (c == '-' || c == '+') {
    minus = (c == '-');
    c = fgetc(s);
  }

  // Actual number parsing
  for (; (c != EOF) && (d = DigitValue(c)) >= 0; c = fgetc(s))
    v = v*10 + d;
  if (c == '.') {
    for (c = fgetc(s); (c != EOF) && (d = DigitValue(c)) >= 0; c = fgetc(s)) {
      w = w*10 + d;
      k *= 10;
    }
  } else if (c == 'e' || c == 'E')
    tprintf("WARNING: Scientific Notation not supported!");

  ungetc(c, s);
  double f  = static_cast<double>(v)
            + static_cast<double>(w) / static_cast<double>(k);

  return minus ? -f : f;
}

double strtofloat(const char* s)
{
  int minus = 0;
  int v = 0;
  int d;
  int k = 1;
  int w = 0;

  while(*s && isspace(static_cast<unsigned char>(*s))) s++;

  // Single optional + or -
  if (*s == '-' || *s == '+') {
    minus = (*s == '-');
    s++;
  }

  // Actual number parsing
  for (; *s && (d = DigitValue(*s)) >= 0; s++)
    v = v*10 + d;
  if (*s == '.') {
    for (++s; *s && (d = DigitValue(*s)) >= 0; s++) {
      w = w*10 + d;
      k *= 10;
    }
  } else if (*s == 'e' || *s == 'E')
    tprintf("WARNING: Scientific Notation not supported!");

  double f  = static_cast<double>(v)
            + static_cast<double>(w) / static_cast<double>(k);

  return minus ? -f : f;
}

static int tess_vfscanf(FILE* stream, const char *format, va_list ap);

int tess_fscanf(FILE* stream, const char *format, ...)
{
  va_list ap;
  int rv;

  va_start(ap, format);
  rv = tess_vfscanf(stream, format, ap);
  va_end(ap);

  return rv;
}

#ifdef EMBEDDED
int fscanf(FILE* stream, const char *format, ...)
{
  va_list ap;
  int rv;

  va_start(ap, format);
  rv = tess_vfscanf(stream, format, ap);
  va_end(ap);

  return rv;
}

int vfscanf(FILE* stream, const char *format, ...)
{
  va_list ap;
  int rv;

  va_start(ap, format);
  rv = tess_vfscanf(stream, format, ap);
  va_end(ap);

  return rv;
}
#endif

#ifndef _MSV_VER
static
int tess_vfscanf(FILE* stream, const char *format, va_list ap)
{
  const char *p = format;
  char ch;
  int q = 0;
  uintmax_t val = 0;
  int rank = RANK_INT;    // Default rank
  unsigned int width = ~0;
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
  char *sarg = NULL;    // %s %c or %[ string argument
  enum Bail bail = BAIL_NONE;
  int sign;
  int converted = 0;    // Successful conversions
  unsigned long matchmap[((1 << CHAR_BIT)+(LongBit()-1))/LongBit()];
  int matchinv = 0;   // Is match map inverted?
  unsigned char range_start = 0;
  off_t start_off = ftell(stream);
  double fval;

  // Skip leading spaces
  SkipSpace(stream);

  while ((ch = *p++) && !bail) {
    switch (state) {
      case ST_NORMAL:
        if (ch == '%') {
          state = ST_FLAGS;
          flags = 0; rank = RANK_INT; width = ~0;
        } else if (isspace(static_cast<unsigned char>(ch))) {
          SkipSpace(stream);
        } else {
          if (fgetc(stream) != ch)
            bail = BAIL_ERR;  // Match failure
        }
        break;

      case ST_FLAGS:
        switch (ch) {
          case '*':
            flags |= FL_SPLAT;
          break;

          case '0' ... '9':
            width = (ch-'0');
            state = ST_WIDTH;
            flags |= FL_WIDTH;
          break;

          default:
            state = ST_MODIFIERS;
            p--;      // Process this character again
          break;
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
              base = 0; sign = 0;
            goto scan_int;

            case 'i':   // Base-independent integer
              base = 0; sign = 1;
            goto scan_int;

            case 'd':   // Decimal integer
              base = 10; sign = 1;
            goto scan_int;

            case 'o':   // Octal integer
              base = 8; sign = 0;
            goto scan_int;

            case 'u':   // Unsigned decimal integer
              base = 10; sign = 0;
            goto scan_int;

            case 'x':   // Hexadecimal integer
            case 'X':
              base = 16; sign = 0;
            goto scan_int;

            case 'n':   // Number of characters consumed
              val = ftell(stream) - start_off;
            goto set_integer;

            scan_int:
              q = SkipSpace(stream);
              if ( q <= 0 ) {
                bail = BAIL_EOF;
                break;
              }
              val = streamtoumax(stream, base);
              converted++;
              // fall through

            set_integer:
              if (!(flags & FL_SPLAT)) {
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

              fval = streamtofloat(stream);
              switch(rank) {
                case RANK_INT:
                  *va_arg(ap, float *) = static_cast<float>(fval);
                break;
                case RANK_LONG:
                  *va_arg(ap, double *) = static_cast<double>(fval);
                break;
              }
              converted++;
            break;

            case 'c':               // Character
              width = (flags & FL_WIDTH) ? width : 1; // Default width == 1
              sarg = va_arg(ap, char *);
              while (width--) {
                if ((q = fgetc(stream)) <= 0) {
                  bail = BAIL_EOF;
                  break;
                }
                *sarg++ = q;
              }
              if (!bail)
                converted++;
            break;

            case 's':               // String
            {
              char *sp;
              sp = sarg = va_arg(ap, char *);
              while (width--) {
                q = fgetc(stream);
                if (isspace(static_cast<unsigned char>(q)) || q <= 0) {
                  ungetc(q, stream);
                  break;
                }
                *sp++ = q;
              }
              if (sarg != sp) {
                *sp = '\0'; // Terminate output
                converted++;
              } else {
                bail = BAIL_EOF;
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
              if (fgetc(stream) != '%' )
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
          unsigned char qc = static_cast<unsigned char>(q);
          if (q <= 0 || !(TestBit(matchmap, qc)^matchinv)) {
            ungetc(q, stream);
            break;
          }
          *sarg++ = q;
        }
        if (oarg != sarg) {
          *sarg = '\0';
          converted++;
        } else {
          bail = (q <= 0) ? BAIL_EOF : BAIL_ERR;
        }
      break;
    }
  }

  if (bail == BAIL_EOF && !converted)
    converted = -1;   // Return EOF (-1)

  return converted;
}
#endif

#ifdef EMBEDDED
int creat(const char *pathname, mode_t mode)
{
  return open(pathname, O_CREAT | O_TRUNC | O_WRONLY, mode);
}
#endif
