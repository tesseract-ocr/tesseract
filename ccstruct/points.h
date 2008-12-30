/**********************************************************************
 * File:        points.h  (Formerly coords.h)
 * Description: Coordinate class definitions.
 * Author:					Ray Smith
 * Created:					Fri Mar 15 08:32:45 GMT 1991
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

#ifndef           POINTS_H
#define           POINTS_H

#include          <stdio.h>
#include          <math.h>
#include          "elst.h"
//#include                                      "ipeerr.h"

class FCOORD;

class DLLSYM ICOORD              //integer coordinate
{
  friend class FCOORD;

  public:
    ICOORD() {  //empty constructor
      xcoord = ycoord = 0;       //default zero
    }
    ICOORD(              //constructor
           inT16 xin,    //x value
           inT16 yin) {  //y value
      xcoord = xin;
      ycoord = yin;
    }
    ~ICOORD () {                 //destructor
    }

                                 //access function
    NEWDELETE2 (ICOORD) inT16 x () const
    {
      return xcoord;
    }
    inT16 y() const {  //access_function
      return ycoord;
    }

    void set_x(  //rewrite function
               inT16 xin) {
      xcoord = xin;              //write new value
    }
    void set_y(              //rewrite function
               inT16 yin) {  //value to set
      ycoord = yin;
    }

    float sqlength() const {  //find sq length
      return (float) (xcoord * xcoord + ycoord * ycoord);
    }

    float length() const {  //find length
      return (float) sqrt (sqlength ());
    }

    float pt_to_pt_sqdist(  //sq dist between pts
                          const ICOORD &pt) const {
      ICOORD gap;

      gap.xcoord = xcoord - pt.xcoord;
      gap.ycoord = ycoord - pt.ycoord;
      return gap.sqlength ();
    }

    float pt_to_pt_dist(  //Distance between pts
                        const ICOORD &pt) const {
      return (float) sqrt (pt_to_pt_sqdist (pt));
    }

    float angle() const {  //find angle
      return (float) atan2 ((double) ycoord, (double) xcoord);
    }

    BOOL8 operator== (           //test equality
    const ICOORD & other) {
      return xcoord == other.xcoord && ycoord == other.ycoord;
    }
    BOOL8 operator!= (           //test inequality
    const ICOORD & other) {
      return xcoord != other.xcoord || ycoord != other.ycoord;
    }
    friend ICOORD operator! (    //rotate 90 deg anti
      const ICOORD &);
    friend ICOORD operator- (    //unary minus
      const ICOORD &);
    friend ICOORD operator+ (    //add
      const ICOORD &, const ICOORD &);
    friend ICOORD & operator+= ( //add
      ICOORD &, const ICOORD &);
    friend ICOORD operator- (    //subtract
      const ICOORD &, const ICOORD &);
    friend ICOORD & operator-= ( //subtract
      ICOORD &, const ICOORD &);
    friend inT32 operator% (     //scalar product
      const ICOORD &, const ICOORD &);
    friend inT32 operator *(  //cross product
                            const ICOORD &,
                            const ICOORD &);
    friend ICOORD operator *(  //multiply
                             const ICOORD &,
                             inT16);
    friend ICOORD operator *(  //multiply
                             inT16,
                             const ICOORD &);
    friend ICOORD & operator*= ( //multiply
      ICOORD &, inT16);
    friend ICOORD operator/ (    //divide
      const ICOORD &, inT16);
                                 //divide
    friend ICOORD & operator/= (ICOORD &, inT16);
    void rotate(                    //rotate
                const FCOORD& vec);  //by vector

    // Setup for iterating over the pixels in a vector by the well-known
    // Bresenham rendering algorithm.
    // Starting with major/2 in the accumulator, on each step move by
    // major_step, and then add minor to the accumulator. When
    // accumulator >= major subtract major and also move by minor_step.
    void setup_render(ICOORD* major_step, ICOORD* minor_step,
                      int* major, int* minor);

    void serialise_asc(  //serialise to ascii
                       FILE *f);
    void de_serialise_asc(  //serialise from ascii
                          FILE *f);

  protected:
    inT16 xcoord;                //x value
    inT16 ycoord;                //y value
};

class DLLSYM ICOORDELT:public ELIST_LINK, public ICOORD
                                 //embedded coord list
{
  public:
    ICOORDELT() {  //empty constructor
    }
    ICOORDELT (                  //constructor
                                 //from ICOORD
    ICOORD icoord):ICOORD (icoord) {
    }
    ICOORDELT(              //constructor
              inT16 xin,    //x value
              inT16 yin) {  //y value
      xcoord = xin;
      ycoord = yin;
    }

    /* Note that prep_serialise() dump() and de_dump() dont need to do anything
    more than terminate recursion. */

    void prep_serialise() const {  //set ptrs to counts
    }

    void dump(  //write external bits
              FILE *) const {
    }

    void de_dump(  //read external bits
                 FILE *) {
    }

                                 //serialise to ascii
    make_serialise(ICOORDELT)

    static ICOORDELT* deep_copy(const ICOORDELT* src) {
      ICOORDELT* elt = new ICOORDELT;
      *elt = *src;
      return elt;
    }

    void serialise_asc(FILE * f);
    void de_serialise_asc(  //serialise from ascii
                          FILE *f);

};

