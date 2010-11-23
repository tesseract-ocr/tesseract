/**********************************************************************
 * File:        ocrclass.h
 * Description: Class definitions and constants for the OCR API.
 * Author:					Hewlett-Packard Co
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
 * structure alignment upto 8.
 **********************************************************************/

#ifndef           OCRCLASS_H
#define           OCRCLASS_H

#ifdef __MSW32__
#include          <windows.h>
#include          "gettimeofday.h"
#else
#include          <sys/time.h>
#endif
#include          <time.h>
#include          "host.h"

/*Maximum lengths of various strings*/
#define MAX_FONT_NAME   34       /*name of font */
#define MAX_OCR_NAME    32       /*name of engine */
#define MAX_OCR_VERSION   17     /*version code of engine */

/*Image parameters*/
#define MIN_IMAGE_SIZE    64     /*smallest image that will be passed */
#define IMAGE_ROUNDING    32     /*all sizes are multiple of this */

#if defined(__SLOW_TIMES__)
/*Maximum timeouts of various functions (in secs)*/
#define STARTUP_TIMEOUT   100    /*start of OCR engine */
#define SHUTDOWN_TIMEOUT  50     /*end of OCR engine */
#define SENDIM_TIMEOUT    50     /*send of image */
#define RELEASE_TIMEOUT   50     /*release of semaphore */
#define READIM_TIMEOUT    100    /*read of image */
#define READTEXT_TIMEOUT  50     /*read of text */
#define PROGRESS_TIMEOUT  30     /*progress every 3 seconds */
#define BADTIMES_TIMEOUT  7      /*max lack of progress */
#else
/*Maximum timeouts of various functions (in secs)*/
#define STARTUP_TIMEOUT   10     /*start of OCR engine */
#define SHUTDOWN_TIMEOUT  6      /*end of OCR engine */
#define SENDIM_TIMEOUT    5      /*send of image */
#define RELEASE_TIMEOUT   5      /*release of semaphore */
#define READIM_TIMEOUT    10     /*read of image */
#define READTEXT_TIMEOUT  5      /*read of text */
#define PROGRESS_TIMEOUT  3      /*progress every 3 seconds */
#define BADTIMES_TIMEOUT  7      /*max lack of progress */
#endif

/*language definitions are identical to RTF*/
#define LANGE_NONE      0x0400   /*no language */
#define LANGE_ALBANIAN    0x041c /*Albanian */
#define LANGE_BRITISH   0x0809   /*International English */
#define LANGE_BULGARIAN   0x0402 /*Bulgarian */
#define LANGE_CROATIAN    0x041a /*Croatian(latin alphabet) */
#define LANGE_CZECH     0x0405   /*Czech */
#define LANGE_DANISH    0x0406   /*Danish */
#define LANGE_DUTCH     0x0413   /*Dutch */
#define LANGE_FINNISH   0x040b   /*Finnish */
#define LANGE_FRENCH    0x040c   /*French */
#define LANGE_GERMAN    0x0407   /*German */
#define LANGE_GREEK     0x0408   /*Greek */
#define LANGE_HUNGARIAN   0x040e /*Hungarian */
#define LANGE_ITALIAN   0x0410   /*Italian */
#define LANGE_JAPANESE    0x0411 /*Japanese */
#define LANGE_KOREAN    0x0412   /*Korean */
#define LANGE_NORWEGIAN   0x0414 /*Bokmal */
#define LANGE_POLISH    0x0415   /*Polish */
#define LANGE_PORTUGESE   0x0416 /*Brazilian Portugese */
#define LANGE_ROMANIAN    0x0418 /*Romanian */
#define LANGE_RUSSIAN   0x0419   /*Russian */
#define LANGE_SCHINESE    0x0804 /*Simplified Chinese */
#define LANGE_SLOVAK    0x041b   /*Slovak */
#define LANGE_SPANISH   0x040a   /*Castilian */
#define LANGE_SWEDISH   0x041d   /*Swedish */
#define LANGE_TCHINESE    0x0404 /*Traditional Chinese */
#define LANGE_TURKISH   0x041f   /*Turkish */
#define LANGE_USENGLISH   0x0409 /*American */

/*font family definitions are identical to RTF*/
#define FFAM_NONE     0          /*unknown */
#define FFAM_ROMAN      1        /*serifed prop */
#define FFAM_SWISS      2        /*sans-serif prop */
#define FFAM_MODERN     3        /*fixed pitch */

