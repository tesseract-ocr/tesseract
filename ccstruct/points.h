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

///integer coordinate
class DLLSYM ICOORD
{
  friend class FCOORD;

  public:
    ///empty constructor
    ICOORD() {
      xcoord = ycoord = 0;       //default zero
    }
	///constructor
	///@param xin x value
	///@param yin y value
    ICOORD(inT16 xin,
           inT16 yin) {
      xcoord = xin;
      ycoord = yin;
    }
	///destructor
    ~ICOORD () {
    }

    ///access function
    NEWDELETE2 (ICOORD) inT16 x () const
    {
      return xcoord;
    }
	///access_function
    inT16 y() const {
      return ycoord;
    }

	///rewrite function
    void set_x(inT16 xin) {
      xcoord = xin;              //write new value
    }
	///rewrite function
    void set_y(inT16 yin) {  //value to set
      ycoord = yin;
    }

    /// Set from the given x,y, shrinking the vector to fit if needed.
    void set_with_shrink(int x, int y);

	///find sq length
    float sqlength() const {
      return (float) (xcoord * xcoord + ycoord * ycoord);
    }

	///find length
    float length() const {
      return (float) sqrt (sqlength ());
    }

	///sq dist between pts
    float pt_to_pt_sqdist(const ICOORD &pt) const {
      ICOORD gap;

      gap.xcoord = xcoord - pt.xcoord;
      gap.ycoord = ycoord - pt.ycoord;
      return gap.sqlength ();
    }

	///Distance between pts
    float pt_to_pt_dist(const ICOORD &pt) const {
      return (float) sqrt (pt_to_pt_sqdist (pt));
    }

	///find angle
    float angle() const {
      return (float) atan2 ((double) ycoord, (double) xcoord);
    }

	///test equality
    BOOL8 operator== (const ICOORD & other) {
      return xcoord == other.xcoord && ycoord == other.ycoord;
    }
	///test inequality
    BOOL8 operator!= (const ICOORD & other) {
      return xcoord != other.xcoord || ycoord != other.ycoord;
    }
	///rotate 90 deg anti
    friend ICOORD operator! (const ICOORD &);
    ///unary minus
	friend ICOORD operator- (const ICOORD &);
	///add
	friend ICOORD operator+ (const ICOORD &, const ICOORD &);
	///add
    friend ICOORD & operator+= (ICOORD &, const ICOORD &);
    ///subtract
	friend ICOORD operator- (const ICOORD &, const ICOORD &);
    ///subtract
    friend ICOORD & operator-= (ICOORD &, const ICOORD &);
	///scalar product
	friend inT32 operator% (const ICOORD &, const ICOORD &);
	///cross product
    friend inT32 operator *(const ICOORD &,
                            const ICOORD &);
	///multiply
    friend ICOORD operator *(const ICOORD &,
                             inT16);
	///multiply
    friend ICOORD operator *(inT16,
                             const ICOORD &);
	///multiply
    friend ICOORD & operator*= (ICOORD &, inT16);
    ///divide
    friend ICOORD operator/ (const ICOORD &, inT16);
    ///divide
    friend ICOORD & operator/= (ICOORD &, inT16);
	///rotate
	///@param vec by vector
    void rotate(const FCOORD& vec);

    /// Setup for iterating over the pixels in a vector by the well-known
    /// Bresenham rendering algorithm.
    /// Starting with major/2 in the accumulator, on each step move by
    /// major_step, and then add minor to the accumulator. When
    /// accumulator >= major subtract major and also move by minor_step.
    void setup_render(ICOORD* major_step, ICOORD* minor_step,
                      int* major, int* minor) const;

	///serialise to ascii
    void serialise_asc(FILE *f);
	///serialise from ascii
    void de_serialise_asc(FILE *f);

  protected:
    inT16 xcoord;                //< x value
    inT16 ycoord;                //< y value
};

