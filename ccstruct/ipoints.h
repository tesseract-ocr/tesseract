/**********************************************************************
 * File:        ipoints.h  (Formerly icoords.h)
 * Description: Inline functions for coords.h.
 * Author:					Ray Smith
 * Created:					Fri Jun 21 15:14:21 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#ifndef           IPOINTS_H
#define           IPOINTS_H

#include          <math.h>

/**********************************************************************
 * operator!
 *
 * Rotate an ICOORD 90 degrees anticlockwise.
 **********************************************************************/

inline ICOORD
operator! (                      //rotate 90 deg anti
const ICOORD & src               //thing to rotate
) {
  ICOORD result;                 //output

  result.xcoord = -src.ycoord;
  result.ycoord = src.xcoord;
  return result;
}


/**********************************************************************
 * operator-
 *
 * Unary minus of an ICOORD.
 **********************************************************************/

inline ICOORD
operator- (                      //unary minus
const ICOORD & src               //thing to minus
) {
  ICOORD result;                 //output

  result.xcoord = -src.xcoord;
  result.ycoord = -src.ycoord;
  return result;
}


/**********************************************************************
 * operator+
 *
 * Add 2 ICOORDS.
 **********************************************************************/

inline ICOORD
operator+ (                      //sum vectors
const ICOORD & op1,              //operands
const ICOORD & op2) {
  ICOORD sum;                    //result

  sum.xcoord = op1.xcoord + op2.xcoord;
  sum.ycoord = op1.ycoord + op2.ycoord;
  return sum;
}


/**********************************************************************
 * operator+=
 *
 * Add 2 ICOORDS.
 **********************************************************************/

inline ICOORD &
operator+= (                     //sum vectors
ICOORD & op1,                    //operands
const ICOORD & op2) {
  op1.xcoord += op2.xcoord;
  op1.ycoord += op2.ycoord;
  return op1;
}


/**********************************************************************
 * operator-
 *
 * Subtract 2 ICOORDS.
 **********************************************************************/

inline ICOORD
operator- (                      //subtract vectors
const ICOORD & op1,              //operands
const ICOORD & op2) {
  ICOORD sum;                    //result

  sum.xcoord = op1.xcoord - op2.xcoord;
  sum.ycoord = op1.ycoord - op2.ycoord;
  return sum;
}


/**********************************************************************
 * operator-=
 *
 * Subtract 2 ICOORDS.
 **********************************************************************/

inline ICOORD &
operator-= (                     //sum vectors
ICOORD & op1,                    //operands
const ICOORD & op2) {
  op1.xcoord -= op2.xcoord;
  op1.ycoord -= op2.ycoord;
  return op1;
}


/**********************************************************************
 * operator%
 *
 * Scalar product of 2 ICOORDS.
 **********************************************************************/

inline inT32
operator% (                      //scalar product
const ICOORD & op1,              //operands
const ICOORD & op2) {
  return op1.xcoord * op2.xcoord + op1.ycoord * op2.ycoord;
}


/**********************************************************************
 * operator*
 *
 * Cross product of 2 ICOORDS.
 **********************************************************************/

inline inT32 operator *(                    //cross product
                        const ICOORD &op1,  //operands
                        const ICOORD &op2) {
  return op1.xcoord * op2.ycoord - op1.ycoord * op2.xcoord;
}


/**********************************************************************
 * operator*
 *
 * Scalar multiply of an ICOORD.
 **********************************************************************/

inline ICOORD operator *(                    //scalar multiply
                         const ICOORD &op1,  //operands
                         inT16 scale) {
  ICOORD result;                 //output

  result.xcoord = op1.xcoord * scale;
  result.ycoord = op1.ycoord * scale;
  return result;
}


inline ICOORD operator *(                   //scalar multiply
                         inT16 scale,
                         const ICOORD &op1  //operands
                        ) {
  ICOORD result;                 //output

  result.xcoord = op1.xcoord * scale;
  result.ycoord = op1.ycoord * scale;
  return result;
}


/**********************************************************************
 * operator*=
 *
 * Scalar multiply of an ICOORD.
 **********************************************************************/

inline ICOORD &
operator*= (                     //scalar multiply
ICOORD & op1,                    //operands
inT16 scale) {
  op1.xcoord *= scale;
  op1.ycoord *= scale;
  return op1;
}


/**********************************************************************
 * operator/
 *
 * Scalar divide of an ICOORD.
 **********************************************************************/

inline ICOORD
operator/ (                      //scalar divide
const ICOORD & op1,              //operands
inT16 scale) {
  ICOORD result;                 //output

  result.xcoord = op1.xcoord / scale;
  result.ycoord = op1.ycoord / scale;
  return result;
}


/**********************************************************************
 * operator/=
 *
 * Scalar divide of an ICOORD.
 **********************************************************************/

inline ICOORD &
operator/= (                     //scalar divide
ICOORD & op1,                    //operands
inT16 scale) {
  op1.xcoord /= scale;
  op1.ycoord /= scale;
  return op1;
}


/**********************************************************************
 * ICOORD::rotate
 *
 * Rotate an ICOORD by the given (normalized) (cos,sin) vector.
 **********************************************************************/

inline void ICOORD::rotate(  //rotate by vector
                           const FCOORD& vec) {
  inT16 tmp;

  tmp = (inT16) floor (xcoord * vec.x () - ycoord * vec.y () + 0.5);
  ycoord = (inT16) floor (ycoord * vec.x () + xcoord * vec.y () + 0.5);
  xcoord = tmp;
}


