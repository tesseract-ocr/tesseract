/**********************************************************************
 * File:			word.c
 * Description: Code for the WERD class.
 * Author:		Ray Smith
 * Created:		Tue Oct 08 14:32:12 BST 1991
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

#ifndef           WERD_H
#define           WERD_H

#include          "varable.h"
#include          "bits16.h"
#include          "strngs.h"
#include          "blckerr.h"
#include          "stepblob.h"
#include          "polyblob.h"
//#include                                                      "larcblob.h"

enum WERD_FLAGS
{
  W_SEGMENTED,                   //< correctly segmented
  W_ITALIC,                      //< italic text
  W_BOLD,                        //< bold text
  W_BOL,                         //< start of line
  W_EOL,                         //< end of line
  W_NORMALIZED,                  //< flags
  W_POLYGON,                     //< approximation
  W_LINEARC,                     //< linearc approx
  W_DONT_CHOP,                   //< fixed pitch chopped
  W_REP_CHAR,                    //< repeated character
  W_FUZZY_SP,                    //< fuzzy space
  W_FUZZY_NON,                   //< fuzzy nonspace
  W_INVERSE                      //< white on black
};

enum DISPLAY_FLAGS
{
  /* Display flags bit number allocations */
  DF_BOX,                        //< Bounding box
  DF_TEXT,                       //< Correct ascii
  DF_POLYGONAL,                  //< Polyg approx
  DF_EDGE_STEP,                  //< Edge steps
  DF_BN_POLYGONAL                //< BL normalisd polyapx
};

class ROW;                       //forward decl

class WERD:public ELIST_LINK
{
  public:
    WERD() {
    }                            //empty constructor
    WERD(                         //constructor
         C_BLOB_LIST *blob_list,  //blobs in word
         uinT8 blanks,            //blanks in front
         const char *text);       //correct text
    WERD(                        //constructor
         PBLOB_LIST *blob_list,  //blobs in word
         uinT8 blanks,           //blanks in front
         const char *text);      //correct text
    WERD(                        //constructor
         PBLOB_LIST *blob_list,  //blobs in word
         WERD *clone);           //use these flags etc.
    WERD(                         //constructor
         C_BLOB_LIST *blob_list,  //blobs in word
         WERD *clone);            //use these flags etc.
    ~WERD () {                   //destructor
      if (flags.bit (W_POLYGON)) {
                                 //use right     destructor
        ((PBLOB_LIST *) & cblobs)->clear ();
                                 //use right     destructor
        ((PBLOB_LIST *) & rej_cblobs)->clear ();
      }
      //              else if (flags.bit(W_LINEARC))
      //                      ((LARC_BLOB_LIST*)&cblobs)->clear();                            //use right     destructor
    }

    WERD *poly_copy(                 //make copy as poly
                    float xheight);  //row xheight
    WERD *larc_copy(                 //make copy as larc
                    float xheight);  //row xheight

                                 //get DUFF compact blobs
    C_BLOB_LIST *rej_cblob_list() {
      if (flags.bit (W_POLYGON))
        WRONG_WORD.error ("WERD::rej_cblob_list", ABORT, NULL);
      return &rej_cblobs;
    }

                                 //get DUFF poly blobs
    PBLOB_LIST *rej_blob_list() {
      if (!flags.bit (W_POLYGON))
        WRONG_WORD.error ("WERD::rej_blob_list", ABORT, NULL);
      return (PBLOB_LIST *) (&rej_cblobs);
    }

    C_BLOB_LIST *cblob_list() {  //get compact blobs
      if (flags.bit (W_POLYGON) || flags.bit (W_LINEARC))
        WRONG_WORD.error ("WERD::cblob_list", ABORT, NULL);
      return &cblobs;
    }
    PBLOB_LIST *blob_list() {  //get poly blobs
      if (!flags.bit (W_POLYGON))
        WRONG_WORD.error ("WERD::blob_list", ABORT, NULL);
                                 //make it right type
      return (PBLOB_LIST *) (&cblobs);
    }
    //      LARC_BLOB_LIST                          *larc_blob_list()                                       //get poly blobs
    //      {
    //              if (!flags.bit(W_LINEARC))
    //                      WRONG_WORD.error("WERD::larc_blob_list",ABORT,NULL);
    //              return (LARC_BLOB_LIST*)(&cblobs);                                              //make it right type
    //      }
    PBLOB_LIST *gblob_list() {  //get generic blobs
                                 //make it right type
      return (PBLOB_LIST *) (&cblobs);
    }

    const char *text() const {  //correct text
      return correct.string ();
    }
    uinT8 space() {  //access function
      return blanks;
    }
    void set_blanks(  //set blanks
                    uinT8 new_blanks) {
      blanks = new_blanks;
    }

