/******************************************************************************
 *
 * File:         blkocc.h  (Formerly blockocc.h)
 * Description:  Block Occupancy routines
 * Author:       Chris Newton
 *
 * (c) Copyright 1991, Hewlett-Packard Company.
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
 ******************************************************************************/

#ifndef           BLKOCC_H
#define           BLKOCC_H

#include                   "params.h"
#include          "elst.h"

/***************************************************************************
CLASS REGION_OCC

  The class REGION_OCC defines a section of outline which exists entirely
  within a single region. The only data held is the min and max x limits of
  the outline within the region.

  REGION_OCCs are held on lists, one list for each region.  The lists are
  built in sorted order of min x. Overlapping REGION_OCCs are not permitted on
  a single list. An overlapping region to be added causes the existing region
  to be extended. This extension may result in the following REGION_OCC on the
  list overlapping the amended one. In this case the amended REGION_OCC is
  further extended to include the range of the following one, so that the
  following one can be deleted.

****************************************************************************/

class REGION_OCC:public ELIST_LINK
{
  public:
    float min_x;                 //Lowest x in region
    float max_x;                 //Highest x in region
    int16_t region_type;           //Type of crossing

    REGION_OCC() = default;  // constructor used
    // only in COPIER etc
    REGION_OCC(  //constructor
               float min,
               float max,
               int16_t region) {
      min_x = min;
      max_x = max;
      region_type = region;
    }
};

ELISTIZEH (REGION_OCC)
#define RANGE_IN_BAND(band_max, band_min, range_max, range_min) \
(((range_min) >= (band_min)) && ((range_max) < (band_max)))
/************************************************************************
Adapted from the following procedure so that it can be used in the bands
class in an include file...

bool    range_in_band[
              range within band?
int16_t band_max,
int16_t band_min,
int16_t range_max,
int16_t range_min]
{
  if ((range_min >= band_min) && (range_max < band_max))
    return true;
  else
    return false;
}
***********************************************************************/
#define RANGE_OVERLAPS_BAND(band_max, band_min, range_max, range_min) \
(((range_max) >= (band_min)) && ((range_min) < (band_max)))
/************************************************************************
Adapted from the following procedure so that it can be used in the bands
class in an include file...

bool    range_overlaps_band[
              range crosses band?
int16_t band_max,
int16_t band_min,
int16_t range_max,
int16_t range_min]
{
  if ((range_max >= band_min) && (range_min < band_max))
    return true;
  else
    return false;
}
***********************************************************************/
/**********************************************************************
  Bands
  -----

  BAND 4
--------------------------------
  BAND 3
--------------------------------

  BAND 2

--------------------------------

  BAND 1

Band 0 is the dot band

Each band has an error margin above and below. An outline is not considered to
have significantly changed bands until it has moved out of the error margin.
*************************************************************************/
class BAND
{
  public:
    int16_t max_max;               //upper max
    int16_t max;                   //nominal max
    int16_t min_max;               //lower max
    int16_t max_min;               //upper min
    int16_t min;                   //nominal min
    int16_t min_min;               //lower min

    BAND() = default; // constructor

    void set(                      // initialise a band
             int16_t new_max_max,    // upper max
             int16_t new_max,        // new nominal max
             int16_t new_min_max,    // new lower max
             int16_t new_max_min,    // new upper min
             int16_t new_min,        // new nominal min
             int16_t new_min_min) {  // new lower min
      max_max = new_max_max;
      max = new_max;
      min_max = new_min_max;
      max_min = new_max_min;
      min = new_min;
      min_min = new_min_min;
    }

    bool in_minimal(            //in minimal limits?
            float y) {  //y value
        return (y >= max_min) && (y < min_max);
    }

    bool in_nominal(            //in nominal limits?
            float y) {  //y value
        return (y >= min) && (y < max);
    }

    bool in_maximal(            //in maximal limits?
            float y) {  //y value
        return (y >= min_min) && (y < max_max);
    }

                                 //overlaps min limits?
    bool range_overlaps_minimal(float y1,    //one range limit
                                float y2) {  //other range limit
      if (y1 > y2)
        return RANGE_OVERLAPS_BAND (min_max, max_min, y1, y2);
      else
        return RANGE_OVERLAPS_BAND (min_max, max_min, y2, y1);
    }

                                 //overlaps nom limits?
    bool range_overlaps_nominal(float y1,    //one range limit
                                float y2) {  //other range limit
      if (y1 > y2)
        return RANGE_OVERLAPS_BAND (max, min, y1, y2);
      else
        return RANGE_OVERLAPS_BAND (max, min, y2, y1);
    }

                                 //overlaps max limits?
    bool range_overlaps_maximal(float y1,    //one range limit
                                float y2) {  //other range limit
      if (y1 > y2)
        return RANGE_OVERLAPS_BAND (max_max, min_min, y1, y2);
      else
        return RANGE_OVERLAPS_BAND (max_max, min_min, y2, y1);
    }

    bool range_in_minimal(             //within min limits?
            float y1,    //one range limit
            float y2) {  //other range limit
      if (y1 > y2)
        return RANGE_IN_BAND (min_max, max_min, y1, y2);
      else
        return RANGE_IN_BAND (min_max, max_min, y2, y1);
    }

    bool range_in_nominal(             //within nom limits?
            float y1,    //one range limit
            float y2) {  //other range limit
      if (y1 > y2)
        return RANGE_IN_BAND (max, min, y1, y2);
      else
        return RANGE_IN_BAND (max, min, y2, y1);
    }

    bool range_in_maximal(             //within max limits?
            float y1,    //one range limit
            float y2) {  //other range limit
      if (y1 > y2)
        return RANGE_IN_BAND (max_max, min_min, y1, y2);
      else
        return RANGE_IN_BAND (max_max, min_min, y2, y1);
    }
};

/* Standard positions */

#define MAX_NUM_BANDS 5
#define UNDEFINED_BAND 99
#define NO_LOWER_LIMIT -9999
#define NO_UPPER_LIMIT 9999

#define DOT_BAND 0

/* Special occupancy code emitted for the 0 region at the end of a word */

#define END_OF_WERD_CODE 255

extern BOOL_VAR_H (blockocc_show_result, false, "Show intermediate results");
extern INT_VAR_H (blockocc_desc_height, 0,
"Descender height after normalisation");
extern INT_VAR_H (blockocc_asc_height, 255,
"Ascender height after normalisation");
extern INT_VAR_H (blockocc_band_count, 4, "Number of bands used");
extern double_VAR_H (textord_underline_threshold, 0.9,
"Fraction of width occupied");

bool test_underline(                   //look for underlines
        bool testing_on,  //drawing blob
        C_BLOB* blob,      //blob to test
        int16_t baseline,    //coords of baseline
        int16_t xheight      //height of line
);

#endif
