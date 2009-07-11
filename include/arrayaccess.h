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

#ifndef  LEPTONICA_ARRAY_ACCESS_H
#define  LEPTONICA_ARRAY_ACCESS_H

/*
 *  arrayaccess.h
 *
 *  1, 2, 4, 8, 16 and 32 bit data access within an array of 32-bit words
 *
 *  This is used primarily to access 1, 2, 4, 8, 16 and 32 bit pixels
 *  in a line of image data, represented as an array of 32-bit words.
 *
 *     pdata:  pointer to first 32-bit word in the array
 *     n:      index of the pixel in the array
 *
 *  Function calls for these accessors are defined in arrayaccess.c.
 *
 *  However, for efficiency we use the inline macros for all accesses.
 *  Even though the 2 and 4 bit set* accessors are more complicated,
 *  they are about 10% faster than the function calls.
 *
 *  The 32 bit access is just a cast and ptr arithmetic.  We include
 *  it so that the input ptr can be void*.
 *
 *  At the end of this file is code for invoking the function calls
 *  instead of inlining.
 *
 *  The macro SET_DATA_BIT_VAL(pdata, n, val) is a bit slower than
 *      if (val == 0)
 *          CLEAR_DATA_BIT(pdata, n);
 *      else
 *          SET_DATA_BIT(pdata, n);
 */


#if 1   /* Inline Accessors */
#ifndef COMPILER_MSVC

    /*--------------------------------------------------*
     *                     1 bit access                 *
     *--------------------------------------------------*/
#define  GET_DATA_BIT(pdata, n) \
    ((*((l_uint32 *)(pdata) + ((n) >> 5)) >> (31 - ((n) & 31))) & 1)

#define  SET_DATA_BIT(pdata, n) \
    (*((l_uint32 *)(pdata) + ((n) >> 5)) |= (0x80000000 >> ((n) & 31)))

#define  CLEAR_DATA_BIT(pdata, n) \
    (*((l_uint32 *)(pdata) + ((n) >> 5)) &= ~(0x80000000 >> ((n) & 31)))

#define  SET_DATA_BIT_VAL(pdata, n, val) \
    ({l_uint32 *_TEMP_WORD_PTR_; \
     _TEMP_WORD_PTR_ = (l_uint32 *)(pdata) + ((n) >> 5); \
     *_TEMP_WORD_PTR_ &= ~(0x80000000 >> ((n) & 31)); \
     *_TEMP_WORD_PTR_ |= ((val) << (31 - ((n) & 31))); \
    })


    /*--------------------------------------------------*
     *                     2 bit access                 *
     *--------------------------------------------------*/
#define  GET_DATA_DIBIT(pdata, n) \
    ((*((l_uint32 *)(pdata) + ((n) >> 4)) >> (2 * (15 - ((n) & 15)))) & 3)

#define  SET_DATA_DIBIT(pdata, n, val) \
    ({l_uint32 *_TEMP_WORD_PTR_; \
     _TEMP_WORD_PTR_ = (l_uint32 *)(pdata) + ((n) >> 4); \
     *_TEMP_WORD_PTR_ &= ~(0xc0000000 >> (2 * ((n) & 15))); \
     *_TEMP_WORD_PTR_ |= ((val) << (30 - 2 * ((n) & 15))); \
    })

#define  CLEAR_DATA_DIBIT(pdata, n) \
    (*((l_uint32 *)(pdata) + ((n) >> 4)) &= ~(0xc0000000 >> (2 * ((n) & 15))))


    /*--------------------------------------------------*
     *                     4 bit access                 *
     *--------------------------------------------------*/
#define  GET_DATA_QBIT(pdata, n) \
     ((*((l_uint32 *)(pdata) + ((n) >> 3)) >> (4 * (7 - ((n) & 7)))) & 0xf)

#define  SET_DATA_QBIT(pdata, n, val) \
    ({l_uint32 *_TEMP_WORD_PTR_; \
     _TEMP_WORD_PTR_ = (l_uint32 *)(pdata) + ((n) >> 3); \
     *_TEMP_WORD_PTR_ &= ~(0xf0000000 >> (4 * ((n) & 7))); \
     *_TEMP_WORD_PTR_ |= ((val) << (28 - 4 * ((n) & 7))); \
    })

#define  CLEAR_DATA_QBIT(pdata, n) \
    (*((l_uint32 *)(pdata) + ((n) >> 3)) &= ~(0xf0000000 >> (4 * ((n) & 7))))


    /*--------------------------------------------------*
     *                     8 bit access                 *
     *--------------------------------------------------*/