/*character set definitions are identical to RTF*/
#define CHSET_ANSI      0        /*Ansi efigs */
#define CHSET_SHIFT_JIS   128    /*JIS X 0208-1990 */
#define CHSET_KOREAN    129      /*KS C 5601-1992 */
#define CHSET_SCHINESE    134    /*GB 2312-80 */
#define CHSET_BIG5      136      /*Big Five */
#define CHSET_CYRILLIC    204    /*Cyrillic */
#define CHSET_EEUROPE   238      /*Eastern Europe */

/*pitch set definitions are identical to RTF*/
#define PITCH_DEF     0          /*default */
#define PITCH_FIXED     1        /*fixed pitch */
#define PITCH_VAR     2          /*variable pitch */

/*Bitmasks for character enhancements.
OR these together for enhancement in ocr_append_char*/
#define EUC_BOLD      1          /*bold character */
#define EUC_ITALIC      2        /*italic char */
#define EUC_UNDERLINE   4        /*underlined char */
#define EUC_SUBSCRIPT   8        /*subscript char */
#define EUC_SUPERSCRIPT   16     /*superscript char */

/*enum for character rendering direction*/
enum OCR_CHAR_DIRECTION
{
  OCR_CDIR_RIGHT_LEFT,           /*right to left horizontal */
  OCR_CDIR_LEFT_RIGHT,           /*left to right horizontal */
  OCR_CDIR_TOP_BOTTOM,           /*top to bottom vertical */
  OCR_CDIR_BOTTOM_TOP            /*bottom to top vertical */
};

/*enum for line rendering direction*/
enum OCR_LINE_DIRECTION
{
  OCR_LDIR_DOWN_RIGHT,           /*horizontal lines go down */
  /*vertical lines go right */
  OCR_LDIR_UP_LEFT               /*horizontal lines go up */
};

/*enum for newline type*/
enum OCR_NEWLINE_TYPE
{
  OCR_NL_NONE,                   /*not a newline */
  OCR_NL_NEWLINE,                /*this is a newline but not new para */
  OCR_NL_NEWPARA                 /*this is a newline and a new para */
};

/*error codes that can be returned from the API functions other than OKAY and HPERR*/
#define OCR_API_NO_MEM    (-2)   /*filled output buffer */
#define OCR_API_BAD_CHAR  (-3)   /*whitespace sent to ocr_append_char */
#define OCR_API_BAD_STATE (-4)   /*invalid call sequence */

/*error codes used for passing errors back to the HP side*/
enum OCR_ERR_CODE
{
  OCR_ERR_NONE,                  /*no error */
  OCR_ERR_CLEAN_EXIT,            /*no error */
  OCR_ERR_NO_MEM,                /*out of memory */
  OCR_ERR_FILE_READ,             /*failed to read data file */
  OCR_ERR_TMP_WRITE,             /*failed to write temp file */
  OCR_ERR_TMP_READ,              /*failed to read temp file */
  OCR_ERR_BAD_DLL,               /*missing or invalid dll subcomponent */
  OCR_ERR_BAD_EXE,               /*missing or invalid exe subcomponent */
  OCR_ERR_BAD_LOAD,              /*failed to load subcomponent */
  OCR_ERR_BAD_LANG,              /*unable to recognize requested language */
  OCR_ERR_BAD_STATE,             /*engine did call out of sequence */
  OCR_ERR_INTERNAL1,             /*internal error type 1 */
  OCR_ERR_INTERNAL2,             /*internal error type 1 */
  OCR_ERR_INTERNAL3,             /*internal error type 1 */
  OCR_ERR_INTERNAL4,             /*internal error type 1 */
  OCR_ERR_INTERNAL5,             /*internal error type 1 */
  OCR_ERR_INTERNAL6,             /*internal error type 1 */
  OCR_ERR_INTERNAL7,             /*internal error type 1 */
  OCR_ERR_INTERNAL8,             /*internal error type 1 */
  OCR_ERR_TIMEOUT                /*timed out in comms */
};                               /*for calls to ocr_error */

/**********************************************************************
 * EFONT_DESC
 * Description of one font.
 * The information required is basically that used by RTF.
 * The name may be either a valid font on the system or the empty string.
 **********************************************************************/

typedef struct                   /*font description */
{
  uinT16 language;               /*default language */
  uinT8 font_family;             /*serif/not, fixed/not */
  uinT8 char_set;                /*character set standard */
  uinT8 pitch;                   /*fixed or prop */
  inT8 name[MAX_FONT_NAME + 1];  /*plain ascii name */
} EFONT_DESC;                    /*font description */

