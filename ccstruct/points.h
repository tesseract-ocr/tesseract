/**********************************************************************
 * File:        points.h  (Formerly coords.h)
 * Description: Coordinate class definitions.
 * Author:      Ray Smith
 * Created:     Fri Mar 15 08:32:45 GMT 1991
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

class FCOORD;

///integer coordinate
class ICOORD
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
    inT16 x() const {
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
    BOOL8 operator== (const ICOORD & other) const {
      return xcoord == other.xcoord && ycoord == other.ycoord;
    }
    ///test inequality
    BOOL8 operator!= (const ICOORD & other) const {
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

    // Writes to the given file. Returns false in case of error.
    bool Serialize(FILE* fp) const;
    // Reads from the given file. Returns false in case of error.
    // If swap is true, assumes a big/little-endian swap is needed.
    bool DeSerialize(bool swap, FILE* fp);

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

    static ICOORDELT* deep_copy(const ICOORDELT* src) {
      ICOORDELT* elt = new ICOORDELT;
      *elt = *src;
      return elt;
    }

};

ELISTIZEH (ICOORDELT)
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
    // Returns the standard feature direction corresponding to this.
    // See binary_angle_plus_pi below for a description of the direction.
    uinT8 to_direction() const;
    // Sets this with a unit vector in the given standard feature direction.
    void from_direction(uinT8 direction);

    // Converts an angle in radians (from ICOORD::angle or FCOORD::angle) to a
    // standard feature direction as an unsigned angle in 256ths of a circle
    // measured anticlockwise from (-1, 0).
    static uinT8 binary_angle_plus_pi(double angle);
    // Inverse of binary_angle_plus_pi returns an angle in radians for the
    // given standard feature direction.
    static double angle_from_direction(uinT8 direction);
    // Returns the point on the given line nearest to this, ie the point such
    // that the vector point->this is perpendicular to the line.
    // The line is defined as a line_point and a dir_vector for its direction.
    // dir_vector need not be a unit vector.
    FCOORD nearest_pt_on_line(const FCOORD& line_point,
                              const FCOORD& dir_vector) const;

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
    // unrotate - undo a rotate(vec)
    // @param vec by vector
    void unrotate(const FCOORD &vec);
    ///divide
    friend FCOORD & operator/= (FCOORD &, float);

  private:
    float xcoord;                //2 floating coords
    float ycoord;
};

#include          "ipoints.h"    /*do inline funcs */
#endif