#ifdef  L_BIG_ENDIAN
#define  GET_DATA_BYTE(pdata, n) \
             (*((l_uint8 *)(pdata) + (n)))
#else  /* L_LITTLE_ENDIAN */
#define  GET_DATA_BYTE(pdata, n) \
             (*(l_uint8 *)((l_uintptr_t)((l_uint8 *)(pdata) + (n)) ^ 3))
#endif  /* L_BIG_ENDIAN */

#ifdef  L_BIG_ENDIAN
#define  SET_DATA_BYTE(pdata, n, val) \
             (*((l_uint8 *)(pdata) + (n)) = (val))
#else  /* L_LITTLE_ENDIAN */
#define  SET_DATA_BYTE(pdata, n, val) \
             (*(l_uint8 *)((l_uintptr_t)((l_uint8 *)(pdata) + (n)) ^ 3) = (val))
#endif  /* L_BIG_ENDIAN */


    /*--------------------------------------------------*
     *                    16 bit access                 *
     *--------------------------------------------------*/
#ifdef  L_BIG_ENDIAN
#define  GET_DATA_TWO_BYTES(pdata, n) \
             (*((l_uint16 *)(pdata) + (n)))
#else  /* L_LITTLE_ENDIAN */
#define  GET_DATA_TWO_BYTES(pdata, n) \
             (*(l_uint16 *)((l_uintptr_t)((l_uint16 *)(pdata) + (n)) ^ 2))
#endif  /* L_BIG_ENDIAN */

#ifdef  L_BIG_ENDIAN
#define  SET_DATA_TWO_BYTES(pdata, n, val) \
             (*((l_uint16 *)(pdata) + (n)) = (val))
#else  /* L_LITTLE_ENDIAN */
#define  SET_DATA_TWO_BYTES(pdata, n, val) \
             (*(l_uint16 *)((l_uintptr_t)((l_uint16 *)(pdata) + (n)) ^ 2) = (val))
#endif  /* L_BIG_ENDIAN */


    /*--------------------------------------------------*
     *                    32 bit access                 *
     *--------------------------------------------------*/
#define  GET_DATA_FOUR_BYTES(pdata, n) \
             (*((l_uint32 *)(pdata) + (n)))

#define  SET_DATA_FOUR_BYTES(pdata, n, val) \
             (*((l_uint32 *)(pdata) + (n)) = (val))


#endif  /* COMPILER_MSVC */
#endif  /* Inline Accessors */



    /*--------------------------------------------------*
     *  Slower, using function calls for all accessors  *
     *--------------------------------------------------*/
#if 0 || COMPILER_MSVC
#define  GET_DATA_BIT(pdata, n)               l_getDataBit(pdata, n)
#define  SET_DATA_BIT(pdata, n)               l_setDataBit(pdata, n)
#define  CLEAR_DATA_BIT(pdata, n)             l_clearDataBit(pdata, n)
#define  SET_DATA_BIT_VAL(pdata, n, val)      l_setDataBitVal(pdata, n, val)

#define  GET_DATA_DIBIT(pdata, n)             l_getDataDibit(pdata, n)
#define  SET_DATA_DIBIT(pdata, n, val)        l_setDataDibit(pdata, n, val)
#define  CLEAR_DATA_DIBIT(pdata, n)           l_clearDataDibit(pdata, n)

#define  GET_DATA_QBIT(pdata, n)              l_getDataQbit(pdata, n)
#define  SET_DATA_QBIT(pdata, n, val)         l_setDataQbit(pdata, n, val)
#define  CLEAR_DATA_QBIT(pdata, n)            l_clearDataQbit(pdata, n)

#define  GET_DATA_BYTE(pdata, n)              l_getDataByte(pdata, n)
#define  SET_DATA_BYTE(pdata, n, val)         l_setDataByte(pdata, n, val)

#define  GET_DATA_TWO_BYTES(pdata, n)         l_getDataTwoBytes(pdata, n)
#define  SET_DATA_TWO_BYTES(pdata, n, val)    l_setDataTwoBytes(pdata, n, val)

#define  GET_DATA_FOUR_BYTES(pdata, n)         l_getDataFourBytes(pdata, n)
#define  SET_DATA_FOUR_BYTES(pdata, n, val)    l_setDataFourBytes(pdata, n, val)
#endif


#endif /* LEPTONICA_ARRAY_ACCESS_H */