/**********************************************************************
 * operator!
 *
 * Rotate an FCOORD 90 degrees anticlockwise.
 **********************************************************************/

inline FCOORD
operator! (                      //rotate 90 deg anti
const FCOORD & src               //thing to rotate
) {
  FCOORD result;                 //output

  result.xcoord = -src.ycoord;
  result.ycoord = src.xcoord;
  return result;
}


/**********************************************************************
 * operator-
 *
 * Unary minus of an FCOORD.
 **********************************************************************/

inline FCOORD
operator- (                      //unary minus
const FCOORD & src               //thing to minus
) {
  FCOORD result;                 //output

  result.xcoord = -src.xcoord;
  result.ycoord = -src.ycoord;
  return result;
}


/**********************************************************************
 * operator+
 *
 * Add 2 FCOORDS.
 **********************************************************************/

inline FCOORD
operator+ (                      //sum vectors
const FCOORD & op1,              //operands
const FCOORD & op2) {
  FCOORD sum;                    //result

  sum.xcoord = op1.xcoord + op2.xcoord;
  sum.ycoord = op1.ycoord + op2.ycoord;
  return sum;
}


/**********************************************************************
 * operator+=
 *
 * Add 2 FCOORDS.
 **********************************************************************/

inline FCOORD &
operator+= (                     //sum vectors
FCOORD & op1,                    //operands
const FCOORD & op2) {
  op1.xcoord += op2.xcoord;
  op1.ycoord += op2.ycoord;
  return op1;
}


/**********************************************************************
 * operator-
 *
 * Subtract 2 FCOORDS.
 **********************************************************************/

inline FCOORD
operator- (                      //subtract vectors
const FCOORD & op1,              //operands
const FCOORD & op2) {
  FCOORD sum;                    //result

  sum.xcoord = op1.xcoord - op2.xcoord;
  sum.ycoord = op1.ycoord - op2.ycoord;
  return sum;
}


/**********************************************************************
 * operator-=
 *
 * Subtract 2 FCOORDS.
 **********************************************************************/

inline FCOORD &
operator-= (                     //sum vectors
FCOORD & op1,                    //operands
const FCOORD & op2) {
  op1.xcoord -= op2.xcoord;
  op1.ycoord -= op2.ycoord;
  return op1;
}


/**********************************************************************
 * operator%
 *
 * Scalar product of 2 FCOORDS.
 **********************************************************************/

inline float
operator% (                      //scalar product
const FCOORD & op1,              //operands
const FCOORD & op2) {
  return op1.xcoord * op2.xcoord + op1.ycoord * op2.ycoord;
}


/**********************************************************************
 * operator*
 *
 * Cross product of 2 FCOORDS.
 **********************************************************************/

inline float operator *(                    //cross product
                        const FCOORD &op1,  //operands
                        const FCOORD &op2) {
  return op1.xcoord * op2.ycoord - op1.ycoord * op2.xcoord;
}


/**********************************************************************
 * operator*
 *
 * Scalar multiply of an FCOORD.
 **********************************************************************/

inline FCOORD operator *(                    //scalar multiply
                         const FCOORD &op1,  //operands
                         float scale) {
  FCOORD result;                 //output

  result.xcoord = op1.xcoord * scale;
  result.ycoord = op1.ycoord * scale;
  return result;
}


inline FCOORD operator *(                   //scalar multiply
                         float scale,
                         const FCOORD &op1  //operands
                        ) {
  FCOORD result;                 //output

  result.xcoord = op1.xcoord * scale;
  result.ycoord = op1.ycoord * scale;
  return result;
}


/**********************************************************************
 * operator*=
 *
 * Scalar multiply of an FCOORD.
 **********************************************************************/

inline FCOORD &
operator*= (                     //scalar multiply
FCOORD & op1,                    //operands
float scale) {
  op1.xcoord *= scale;
  op1.ycoord *= scale;
  return op1;
}


/**********************************************************************
 * operator/
 *
 * Scalar divide of an FCOORD.
 **********************************************************************/

inline FCOORD
operator/ (                      //scalar divide
const FCOORD & op1,              //operands
float scale) {
  FCOORD result;                 //output

  if (scale != 0) {
    result.xcoord = op1.xcoord / scale;
    result.ycoord = op1.ycoord / scale;
  }
  return result;
}


/**********************************************************************
 * operator/=
 *
 * Scalar divide of an FCOORD.
 **********************************************************************/

inline FCOORD &
operator/= (                     //scalar divide
FCOORD & op1,                    //operands
float scale) {
  if (scale != 0) {
    op1.xcoord /= scale;
    op1.ycoord /= scale;
  }
  return op1;
}


/**********************************************************************
 * rotate
 *
 * Rotate an FCOORD by the given (normalized) (cos,sin) vector.
 **********************************************************************/

inline void FCOORD::rotate(  //rotate by vector
                           const FCOORD vec) {
  float tmp;

  tmp = xcoord * vec.x () - ycoord * vec.y ();
  ycoord = ycoord * vec.x () + xcoord * vec.y ();
  xcoord = tmp;
}

inline void FCOORD::unrotate(const FCOORD& vec) {
  rotate(FCOORD(vec.x(), -vec.y()));
}

#endif