/**********************************************************************
 * EOCR_DESC
 * Description of the OCR engine provided at startup.
 * The name and version may be reported to the user at some point.
 * The fonts array should indicate the fonts that the OCR system
 * can recognize.
 **********************************************************************/

typedef struct                   /*startup info */
{
  inT32 protocol;                /*interface version */
  uinT32 font_count;             /*number of fonts */
  uinT16 language;               /*default language */
  uinT16 name[MAX_OCR_NAME + 1]; /*name of engine */
                                 /*version of engine */
  uinT16 version[MAX_OCR_VERSION + 1];
  EFONT_DESC fonts[1];           /*array of fonts */
} EOCR_DESC;                     /*startup info */

/**********************************************************************
 * ESTRIP_DESC
 * Description of the image strip as it is passed to the engine.
 * The image is always 1 bit, with 1=black.
 * The width is always a multiple of 32, so padding is always OK.
 * The height of the full image is always a multiple of 32.
 * The top y coordinate is 0, and increases down.
 * The top leftmost pixel is in the most significant bit of the first byte.
 **********************************************************************/

typedef struct                   /*bitmap strip */
{
  inT16 x_size;                  /*width in pixels */
  inT16 y_size;                  /*of full image */
  inT16 strip_size;              /*of this strip */
  inT16 resolution;              /*pixels per inch */
  uinT8 data[8];                 /*image data */
} ESTRIP_DESC;                   /*bitmap strip */

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

typedef struct                   /*single character */
{
// It should be noted that the format for char_code for version 2.0 and beyond is UTF8
// which means that ASCII characters will come out as one structure but other characters
// will be returned in two or more instances of this structure with a single byte of the
// UTF8 code in each, but each will have the same bounding box.
// Programs which want to handle languagues with different characters sets will need to
// handle extended characters appropriately, but *all* code needs to be prepared to
// receive UTF8 coded characters for characters such as bullet and fancy quotes.
  uinT16 char_code;              /*character itself */
  inT16 left;                    /*of char (-1) */
  inT16 right;                   /*of char (-1) */
  inT16 top;                     /*of char (-1) */
  inT16 bottom;                  /*of char (-1) */
  inT16 font_index;              /*what font (0) */
  uinT8 confidence;              /*0=perfect, 100=reject (0/100) */
  uinT8 point_size;              /*of char, 72=i inch, (10) */
  inT8 blanks;                   /*no of spaces before this char (1) */
  uinT8 formatting;              /*char formatting (0) */
} EANYCODE_CHAR;                 /*single character */

/**********************************************************************
 * ETEXT_DESC
 * Description of the output of the OCR engine.
 * This structure is used as both a progress monitor and the final
 * output header, since it needs to be a valid progress monitor while
 * the OCR engine is storing its output to shared memory.
 * During progress, all the buffer info is -1.
 * Progress starts at 0 and increases to 100 during OCR. No other constraint.
 * Every progress callback, the OCR engine must set ocr_alive to 1.
 * The HP side will set ocr_alive to 0. Repeated failure to reset
 * to 1 indicates that the OCR engine is dead.
 * If the cancel function is not null then it is called with the number of
 * user words found. If it returns true then operation is cancelled.
 **********************************************************************/
typedef bool (*CANCEL_FUNC)(void* cancel_this, int words);

class ETEXT_DESC                 /*output header */
{
 public:
  inT16 count;                   /*chars in this buffer(0) */
  inT16 progress;                /*percent complete increasing (0-100) */
  inT8 more_to_come;             /*true if not last */
  volatile inT8 ocr_alive;       /*ocr sets to 1, HP 0 */
  inT8 err_code;                 /*for errcode use */
  CANCEL_FUNC cancel;            /*returns true to cancel */
  void* cancel_this;             /*this or other data for cancel*/
  struct timeval end_time;       /*time to stop. expected to be set only by call
                                   to set_deadline_msecs()*/
  EANYCODE_CHAR text[1];         /*character data */

  ETEXT_DESC() : count(0), progress(0), more_to_come(0), ocr_alive(0),
                   err_code(0), cancel(NULL), cancel_this(NULL) {
    end_time.tv_sec = 0;
    end_time.tv_usec = 0;
  }

  // Sets the end time to be deadline_msecs milliseconds from now.
  void set_deadline_msecs(inT32 deadline_msecs) {
    gettimeofday(&end_time, NULL);
    inT32 deadline_secs = deadline_msecs / 1000;
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
    gettimeofday(&now, NULL);
    return (now.tv_sec > end_time.tv_sec || (now.tv_sec == end_time.tv_sec &&
                                             now.tv_usec > end_time.tv_usec));
  }
};

#endif
