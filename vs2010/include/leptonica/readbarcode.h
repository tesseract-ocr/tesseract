/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -  This software is distributed in the hope that it will be
 -  useful, but with NO WARRANTY OF ANY KIND.
 -  No author or distributor accepts responsibility to anyone for the
 -  consequences of using this software, or for whether it serves any
 -  particular purpose or works at all, unless he or she says so in
 -  writing.  Everyone is granted permission to copy, modify and
 -  redistribute this source code, for commercial or non-commercial
 -  purposes, with the following restrictions: (1) the origin of this
 -  source code must not be misrepresented; (2) modified versions must
 -  be plainly marked as such; and (3) this notice may not be removed
 -  or altered from any source or modified source distribution.
 *====================================================================*/

#ifndef  LEPTONICA_READBARCODE_H
#define  LEPTONICA_READBARCODE_H

    /* ----------------------------------------------------------------- *
     *            Flags for method of extracting barcode widths          *
     * ----------------------------------------------------------------- */
enum {
    L_USE_WIDTHS = 1,           /* use histogram of barcode widths           */
    L_USE_WINDOWS = 2           /* find best window for decoding transitions */
};

    /* ----------------------------------------------------------------- *
     *                     Flags for barcode formats                     *
     * These are used both to identify a barcode format and to identify  *
     * the decoding method to use on a barcode.                          *
     * ----------------------------------------------------------------- */
enum {
    L_BF_UNKNOWN = 0,           /* unknown format                            */
    L_BF_ANY = 1,               /* try decoding with all known formats       */
    L_BF_CODE128 = 2,           /* decode with Code128 format                */
    L_BF_EAN8 = 3,              /* decode with EAN8 format                   */
    L_BF_EAN13 = 4,             /* decode with EAN13 format                  */
    L_BF_CODE2OF5 = 5,          /* decode with Code 2 of 5 format            */
    L_BF_CODEI2OF5 = 6,         /* decode with Interleaved 2 of 5 format     */
    L_BF_CODE39 = 7,            /* decode with Code39 format                 */
    L_BF_CODE93 = 8,            /* decode with Code93 format                 */
    L_BF_CODABAR = 9,           /* decode with Code93 format                 */
    L_BF_UPCA = 10              /* decode with UPC A format                  */
};

    /* ----------------------------------------------------------------- *
     *                  Currently supported formats                      *
     *            Update these arrays as new formats are added.          *
     * ----------------------------------------------------------------- */
static const l_int32  SupportedBarcodeFormat[] = {
    L_BF_CODE2OF5,
    L_BF_CODEI2OF5,
    L_BF_CODE93,
    L_BF_CODE39,
    L_BF_CODABAR,
    L_BF_UPCA,
    L_BF_EAN13
};
static const char  *SupportedBarcodeFormatName[] = {
    "Code2of5",
    "CodeI2of5",
    "Code93",
    "Code39",
    "Codabar",
    "Upca",
    "Ean13"
};
static const l_int32  NumSupportedBarcodeFormats = 7;


    /* ----------------------------------------------------------------- *
     *                       Code 2 of 5 symbology                       *
     * ----------------------------------------------------------------- */
static const char *Code2of5[] = {
    "111121211", "211111112", "112111112", "212111111",   /* 0 - 3 */
    "111121112", "211121111", "112121111", "111111212",   /* 4 - 7 */
    "211111211", "112111211",                             /* 8 - 9 */
    "21211", "21112"                                      /* Start, Stop */
};

static const l_int32  C25_START = 10;
static const l_int32  C25_STOP =  11;


    /* ----------------------------------------------------------------- *
     *                Code Interleaved 2 of 5 symbology                  *
     * ----------------------------------------------------------------- */
static const char *CodeI2of5[] = {
    "11221", "21112", "12112", "22111", "11212",    /*  0 - 4 */
    "21211", "12211", "11122", "21121", "12121",    /*  5 - 9 */
    "1111", "211"                                   /*  start, stop */
};

static const l_int32  CI25_START = 10;
static const l_int32  CI25_STOP =  11;


    /* ----------------------------------------------------------------- *
     *                         Code 93 symbology                         *
     * ----------------------------------------------------------------- */
static const char *Code93[] = {
    "131112", "111213", "111312", "111411", "121113", /* 0: 0 - 4 */
    "121212", "121311", "111114", "131211", "141111", /* 5: 5 - 9 */
    "211113", "211212", "211311", "221112", "221211", /* 10: A - E */
    "231111", "112113", "112212", "112311", "122112", /* 15: F - J */
    "132111", "111123", "111222", "111321", "121122", /* 20: K - O */
    "131121", "212112", "212211", "211122", "211221", /* 25: P - T */
    "221121", "222111", "112122", "112221", "122121", /* 30: U - Y */
    "123111", "121131", "311112", "311211", "321111", /* 35: Z,-,.,SP,$ */
    "112131", "113121", "211131", "131221", "312111", /* 40: /,+,%,($),(%) */
    "311121", "122211", "111141"                      /* 45: (/),(+), Start */
};

    /* Use "[]{}#" to represent special codes 43-47 */
static const char Code93Val[] =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%[]{}#";

static const l_int32  C93_START = 47;
static const l_int32  C93_STOP =  47;


    /* ----------------------------------------------------------------- *
     *                         Code 39 symbology                         *
     * ----------------------------------------------------------------- */
