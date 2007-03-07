#ifndef                    TESSCLAS_H
#define                    TESSCLAS_H 1

#define SPLINESIZE      23       /*max spline parts to a line */

#define TBLOBFLAGS      4        /*No of flags in a blob */
#define MAX_WO_CLASSES    3
#define EDGEPTFLAGS     4        /*concavity,length etc. */

typedef struct
{
  double a;                      /*x squared */
  double b;                      /*x */
  double c;                      /*constant */
} QUAD_SPEC;                     /*definiton of quadratic */

typedef struct
{
  int segments;                  /*no of spline segments */
  int xstarts[SPLINESIZE];       /*start x coords */
  QUAD_SPEC quads[SPLINESIZE];   /*quadratic sections */
} SPLINE_SPEC;                   /*quadratic spline */

typedef struct
{
  short x;                       /*absolute x coord */
  short y;                       /*absolute y coord */
} TPOINT;
typedef TPOINT VECTOR;           /*structure for coordinates */

typedef struct
{
  char dx;                       /*compact vectors */
  char dy;
} BYTEVEC;

typedef struct edgeptstruct
{
  TPOINT pos;                    /*position */
  VECTOR vec;                    /*vector to next point */
  char flags[EDGEPTFLAGS];       /*concavity, length etc */
  struct edgeptstruct *next;     /*anticlockwise element */
  struct edgeptstruct *prev;     /*clockwise element */
} EDGEPT;                        /*point on expanded outline */

typedef struct blobstruct
{
  struct olinestruct *outlines;  /*list of outlines in blob */
  char flags[TBLOBFLAGS];        /*blob flags */
  char correct;                  /*correct text */
  char guess;                    /*best guess */
                                 /*quickie classification */
  unsigned char classes[MAX_WO_CLASSES];
                                 /*quickie ratings */
  unsigned char values[MAX_WO_CLASSES];
  struct blobstruct *next;       /*next blob in block */
} TBLOB;                         /*blob structure */

typedef struct olinestruct
{
  TPOINT topleft;                /*top left of loop */
  TPOINT botright;               /*bottom right of loop */
  TPOINT start;                  /*start of loop */
  BYTEVEC *compactloop;          /*ptr to compacted loop */
  EDGEPT *loop;                  /*edgeloop */
  void *node;                    /*1st node on outline */
  struct olinestruct *next;      /*next at this level */
  struct olinestruct *child;     /*inner outline */
} TESSLINE;                      /*outline structure */

typedef struct wordstruct
{
  struct textrowstruct *row;     /*row it came from */
  char *correct;                 /*correct word string */
  char *guess;                   /*guess word string */
  TBLOB *blobs;                  /*blobs in word */
  int blanks;                    /*blanks before word */
  int blobcount;                 /*no of blobs in word */
  struct wordstruct *next;       /*next word */
} TWERD;                         /*word structure */

typedef struct textrowstruct
{
  int blobcount;                 /** count of blobs in row. **/
  TBLOB *blobs;                  /*list of blobs in row */
  TWERD *words;                  /*list of words in row */
  int mean_y;                    /** y coordinate of centre of row **/
  int max_y;                     /** y coordinate of top of row **/
  int min_y;                     /** y coordinate of bottom of row **/
  SPLINE_SPEC xheight;           /*top of row */
  SPLINE_SPEC baseline;          /*bottom of row */
  float descdrop;                /*descender drop */
  float ascrise;                 /*ascender rise */
  float lineheight;              /*average xheight-baseline */
  int kerning;                   /*kerning of row */
  int space;                     /*spacing of row */
  float space_threshold;         /*Bayesian space limit */
  int p_spaced;                  /*proportinal flag */
  int b_space;                   /*block spacing */
  int b_kern;                    /*block kerning */
  struct textrowstruct *next;    /*next row in block */
} TEXTROW;

typedef struct blockstruct       /** list of coordinates **/
{
  TBLOB *blobs;                  /*blobs in block */
  TEXTROW *rows;                 /*rows in block */
  int blobcount;                 /*no of blobs */
  short xmin;
  short xmax;
  short ymin;
  short ymax;
  char type;                     /** block type **/
  char p_spaced;                 /** flag to show propertianal spacing **/
  short rowcount;                /** number of rows **/
  short leading;                 /** space between rows **/
  short kerning;                 /** space between characters **/
  short space;                   /** distance between char centres **/
  short minwidth;                /*min width of char in block */
  short p_size;                  /** point size of text **/
  short l_margin;                /** posn of left margin **/
  short italic;                  /** flag to show italic block **/
  short spurious;                /** percentage of spurious characters **/
  struct blockstruct *next;      /*next text block */
} TEXTBLOCK;                     /*block from image */

/**********************************************************************
 * iterate_blobs
 *
 * Visit all the words in a list using a local variable.
 **********************************************************************/

#define iterate_blobs(blob,blobs)  \
for (blob = blobs; blob != NULL; blob = blob->next)
#endif
