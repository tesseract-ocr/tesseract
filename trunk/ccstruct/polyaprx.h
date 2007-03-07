/**********************************************************************
 * File:        polyaprx.h  (Formerly polygon.h)
 * Description: Code for polygonal approximation from old edgeprog.
 * Author:		Ray Smith
 * Created:		Thu Nov 25 11:42:04 GMT 1993
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#ifndef           POLYAPRX_H
#define           POLYAPRX_H

#include          "tessclas.h"
#include          "poutline.h"
#include          "coutln.h"

OUTLINE *tesspoly_outline(                       //old approximation
                          C_OUTLINE *c_outline,  //input
                          float                  //xheight
                         );
EDGEPT *edgesteps_to_edgepts (   //convert outline
C_OUTLINE * c_outline,           //input
EDGEPT edgepts[]                 //output is array
);
void fix2(                //polygonal approx
          EDGEPT *start,  /*loop to approimate */
          int area);
EDGEPT *poly2(                  //second poly
              EDGEPT *startpt,  /*start of loop */
              int area          /*area of blob box */
             );
void cutline(                //recursive refine
             EDGEPT *first,  /*ends of line */
             EDGEPT *last,
             int area        /*area of object */
            );
#define fixed_dist      20       //really an int_variable
#define point_diff(p,p1,p2) (p).x = (p1).x - (p2).x ; (p).y = (p1).y - (p2).y
#define CROSS(a,b) ((a).x * (b).y - (a).y * (b).x)
#define LENGTH(a) ((a).x * (a).x + (a).y * (a).y)
#endif