static const char *Code39[] = {
    "111221211", "211211112", "112211112", "212211111",  /* 0: 0 - 3      */
    "111221112", "211221111", "112221111", "111211212",  /* 4: 4 - 7      */
    "211211211", "112211211", "211112112", "112112112",  /* 8: 8 - B      */
    "212112111", "111122112", "211122111", "112122111",  /* 12: C - F     */
    "111112212", "211112211", "112112211", "111122211",  /* 16: G - J     */
    "211111122", "112111122", "212111121", "111121122",  /* 20: K - N     */
    "211121121", "112121121", "111111222", "211111221",  /* 24: O - R     */
    "112111221", "111121221", "221111112", "122111112",  /* 28: S - V     */
    "222111111", "121121112", "221121111", "122121111",  /* 32: W - Z     */
    "121111212", "221111211", "122111211", "121212111",  /* 36: -,.,SP,$  */
    "121211121", "121112121", "111212121", "121121211"   /* 40: /,+,%,*   */
};

    /* Use "*" to represent the Start and Stop codes (43) */
static const char Code39Val[] =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%*";

static const l_int32  C39_START = 43;
static const l_int32  C39_STOP =  43;


    /* ----------------------------------------------------------------- *
     *                         Codabar symbology                         *
     * ----------------------------------------------------------------- */
static const char *Codabar[] = {
    "1111122", "1111221", "1112112", "2211111", "1121121", /* 0: 0 - 4      */
    "2111121", "1211112", "1211211", "1221111", "2112111", /* 5: 5 - 9      */
    "1112211", "1122111", "2111212", "2121112", "2121211", /* 10: -,$,:,/,. */
    "1121212", "1122121", "1212112", "1112122", "1112221"  /* 15: +,A,B,C,D */
};

    /* Ascii representations for codes 16-19: (A or T), (B or N), (C or *),
     * (D or E).  These are used in pairs for the Start and Stop codes. */
static const char CodabarVal[] = "0123456789-$:/.+ABCD";


    /* ----------------------------------------------------------------- *
     *                          UPC-A symbology                          *
     * ----------------------------------------------------------------- */
static const char *Upca[] = {
    "3211", "2221", "2122", "1411", "1132",  /* 0: 0 - 4              */
    "1231", "1114", "1312", "1213", "3112",  /* 5: 5 - 9              */
    "111", "111", "11111"                    /* 10: Start, Stop, Mid  */
};

static const l_int32  UPCA_START = 10;
static const l_int32  UPCA_STOP =  11;
static const l_int32  UPCA_MID =   12;


    /* ----------------------------------------------------------------- *
     *                         Code128 symbology                         *
     * ----------------------------------------------------------------- */
static const char *Code128[] = {
    "212222", "222122", "222221", "121223", "121322",    /*  0 - 4 */
    "131222", "122213", "122312", "132212", "221213",    /*  5 - 9 */
    "221312", "231212", "112232", "122132", "122231",    /* 10 - 14 */
    "113222", "123122", "123221", "223211", "221132",    /* 15 - 19 */
    "221231", "213212", "223112", "312131", "311222",    /* 20 - 24 */
    "321122", "321221", "312212", "322112", "322211",    /* 25 - 29 */
    "212123", "212321", "232121", "111323", "131123",    /* 30 - 34 */
    "131321", "112313", "132113", "132311", "211313",    /* 35 - 39 */
    "231113", "231311", "112133", "112331", "132131",    /* 40 - 44 */
    "113123", "113321", "133121", "313121", "211331",    /* 45 - 49 */
    "231131", "213113", "213311", "213131", "311123",    /* 50 - 54 */
    "311321", "331121", "312113", "312311", "332111",    /* 55 - 59 */
    "314111", "221411", "431111", "111224", "111422",    /* 60 - 64 */
    "121124", "121421", "141122", "141221", "112214",    /* 65 - 69 */
    "112412", "122114", "122411", "142112", "142211",    /* 70 - 74 */
    "241211", "221114", "413111", "241112", "134111",    /* 75 - 79 */
    "111242", "121142", "121241", "114212", "124112",    /* 80 - 84 */
    "124211", "411212", "421112", "421211", "212141",    /* 85 - 89 */
    "214121", "412121", "111143", "111341", "131141",    /* 90 - 94 */
    "114113", "114311", "411113", "411311", "113141",    /* 95 - 99 */
    "114131", "311141", "411131", "211412", "211214",    /* 100 - 104 */
    "211232", "2331112"                                  /* 105 - 106 */
};

static const l_int32  C128_FUN_3 =    96;   /* in A or B only; in C it is 96 */
static const l_int32  C128_FUNC_2 =   97;   /* in A or B only; in C it is 97 */
static const l_int32  C128_SHIFT =    98;   /* in A or B only; in C it is 98 */
static const l_int32  C128_GOTO_C =   99;   /* in A or B only; in C it is 99 */
static const l_int32  C128_GOTO_B =  100;
static const l_int32  C128_GOTO_A =  101;
static const l_int32  C128_FUNC_1 =  102;
static const l_int32  C128_START_A = 103;
static const l_int32  C128_START_B = 104;
static const l_int32  C128_START_C = 105;
static const l_int32  C128_STOP =    106;
    /* code 128 symbols are 11 units */
static const l_int32  C128_SYMBOL_WIDTH = 11;



#endif  /* LEPTONICA_READBARCODE_H */
