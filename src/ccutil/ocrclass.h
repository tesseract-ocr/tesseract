/**********************************************************************
 * File:        ocrclass.h
 * Description: Class definitions and constants for the OCR API.
 * Author:          Hewlett-Packard Co
 *
 * (C) Copyright 1996, Hewlett-Packard Co.
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
 **********************************************************************/

/**********************************************************************
 * This file contains typedefs for all the structures used by
 * the HP OCR interface.
 * The code is designed to be used with either a C or C++ compiler.
 * The structures are designed to allow them to be used with any
 * structure alignment up to 8.
 **********************************************************************/

#ifndef            CCUTIL_OCRCLASS_H_
#define            CCUTIL_OCRCLASS_H_

#ifndef __GNUC__
#ifdef _WIN32
#include          "gettimeofday.h"
#endif
#else
#include          <sys/time.h>
#endif
#include          <ctime>
#include          "host.h"

/*Maximum lengths of various strings*/
#define MAX_FONT_NAME   34       /*name of font */
#define MAX_OCR_NAME    32       /*name of engine */
#define MAX_OCR_VERSION   17     /*version code of engine */

/*pitch set definitions are identical to RTF*/
#define PITCH_DEF     0          /*default */
#define PITCH_FIXED     1        /*fixed pitch */
#define PITCH_VAR     2          /*variable pitch */

/**********************************************************************
 * EANYCODE_CHAR
 * Description of a single character. The character code is defined by
 * the character set of the current font.
 * Output text is sent as an array of these structures.
 * Spaces and line endings in the output are represented in the
 * structures of the surrounding characters. They are not directly
 * represented as characters.
 * The first character in a word has a positive value of blanks.
 * Missing information should be set to the defaults in the comments.
 * If word bounds are known, but not character bounds, then the top and
 * bottom of each character should be those of the word. The left of the
 * first and right of the last char in each word should be set. All other
 * lefts and rights should be set to -1.
 * If set, the values of right and bottom are left+width and top+height.
 * Most of the members come directly from the parameters to ocr_append_char.
 * The formatting member uses the enhancement parameter and combines the
 * line direction stuff into the top 3 bits.
 * The coding is 0=RL char, 1=LR char, 2=DR NL, 3=UL NL, 4=DR Para,
 * 5=UL Para, 6=TB char, 7=BT char. API users do not need to know what
 * the coding is, only that it is backwards compatible with the previous
 * version.
 **********************************************************************/

typedef struct {                  /*single character */
// It should be noted that the format for char_code for version 2.0 and beyond
// is UTF8 which means that ASCII characters will come out as one structure but
// other characters will be returned in two or more instances of this structure
// with a single byte of the  UTF8 code in each, but each will have the same
// bounding box. Programs which want to handle languagues with different
// characters sets will need to handle extended characters appropriately, but
// *all* code needs to be prepared to receive UTF8 coded characters for
// characters such as bullet and fancy quotes.
  uint16_t char_code;              /*character itself */
  int16_t left;                    /*of char (-1) */
  int16_t right;                   /*of char (-1) */
  int16_t top;                     /*of char (-1) */
  int16_t bottom;                  /*of char (-1) */
  int16_t font_index;              /*what font (0) */
  uint8_t confidence;              /*0=perfect, 100=reject (0/100) */
  uint8_t point_size;              /*of char, 72=i inch, (10) */
  int8_t blanks;                   /*no of spaces before this char (1) */
  uint8_t formatting;              /*char formatting (0) */
} EANYCODE_CHAR;                 /*single character */

/**********************************************************************
 * ETEXT_DESC
 * Description of the output of the OCR engine.
 * This structure is used as both a progress monitor and the final
 * output header, since it needs to be a valid progress monitor while
 * the OCR engine is storing its output to shared memory.
 * During progress, all the buffer info is -1.
 * Progress starts at 0 and increases to 100 during OCR. No other constraint.
 * Additionally the progress callback contains the bounding box of the word that
 * is currently being processed.
 * Every progress callback, the OCR engine must set ocr_alive to 1.
 * The HP side will set ocr_alive to 0. Repeated failure to reset
 * to 1 indicates that the OCR engine is dead.
 * If the cancel function is not null then it is called with the number of
 * user words found. If it returns true then operation is cancelled.
 **********************************************************************/
class ETEXT_DESC;

typedef bool (*CANCEL_FUNC)(void* cancel_this, int words);
typedef bool (*PROGRESS_FUNC)(int progress, int left, int right, int top,
                              int bottom);
typedef bool (*PROGRESS_FUNC2)(ETEXT_DESC* ths, int left, int right, int top,
                               int bottom);

class ETEXT_DESC {             // output header
 public:
  int16_t count;     /// chars in this buffer(0)
  int16_t progress;  /// percent complete increasing (0-100)
  /** Progress monitor covers word recognition and it does not cover layout
  * analysis.
  * See Ray comment in https://github.com/tesseract-ocr/tesseract/pull/27 */
  int8_t more_to_come;                /// true if not last
  volatile int8_t ocr_alive;          /// ocr sets to 1, HP 0
  int8_t err_code;                    /// for errcode use
  CANCEL_FUNC cancel;               /// returns true to cancel
  PROGRESS_FUNC progress_callback;  /// called whenever progress increases
  PROGRESS_FUNC2 progress_callback2;/// monitor-aware progress callback
  void* cancel_this;                /// this or other data for cancel
  struct timeval end_time;          /// Time to stop. Expected to be set only
                                    /// by call to set_deadline_msecs().
  EANYCODE_CHAR text[1];            /// character data

  ETEXT_DESC()
      : count(0),
        progress(0),
        more_to_come(0),
        ocr_alive(0),
        err_code(0),
        cancel(nullptr),
        progress_callback(nullptr),
        progress_callback2(&default_progress_func),
        cancel_this(nullptr) {
    end_time.tv_sec = 0;
    end_time.tv_usec = 0;
  }

  // Sets the end time to be deadline_msecs milliseconds from now.
  void set_deadline_msecs(int32_t deadline_msecs) {
    gettimeofday(&end_time, nullptr);
    int32_t deadline_secs = deadline_msecs / 1000;
    end_time.tv_sec += deadline_secs;
    end_time.tv_usec += (deadline_msecs -  deadline_secs * 1000) * 1000;
    if (end_time.tv_usec > 1000000) {
      end_time.tv_usec -= 1000000;
      ++end_time.tv_sec;
    }
  }

  // Returns false if we've not passed the end_time, or have not set a deadline.
  bool deadline_exceeded() const {
    if (end_time.tv_sec == 0 && end_time.tv_usec == 0) return false;
    struct timeval now;
    gettimeofday(&now, nullptr);
    return (now.tv_sec > end_time.tv_sec || (now.tv_sec == end_time.tv_sec &&
                                             now.tv_usec > end_time.tv_usec));
  }

private:
  static bool default_progress_func(ETEXT_DESC* ths, int left, int right, int top,
                                    int bottom)
  {
    if (ths->progress_callback) {
      return (*(ths->progress_callback))(ths->progress, left, right, top, bottom);
    }
    return true;
  }

};

#endif  // CCUTIL_OCRCLASS_H_
