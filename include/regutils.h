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
 *       Case 2: distance_reg outfile.txt
 *           This runs the test against the set of golden files.  It
 *           appends to 'outfile.txt' either "SUCCESS" or "FAILURE",
 *           as well as the details of any parts of the test that failed.
 *
 *       Case 3: distance_reg
 *           This runs the test against the set of golden files.  The
 *           minimal output, which in case 2 went to a file, is
 *           instead sent to stderr.  In addition, this displays
 *           images and plots that are specified in the test under
 *           control of the @display variable.  The @display is
 *           only enabled when the regression test is called without
 *           any arguments (case 3).
 *
 *   Regression tests follow the pattern given below.  In an actual
 *   case, comparisons of pix and of files can occur in any order.
 *   We give a specific order here for clarity.
 *
 *       l_int32       success, display, count;
 *       FILE         *fp;  // stream only opened for case 2
 *       L_REGPARAMS  *rp;  // needed for either convenient argument
 *                          // passthrough to subroutines or for
 *                          // automated "write and check"
 *
 *       // Setup variables; optionally open stream
 *       if (regTestSetup(argc, argv, &fp, &display, &success, &rp))
 *           return 1;
 *
 *       // Test pairs of generated pix for identity.  Here we label
 *       // the count on the golden files (0 and 1) explicitly.
 *       regTestComparePix(fp, argv, pix1, pix2, 0, &success);
 *       regTestComparePix(fp, argv, pix1, pix2, 1, &success);
 *
 *       // Test pairs of generated pix for similarity.  These
 *       // generate or test against golden files 2 and 3.
 *       regTestCompareSimilarPix(fp, argv, pix1, pix2, 15, 0.001, 2,
 *                                &success, 0);
 *       regTestCompareSimilarPix(fp, argv, pix1, pix2, 15, 0.0005, 3,
 *                                &success, 0);
 *
 *       // Generation of <newfile*> outputs and testing for identity
 *       // These files can be anything, of course.
 *       regTestCheckFile(fp, argv, <newfile0>, 4, &success);
 *       regTestCheckFile(fp, argv, <newfile1>, 5, &success);
 *
 *       // Test pairs of output golden files for identity.  Here we
 *       // are comparing golden files 4 and 5.
 *       regTestCompareFiles(fp, argv, 4, 5, &success);
 *
 *       // "Write and check".  This writes a pix using a canonical
 *       // formulation for the local filename and either (a) generates a
 *       // golden file if requested or (b) tests the local file
 *       // against a golden file.  Here we are generating or testing
 *       // golden files 6 and 7, which are pix that have been
 *       // compressed with png and jpeg, respectively.  Each time that
 *       // regTestWritePixAndCheck() is called, the count is increased
 *       // by 1 and embedded in the local filename (and, if generating,
 *       // in the golden filename as well).
 *       count = 6;  // initialize it
 *       regTestWritePixAndCheck(pix1, IFF_PNG, &count, rp);
 *       regTestWritePixAndCheck(pix2, IFF_JFIF_JPEG, &count, rp);
 *
 *       // Display when reg test called with only one argument
 *       pixDisplayWithTitle(pix1, 100, 100, NULL, display);
 *
 *       // Clean up and output result
 *       regTestCleanup(argc, argv, fp, success, rp);
 *
 *  Note that the optional returned regparams (defined below) can be
 *  used to pass the parameters cleanly through to subroutines; e.g.,
 *
 *        TestAll(Pix *pixs, L_RegParams *rp) {
 *            ...
 *            regTestCheckFile(rp->fp, rp->argv, fname, n, &rp->success);
 *            ...
 *        }
 * 
 */

#include <stdio.h>

/*-------------------------------------------------------------------------*
 *                     Regression test parameter packer                    *
 *-------------------------------------------------------------------------*/
struct L_RegParams
{
    FILE          *fp;
    char         **argv;
    l_int32        success;
    l_int32        display;
};
typedef struct L_RegParams  L_REGPARAMS;


#endif  /* LEPTONICA_REGUTILS_H */

