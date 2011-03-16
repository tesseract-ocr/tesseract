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

#ifndef  LEPTONICA_REGUTILS_H
#define  LEPTONICA_REGUTILS_H

/*
 *   regutils.h
 *
 *   Contains this regression test parameter packaging struct
 *       struct L_RegParams
 *
 *
 *   The regression test utility allows you to write regression tests
 *   that compare results with existing "golden files".
 *
 *   Such regression tests can be called in three ways.
 *   For example, for distance_reg:
 *
 *       Case 1: distance_reg generate
 *           This generates golden files in /tmp for the reg test.
 *
 *       Case 2: distance_reg compare
 *           This runs the test against the set of golden files.  It
 *           appends to 'outfile.txt' either "SUCCESS" or "FAILURE",
 *           as well as the details of any parts of the test that failed.
 *           It writes to a temporary file stream (fp)
 *
 *       Case 3: distance_reg [display]
 *           This runs the test but makes no comparison of the output
 *           against the set of golden files.  In addition, this displays
 *           images and plots that are specified in the test under
 *           control of the display variable.  Display is enabled only
 *           for this case.  Using 'display' on the command line is optional.
 *
 *   Regression tests follow the pattern given below.  In an actual
 *   case, comparisons of pix and of files can occur in any order.
 *   We give a specific order here for clarity.
 *
 *       L_REGPARAMS  *rp;  // holds data required by the test functions
 *
 *       // Setup variables; optionally open stream
 *       if (regTestSetup(argc, argv, &rp))
 *           return 1;
 *
 *       // Test pairs of generated pix for identity.  This compares
 *       // two pix; no golden file is generated.
 *       regTestComparePix(rp, pix1, pix2);
 *
 *       // Test pairs of generated pix for similarity.  This compares
 *       // two pix; no golden file is generated.  The last arg determines
 *       // if stats are to be written to stderr.
 *       regTestCompareSimilarPix(rp, pix1, pix2, 15, 0.001, 0);
 *
 *       // Generation of <newfile*> outputs and testing for identity
 *       // These files can be anything, of course.
 *       regTestCheckFile(rp, <newfile0>);
 *       regTestCheckFile(rp, <newfile1>);
 *
 *       // Test pairs of output golden files for identity.  Here we
 *       // are comparing golden files 4 and 5.
 *       regTestCompareFiles(rp, 4, 5);
 *
 *       // "Write and check".  This writes a pix using a canonical
 *       // formulation for the local filename and either:
 *       //     case 1: generates a golden file
 *       //     case 2: compares the local file with a golden file
 *       //     case 3: generates local files and displays
 *       // Here we write the pix compressed with png and jpeg, respectively;
 *       // Then check against the golden file.  The internal @index
 *       // is incremented; it is embedded in the local filename and,
 *       // if generating, in the golden file as well.
 *       regTestWritePixAndCheck(rp, pix1, IFF_PNG);
 *       regTestWritePixAndCheck(rp, pix2, IFF_JFIF_JPEG);
 *
 *       // Display if reg test was called in 'display' mode
 *       pixDisplayWithTitle(pix1, 100, 100, NULL, rp->display);
 *
 *       // Clean up and output result
 *       regTestCleanup(rp);
 */


/*-------------------------------------------------------------------------*
 *                     Regression test parameter packer                    *
 *-------------------------------------------------------------------------*/
struct L_RegParams
{
    FILE    *fp;        /* stream to temporary output file for compare mode */
    char    *testname;  /* name of test, without '_reg'                     */
    char    *tempfile;  /* name of temp file for compare mode output        */
    l_int32  mode;      /* generate, compare or display                     */
    l_int32  index;     /* index into saved files for this test; 0-based    */
    l_int32  success;   /* overall result of the test                       */
    l_int32  display;   /* 1 if in display mode; 0 otherwise                */
    L_TIMER  tstart;    /* marks beginning of the reg test                  */
};
typedef struct L_RegParams  L_REGPARAMS;


    /* Running modes for the test */
enum {
    L_REG_GENERATE = 0,
    L_REG_COMPARE = 1,
    L_REG_DISPLAY = 2
};


#endif  /* LEPTONICA_REGUTILS_H */