ELISTIZEH_S (ICOORDELT)
class DLLSYM FCOORD
{
  public:
    FCOORD() {
    }                            //empty constructor
    FCOORD(               //constructor
           float xvalue,  //coords to set
           float yvalue) {
      xcoord = xvalue;           //set coords
      ycoord = yvalue;
    }
    FCOORD(                  //make from ICOORD
           ICOORD icoord) {  //coords to set
      xcoord = icoord.xcoord;
      ycoord = icoord.ycoord;
    }

    float x() const {  //get coords
      return xcoord;
    }
    float y() const {
      return ycoord;
    }
    void set_x(  //rewrite function
               float xin) {
      xcoord = xin;              //write new value
    }
    void set_y(              //rewrite function
               float yin) {  //value to set
      ycoord = yin;
    }

    float sqlength() const {  //find sq length
      return xcoord * xcoord + ycoord * ycoord;
    }

    float length() const {  //find length
      return (float) sqrt (sqlength ());
    }

    float pt_to_pt_sqdist(  //sq dist between pts
                          const FCOORD &pt) const {
      FCOORD gap;

      gap.xcoord = xcoord - pt.xcoord;
      gap.ycoord = ycoord - pt.ycoord;
      return gap.sqlength ();
    }

    float pt_to_pt_dist(  //Distance between pts
                        const FCOORD &pt) const {
      return (float) sqrt (pt_to_pt_sqdist (pt));
    }

    float angle() const {  //find angle
      return (float) atan2 (ycoord, xcoord);
    }

    bool normalise();  //Convert to unit vec

    BOOL8 operator== (           //test equality
    const FCOORD & other) {
      return xcoord == other.xcoord && ycoord == other.ycoord;
    }
    BOOL8 operator!= (           //test inequality
    const FCOORD & other) {
      return xcoord != other.xcoord || ycoord != other.ycoord;
    }
                                 //rotate 90 deg anti
    friend FCOORD operator! (const FCOORD &);
                                 //unary minus
    friend FCOORD operator- (const FCOORD &);
                                 //add
    friend FCOORD operator+ (const FCOORD &, const FCOORD &);
                                 //add
    friend FCOORD & operator+= (FCOORD &, const FCOORD &);
                                 //subtract
    friend FCOORD operator- (const FCOORD &, const FCOORD &);
                                 //subtract
    friend FCOORD & operator-= (FCOORD &, const FCOORD &);
                                 //scalar product
    friend float operator% (const FCOORD &, const FCOORD &);
                                 //cross product
    friend float operator *(const FCOORD &, const FCOORD &);
    friend FCOORD operator *(const FCOORD &, float);
    //multiply
    friend FCOORD operator *(float, const FCOORD &);
    //multiply
                                 //multiply
    friend FCOORD & operator*= (FCOORD &, float);
    friend FCOORD operator/ (const FCOORD &, float);
    //divide
    void rotate(                    //rotate
                const FCOORD vec);  //by vector
                                 //divide
    friend FCOORD & operator/= (FCOORD &, float);

  private:
    float xcoord;                //2 floating coords
    float ycoord;
};

#include          "ipoints.h"    /*do inline funcs */
#endif