    void set_text(                         //replace correct text
                  const char *new_text) {  //with this
      correct = new_text;
    }

    TBOX bounding_box();  //compute bounding box

    BOOL8 flag(                          //test flag
               WERD_FLAGS mask) const {  //flag to test
      return flags.bit (mask);
    }
    void set_flag(                  //set flag value
                  WERD_FLAGS mask,  //flag to test
                  BOOL8 value) {    //value to set
      flags.set_bit (mask, value);
    }

    BOOL8 display_flag(                     //test display flag
                       uinT8 flag) const {  //flag to test
      return disp_flags.bit (flag);
    }

    void set_display_flag(                //set display flag
                          uinT8 flag,     //flag to set
                          BOOL8 value) {  //value to set
      disp_flags.set_bit (flag, value);
    }

    WERD *shallow_copy();  //shallow copy word

    void move(                    // reposition word
              const ICOORD vec);  // by vector

    void scale(                   // scale word
               const float vec);  // by multiplier

    void join_on(                //append word
                 WERD *&other);  //Deleting other

    void copy_on(                //copy blobs
                 WERD *&other);  //from other

    void baseline_normalise (    // Tess style BL Norm
                                 //optional antidote
      ROW * row, DENORM * denorm = NULL);

    void baseline_normalise_x (  //Use non standard xht
      ROW * row, float x_height, //Weird value to use
      DENORM * denorm = NULL);   //optional antidote

    void baseline_denormalise(  //un-normalise
                              const DENORM *denorm);

    void print(            //print
               FILE *fp);  //file to print on

    void plot (                  //draw one
      ScrollView* window,             //window to draw in
                                 //uniform colour
      ScrollView::Color colour, BOOL8 solid = FALSE);

    void plot (                  //draw one
                                 //in rainbow colours
      ScrollView* window, BOOL8 solid = FALSE);

    void plot_rej_blobs (        //draw one
                                 //in rainbow colours
      ScrollView* window, BOOL8 solid = FALSE);

    WERD & operator= (           //assign words
      const WERD & source);      //from this

    void prep_serialise() {  //set ptrs to counts
      correct.prep_serialise ();
      if (flags.bit (W_POLYGON))
        ((PBLOB_LIST *) (&cblobs))->prep_serialise ();
      //              else if (flags.bit(W_LINEARC))
      //                      ((LARC_BLOB_LIST*)(&cblobs))->prep_serialise();
      else
        cblobs.prep_serialise ();
      rej_cblobs.prep_serialise ();
    }

    void dump(  //write external bits
              FILE *f) {
      correct.dump (f);
      if (flags.bit (W_POLYGON))
        ((PBLOB_LIST *) (&cblobs))->dump (f);
      //              else if (flags.bit(W_LINEARC))
      //                      ((LARC_BLOB_LIST*)(&cblobs))->dump( f );
      else
        cblobs.dump (f);
      rej_cblobs.dump (f);
    }

    void de_dump(  //read external bits
                 FILE *f) {
      correct.de_dump (f);
      if (flags.bit (W_POLYGON))
        ((PBLOB_LIST *) (&cblobs))->de_dump (f);
      //              else if (flags.bit(W_LINEARC))
      //                      ((LARC_BLOB_LIST*)(&cblobs))->de_dump( f );
      else
        cblobs.de_dump (f);
      rej_cblobs.de_dump (f);
    }

    make_serialise (WERD) private:
    uinT8 blanks;                //no of blanks
    uinT8 dummy;                 //padding
    BITS16 flags;                //flags about word
    BITS16 disp_flags;           //display flags
    inT16 dummy2;                //padding
    STRING correct;              //correct text
    C_BLOB_LIST cblobs;          //compacted blobs
    C_BLOB_LIST rej_cblobs;      //DUFF blobs
};

ELISTIZEH_S (WERD)
#include          "ocrrow.h"     //placed here due to
extern BOOL_VAR_H (bln_numericmode, 0, "Optimize for numbers");
extern INT_VAR_H (bln_x_height, 128, "Baseline Normalisation X-height");
extern INT_VAR_H (bln_baseline_offset, 64,
"Baseline Norm. offset of baseline");
//void                                                          poly_linearc_outlines(                  //do list of outlines
//LARC_OUTLINE_LIST                             *srclist,                                                       //list to convert
//OUTLINE_LIST                                  *destlist                                                       //desstination list
//);
//OUTLINE                                                       *poly_larcline(                                 //draw it
//LARC_OUTLINE                                  *srcline                                                                //one to approximate
//);
int word_comparator(                     //sort blobs
                    const void *word1p,  //ptr to ptr to word1
                    const void *word2p   //ptr to ptr to word2
                   );
#endif
