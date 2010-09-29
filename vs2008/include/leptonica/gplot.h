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

#ifndef  LEPTONICA_GPLOT_H
#define  LEPTONICA_GPLOT_H

/*
 *   gplot.h
 *
 *       Data structures and parameters for generating gnuplot files
 */

#define  GPLOT_VERSION_NUMBER    1

#define  NUM_GPLOT_STYLES      5
enum GPLOT_STYLE {
    GPLOT_LINES       = 0,
    GPLOT_POINTS      = 1,
    GPLOT_IMPULSES    = 2,
    GPLOT_LINESPOINTS = 3,
    GPLOT_DOTS        = 4
};

#define  NUM_GPLOT_OUTPUTS     6
enum GPLOT_OUTPUT {
    GPLOT_NONE  = 0,
    GPLOT_PNG   = 1,
    GPLOT_PS    = 2,
    GPLOT_EPS   = 3,
    GPLOT_X11   = 4,
    GPLOT_LATEX = 5
};

enum GPLOT_SCALING {
    GPLOT_LINEAR_SCALE  = 0,   /* default */
    GPLOT_LOG_SCALE_X   = 1,
    GPLOT_LOG_SCALE_Y   = 2,
    GPLOT_LOG_SCALE_X_Y = 3
};

extern const char  *gplotstylenames[];  /* used in gnuplot cmd file */
extern const char  *gplotfilestyles[];  /* used in simple file input */
extern const char  *gplotfileoutputs[]; /* used in simple file input */

struct GPlot
{
    char          *rootname;   /* for cmd, data, output            */
    char          *cmdname;    /* command file name                */
    struct Sarray *cmddata;    /* command file contents            */
    struct Sarray *datanames;  /* data file names                  */
    struct Sarray *plotdata;   /* plot data (1 string/file)        */
    struct Sarray *plottitles; /* title for each individual plot   */
    struct Numa   *plotstyles; /* plot style for individual plots  */
    l_int32        nplots;     /* current number of plots          */
    char          *outname;    /* output file name                 */
    l_int32        outformat;  /* GPLOT_OUTPUT values              */
    l_int32        scaling;    /* GPLOT_SCALING values             */
    char          *title;      /* optional                         */
    char          *xlabel;     /* optional x axis label            */
    char          *ylabel;     /* optional y axis label            */
};
typedef struct GPlot  GPLOT;


#endif /* LEPTONICA_GPLOT_H */
