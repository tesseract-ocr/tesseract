/**************************************************************************
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
**************************************************************************/

#ifndef CONST_H
#define CONST_H

/*This file contains constants which are global to the entire system*/

#define PATHSIZE      8192       /*max elements in edge path */
#define OUTLINESIZE     256      /*max elements in aprroximated path */
#define BLOBSIZE      10000      /*max no of blobs on a page */

#if 0
#define FIRSTCHAR       '!'      /*first char in set */
#define LASTCHAR        '~'      /*last char in set */
#define CHARSETSIZE     (LASTCHAR-FIRSTCHAR+1)
                                 no of chars in set
#define MERGESIZE     10         /*max chars in a class */
#define MAXCHARSIZE     159      /*max size of any char */
#define CLASSIZE      256        /*max no of classes */
#define SPLITSIZE     4          /*no of to_classes per class */
#define BADCLASS      255        /*null class */
#define BADMATCH      255        /*no match */
#define CLASSLENGTH     16       /*max chars in a class string */
#endif

#define VECSCALE      3          /*vector scaling factor in fx */
#define REALSCALE       ((double)(1<<VECSCALE))
                                 /*2.0**VECSCALE */
#define SPLINESIZE      23       /*max spline parts to a line */

#ifndef NULL
#define NULL        0            /*null pointer array index */
#endif

#define MAXUCHAR      255        /*max value of unsigned char */
#define PI          3.14159265359/*pi */
#ifndef __UNIX__
                                 /*approximate!! */
#define MAXFLOAT      2000000000.0f
#endif

#define FILENAMESIZE    1024     /*max permissible path name length */

#define MAX_WO_CLASSES    3      /*no of quickie classes */
//#define BLOBFLAGS                     4                                                       /*No of flags in a blob*/
#define ITALIC        0          /*measure of italicness */
#define ASPECT_RATIO    2        /*aspect ratio of blob */

#define NODEFLAGS     4          /*no of flags in a node */

#define EDGEPTFLAGS     4        /*concavity,length etc. */
#define FLAGS       0            /*flags array indices */
#define CONVEX        1          /*TESSLINE point is convex */
#define CONCAVE       2          /*used and set only in edges */
//#define FIXED                         4                                                       /*TESSLINE point is fixed*/
#define ONHULL        8          /*on convex hull */

#define RUNLENGTH     1          /*length of run */

#define DIR         2            /*direction of run */

#define CORRECTION      3        /*correction of run */

#define OUTLINES_PER_BLOB 8      /*max no of outlines in blob */

#define PLUS        1            /*starbase markers */
#define CIRCLE        3

//#define WHITE                         1                                                       /*starbase colours*/
//#define RED                                   2
//#define YELLOW                                3
//#define GREEN                         4
//#define CYAN                          5
//#define BLUE                          6

#define SMD         0x100000     /*memory driver output */

#define SCAN        0            /*scanner process id */
#define EDGE        1            /*edge process id */
#define FX          2            /*fx process id */
#define TESSTO      3            /*pageseg process id */
#define OCR         4            /*ocr process id */
#define MAXPROC       (OCR+1)    /*no of processes */

/*debugs[OCR] control flags*/
#define STRINGCMPS      0x1      /*show ocrdiff compares */

/*acts[OCR] control flags*/
#define CHECKS        0x1        /*run accuracy checks */
#define WRITEERRORS     0x2      /*write error output */
#define WRITECORRECTS   0x4      /*write correct blobs as errors */
#define WRITEWERDS      0x8      /*write whole words */
#define FXSELECT      0x10       /*write error output */
#define WRITEROWFILE    0x06     /*any write errors */
#define LEARN       0x100        /*learn mode */
#define WRITELEARNFILE    0xf00  /*any learning */
#define EACHWERD      0x2000     /*clear vdc after each word */
#endif