class DLLSYM ICOORDELT:public ELIST_LINK, public ICOORD
                                 //embedded coord list
{
  public:
    ///empty constructor
    ICOORDELT() {  
    }
	///constructor from ICOORD
    ICOORDELT (ICOORD icoord):ICOORD (icoord) {
    }
	///constructor
	///@param xin x value
	///@param yin y value
    ICOORDELT(inT16 xin,
              inT16 yin) {
      xcoord = xin;
      ycoord = yin;
    }

    /* Note that prep_serialise() dump() and de_dump() dont need to do anything
    more than terminate recursion. */

	///set ptrs to counts
    void prep_serialise() const {
    }

	///write external bits
    void dump(FILE *) const {
    }

	///read external bits
    void de_dump(FILE *) {
    }

    ///serialise to ascii
    make_serialise(ICOORDELT)

    static ICOORDELT* deep_copy(const ICOORDELT* src) {
      ICOORDELT* elt = new ICOORDELT;
      *elt = *src;
      return elt;
    }

	///serialise to ascii
    void serialise_asc(FILE * f);
	///deserialise from ascii
    void de_serialise_asc(FILE *f);

};

ELISTIZEH_S (ICOORDELT)
class DLLSYM FCOORD
{
  public:
    ///empty constructor
    FCOORD() {
    }
	///constructor
	///@param xvalue x value
	///@param yvalue y value
    FCOORD(float xvalue,
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
	///rewrite function
    void set_x(float xin) {
      xcoord = xin;              //write new value
    }
	///rewrite function
    void set_y(float yin) {  //value to set
      ycoord = yin;
    }

	///find sq length
    float sqlength() const {
      return xcoord * xcoord + ycoord * ycoord;
    }

	///find length
    float length() const {
      return (float) sqrt (sqlength ());
    }

	///sq dist between pts
    float pt_to_pt_sqdist(const FCOORD &pt) const {
      FCOORD gap;

      gap.xcoord = xcoord - pt.xcoord;
      gap.ycoord = ycoord - pt.ycoord;
      return gap.sqlength ();
    }

	///Distance between pts
    float pt_to_pt_dist(const FCOORD &pt) const {
      return (float) sqrt (pt_to_pt_sqdist (pt));
    }

	///find angle
    float angle() const {
      return (float) atan2 (ycoord, xcoord);
    }

	///Convert to unit vec
    bool normalise();

	///test equality
    BOOL8 operator== (const FCOORD & other) {
      return xcoord == other.xcoord && ycoord == other.ycoord;
    }
	///test inequality
    BOOL8 operator!= (const FCOORD & other) {
      return xcoord != other.xcoord || ycoord != other.ycoord;
    }
    ///rotate 90 deg anti
    friend FCOORD operator! (const FCOORD &);
    ///unary minus
    friend FCOORD operator- (const FCOORD &);
    ///add
    friend FCOORD operator+ (const FCOORD &, const FCOORD &);
    ///add
    friend FCOORD & operator+= (FCOORD &, const FCOORD &);
    ///subtract
    friend FCOORD operator- (const FCOORD &, const FCOORD &);
    ///subtract
    friend FCOORD & operator-= (FCOORD &, const FCOORD &);
    ///scalar product
    friend float operator% (const FCOORD &, const FCOORD &);
    ///cross product
    friend float operator *(const FCOORD &, const FCOORD &);
    ///multiply
	friend FCOORD operator *(const FCOORD &, float);
    ///multiply
    friend FCOORD operator *(float, const FCOORD &);

    ///multiply
    friend FCOORD & operator*= (FCOORD &, float);
    ///divide
    friend FCOORD operator/ (const FCOORD &, float);
	///rotate
	///@param vec by vector
    void rotate(const FCOORD vec);
    ///divide
    friend FCOORD & operator/= (FCOORD &, float);

  private:
    float xcoord;                //2 floating coords
    float ycoord;
};

#include          "ipoints.h"    /*do inline funcs */
#endif
