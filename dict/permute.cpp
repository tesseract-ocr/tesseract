/* -*-C-*-
 ********************************************************************************
 *
 * File:        permute.c  (Formerly permute.c)
 * Description:  Choose OCR text given character-probability maps
 *               for sequences of glyph fragments and a dictionary provided as
 *               a Dual Acyclic Word Graph.
 *               In this file, "permute" should be read "combine."
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Sep 22 14:05:51 1989
 * Modified:     Thu Jan  3 16:38:46 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1989, Hewlett-Packard Company.
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
 *********************************************************************************/
/*----------------------------------------------------------------------
            I n c l u d e s
---------------------------------------------------------------------*/

#include <assert.h>
#include <math.h>

#include "fstmodel.h"

#include "const.h"

#include "permute.h"

#include "callcpp.h"
#include "choices.h"
#include "context.h"
#include "conversion.h"
#include "debug.h"
#include "freelist.h"
#include "globals.h"
#include "hyphen.h"
#include "ndminx.h"
#include "permdawg.h"
#include "permngram.h"
#include "permnum.h"
#include "ratngs.h"
#include "stopper.h"
#include "tordvars.h"
#include "tprintf.h"
#include "trie.h"
#include "varable.h"
#include "unicharset.h"
#include "dict.h"
#include "image.h"
#include "ccutil.h"

int permutation_count;           // Used in metrics.cpp.
/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
// TODO(tkielbus) Choose a value for the MAX_NUM_EDGES constant
// (or make it dynamic)
#define MAX_NUM_EDGES          2000000
#define MAX_DOC_EDGES          250000
#define RESERVED_DOC_EDGES     10000
#define MAX_USER_EDGES         50000
#define USER_RESERVED_EDGES    2000
                                 /* Weights for adjustment */
#define NON_WERD               1.25
#define GARBAGE_STRING         1.5
#define MAX_PERM_LENGTH         128

EDGE_ARRAY pending_words;
EDGE_ARRAY document_words;
EDGE_ARRAY user_words;
EDGE_ARRAY word_dawg;

INT_VAR(fragments_debug, 0, "Debug character fragments");

BOOL_VAR(segment_debug, 0, "Debug the whole segmentation process");

BOOL_VAR(segment_adjust_debug, 0, "Segmentation adjustment debug");

double_VAR(segment_penalty_dict_nonword, NON_WERD,
           "Score multiplier for glyph fragment segmentations which do not "
           "match a dictionary word (lower is better).");

double_VAR(segment_penalty_garbage, GARBAGE_STRING,
           "Score multiplier for poorly cased strings that are not in the "
           "dictionary and generally look like garbage (lower is better).");

BOOL_VAR(save_doc_words, 0, "Save Document Words");

BOOL_VAR(doc_dict_enable, 1, "Enable Document Dictionary ");
/* PREV DEFAULT 0 */

BOOL_VAR(ngram_permuter_activated, FALSE,
         "Activate character-level n-gram-based permuter");

BOOL_VAR(fst_activated, FALSE, "Activate fst");

int permute_only_top = 0;

#if 0
                                 //0x0=.
static inT32 bigram_counts[256][3] = { {
    0, 0, 0
  },
  {                              //0x1=.
    0, 0, 0
  },
  {                              //0x2=.
    0, 0, 0
  },
  {                              //0x3=.
    0, 0, 0
  },
  {                              //0x4=.
    0, 0, 0
  },
  {                              //0x5=.
    0, 0, 0
  },
  {                              //0x6=.
    0, 0, 0
  },
  {                              //0x7=.
    0, 0, 0
  },
  {                              //0x8=.
    0, 0, 0
  },
  {                              //0x9=.
    0, 0, 0
  },
  {                              //0xa=.
    93, 28, 0
  },
  {                              //0xb=.
    0, 0, 0
  },
  {                              //0xc=.
    0, 0, 0
  },
  {                              //0xd=.
    0, 0, 0
  },
  {                              //0xe=.
    0, 0, 0
  },
  {                              //0xf=.
    0, 0, 0
  },
  {                              //0x10=.
    0, 0, 0
  },
  {                              //0x11=.
    0, 0, 0
  },
  {                              //0x12=.
    0, 0, 0
  },
  {                              //0x13=.
    0, 0, 0
  },
  {                              //0x14=.
    0, 0, 0
  },
  {                              //0x15=.
    0, 0, 0
  },
  {                              //0x16=.
    0, 0, 0
  },
  {                              //0x17=.
    0, 0, 0
  },
  {                              //0x18=.
    0, 0, 0
  },
  {                              //0x19=.
    0, 0, 0
  },
  {                              //0x1a=.
    0, 0, 0
  },
  {                              //0x1b=.
    0, 0, 0
  },
  {                              //0x1c=.
    0, 0, 0
  },
  {                              //0x1d=.
    0, 0, 0
  },
  {                              //0x1e=.
    0, 0, 0
  },
  {                              //0x1f=.
    0, 0, 0
  },
  {                              //0x20=
    324, 377, 2
  },
  {                              //0x21=!
    2, 1, 0
  },
  {                              //0x22="
    2, 1, 0
  },
  {                              //0x23=#
    1, 0, 1
  },
  {                              //0x24=$
    2, 1, 0
  },
  {                              //0x25=%
    2, 0, 0
  },
  {                              //0x26=&
    2, 1, 0
  },
  {                              //0x27='
    1, 21, 8
  },
  {                              //0x28=(
    2, 1, 0
  },
  {                              //0x29=)
    19, 0, 0
  },
  {                              //0x2a=*
    2, 1, 0
  },
  {                              //0x2b=+
    1, 0, 0
  },
  {                              //0x2c=,
    75, 4, 0
  },
  {                              //0x2d=-
    52, 7, 0
  },
  {                              //0x2e=.
    190, 16, 3
  },
  {                              //0x2f=/
    53, 2, 0
  },
  {                              //0x30=0
    399, 0, 0
  },
  {                              //0x31=1
    220, 0, 0
  },
  {                              //0x32=2
    226, 0, 0
  },
  {                              //0x33=3
    128, 0, 0
  },
  {                              //0x34=4
    147, 0, 0
  },
  {                              //0x35=5
    179, 0, 1
  },
  {                              //0x36=6
    173, 0, 0
  },
  {                              //0x37=7
    115, 0, 0
  },
  {                              //0x38=8
    107, 0, 0
  },
  {                              //0x39=9
    934, 0, 1
  },
  {                              //0x3a=:
    27, 0, 1
  },
  {                              //0x3b=;
    2, 1, 0
  },
  {                              //0x3c=<
    2, 1, 0
  },
  {                              //0x3d==
    2, 1, 0
  },
  {                              //0x3e=>
    2, 1, 0
  },
  {                              //0x3f=?
    2, 1, 0
  },
  {                              //0x40=@
    2, 1, 0
  },
  {                              //0x41=A
    3, 1, 0
  },
  {                              //0x42=B
    1, 73, 0
  },
  {                              //0x43=C
    1, 6, 0
  },
  {                              //0x44=D
    1, 24, 0
  },
  {                              //0x45=E
    1, 2, 0
  },
  {                              //0x46=F
    1, 19, 0
  },
  {                              //0x47=G
    1, 2, 0
  },
  {                              //0x48=H
    3, 2, 1
  },
  {                              //0x49=I
    0, 68, 0
  },
  {                              //0x4a=J
    1, 2, 0
  },
  {                              //0x4b=K
    1, 2, 0
  },
  {                              //0x4c=L
    1, 82, 0
  },
  {                              //0x4d=M
    10, 10, 0
  },
  {                              //0x4e=N
    3, 239, 0
  },
  {                              //0x4f=O
    1, 10, 0
  },
  {                              //0x50=P
    0, 1, 3
  },
  {                              //0x51=Q
    2, 3, 0
  },
  {                              //0x52=R
    1, 43, 0
  },
  {                              //0x53=S
    1, 53, 0
  },
  {                              //0x54=T
    2, 18, 0
  },
  {                              //0x55=U
    1, 2, 0
  },
  {                              //0x56=V
    1, 17, 0
  },
  {                              //0x57=W
    1, 5, 0
  },
  {                              //0x58=X
    1, 6, 0
  },
  {                              //0x59=Y
    1, 2, 0
  },
  {                              //0x5a=Z
    1, 2, 0
  },
  {                              //0x5b=[
    2, 1, 0
  },
  {                              //0x5c=backslash
    2, 1, 0
  },
  {                              //0x5d=]
    2, 1, 0
  },
  {                              //0x5e=^
    2, 1, 0
  },
  {                              //0x5f=_
    2, 1, 0
  },
  {                              //0x60=`
    1, 0, 2
  },
  {                              //0x61=a
    0, 0, 671
  },
  {                              //0x62=b
    0, 1, 16
  },
  {                              //0x63=c
    0, 2, 1
  },
  {                              //0x64=d
    0, 14, 0
  },
  {                              //0x65=e
    0, 0, 763
  },
  {                              //0x66=f
    0, 186, 0
  },
  {                              //0x67=g
    0, 2, 1
  },
  {                              //0x68=h
    0, 2, 1
  },
  {                              //0x69=i
    0, 0, 818
  },
  {                              //0x6a=j
    0, 2, 1
  },
  {                              //0x6b=k
    0, 4, 1
  },
  {                              //0x6c=l
    0, 26, 3
  },
  {                              //0x6d=m
    0, 69, 0
  },
  {                              //0x6e=n
    0, 885, 0
  },
  {                              //0x6f=o
    0, 17, 722
  },
  {                              //0x70=p
    0, 1, 5
  },
  {                              //0x71=q
    2, 1, 0
  },
  {                              //0x72=r
    0, 21, 0
  },
  {                              //0x73=s
    3, 49, 0
  },
  {                              //0x74=t
    0, 219, 5
  },
  {                              //0x75=u
    0, 0, 56
  },
  {                              //0x76=v
    0, 4, 0
  },
  {                              //0x77=w
    0, 2, 1
  },
  {                              //0x78=x
    0, 2, 1
  },
  {                              //0x79=y
    0, 1, 23
  },
  {                              //0x7a=z
    0, 2, 1
  },
  {                              //0x7b={
    2, 1, 0
  },
  {                              //0x7c=|
    59, 0, 3
  },
  {                              //0x7d=}
    2, 1, 0
  },
  {                              //0x7e=~
    2, 1, 0
  },
  {                              //0x7f=.
    0, 0, 0
  },
  {                              //0x80=.
    0, 0, 0
  },
  {                              //0x81=.
    0, 0, 0
  },
  {                              //0x82=.
    0, 0, 0
  },
  {                              //0x83=.
    0, 0, 0
  },
  {                              //0x84=.
    0, 0, 0
  },
  {                              //0x85=.
    0, 0, 0
  },
  {                              //0x86=.
    0, 0, 0
  },
  {                              //0x87=.
    0, 0, 0
  },
  {                              //0x88=.
    0, 0, 0
  },
  {                              //0x89=.
    0, 0, 0
  },
  {                              //0x8a=.
    0, 0, 0
  },
  {                              //0x8b=.
    0, 0, 0
  },
  {                              //0x8c=.
    0, 0, 0
  },
  {                              //0x8d=.
    0, 0, 0
  },
  {                              //0x8e=.
    0, 0, 0
  },
  {                              //0x8f=.
    0, 0, 0
  },
  {                              //0x90=.
    0, 0, 0
  },
  {                              //0x91=.
    0, 0, 0
  },
  {                              //0x92=.
    0, 0, 0
  },
  {                              //0x93=.
    0, 0, 0
  },
  {                              //0x94=.
    0, 0, 0
  },
  {                              //0x95=.
    0, 0, 0
  },
  {                              //0x96=.
    0, 0, 0
  },
  {                              //0x97=.
    0, 0, 0
  },
  {                              //0x98=.
    0, 0, 0
  },
  {                              //0x99=.
    0, 0, 0
  },
  {                              //0x9a=.
    0, 0, 0
  },
  {                              //0x9b=.
    0, 0, 0
  },
  {                              //0x9c=.
    0, 0, 0
  },
  {                              //0x9d=.
    0, 0, 0
  },
  {                              //0x9e=.
    0, 0, 0
  },
  {                              //0x9f=.
    0, 0, 0
  },
  {                              //0xa0=.
    0, 0, 0
  },
  {                              //0xa1=.
    0, 0, 0
  },
  {                              //0xa2=.
    0, 0, 0
  },
  {                              //0xa3=.
    0, 0, 0
  },
  {                              //0xa4=.
    0, 0, 0
  },
  {                              //0xa5=.
    0, 0, 0
  },
  {                              //0xa6=.
    0, 0, 0
  },
  {                              //0xa7=.
    0, 0, 0
  },
  {                              //0xa8=.
    0, 0, 0
  },
  {                              //0xa9=.
    0, 0, 0
  },
  {                              //0xaa=.
    0, 0, 0
  },
  {                              //0xab=.
    0, 0, 0
  },
  {                              //0xac=.
    0, 0, 0
  },
  {                              //0xad=.
    0, 0, 0
  },
  {                              //0xae=.
    0, 0, 0
  },
  {                              //0xaf=.
    0, 0, 0
  },
  {                              //0xb0=.
    0, 0, 0
  },
  {                              //0xb1=.
    0, 0, 0
  },
  {                              //0xb2=.
    0, 0, 0
  },
  {                              //0xb3=.
    0, 0, 0
  },
  {                              //0xb4=.
    0, 0, 0
  },
  {                              //0xb5=.
    0, 0, 0
  },
  {                              //0xb6=.
    0, 0, 0
  },
  {                              //0xb7=.
    0, 0, 0
  },
  {                              //0xb8=.
    0, 0, 0
  },
  {                              //0xb9=.
    0, 0, 0
  },
  {                              //0xba=.
    0, 0, 0
  },
  {                              //0xbb=.
    0, 0, 0
  },
  {                              //0xbc=.
    0, 0, 0
  },
  {                              //0xbd=.
    0, 0, 0
  },
  {                              //0xbe=.
    0, 0, 0
  },
  {                              //0xbf=.
    0, 0, 0
  },
  {                              //0xc0=.
    0, 0, 0
  },
  {                              //0xc1=.
    0, 0, 0
  },
  {                              //0xc2=.
    0, 0, 0
  },
  {                              //0xc3=.
    0, 0, 0
  },
  {                              //0xc4=.
    0, 0, 0
  },
  {                              //0xc5=.
    0, 0, 0
  },
  {                              //0xc6=.
    0, 0, 0
  },
  {                              //0xc7=.
    0, 0, 0
  },
  {                              //0xc8=.
    0, 0, 0
  },
  {                              //0xc9=.
    0, 0, 0
  },
  {                              //0xca=.
    0, 0, 0
  },
  {                              //0xcb=.
    0, 0, 0
  },
  {                              //0xcc=.
    0, 0, 0
  },
  {                              //0xcd=.
    0, 0, 0
  },
  {                              //0xce=.
    0, 0, 0
  },
  {                              //0xcf=.
    0, 0, 0
  },
  {                              //0xd0=.
    0, 0, 0
  },
  {                              //0xd1=.
    0, 0, 0
  },
  {                              //0xd2=.
    0, 0, 0
  },
  {                              //0xd3=.
    0, 0, 0
  },
  {                              //0xd4=.
    0, 0, 0
  },
  {                              //0xd5=.
    0, 0, 0
  },
  {                              //0xd6=.
    0, 0, 0
  },
  {                              //0xd7=.
    0, 0, 0
  },
  {                              //0xd8=.
    0, 0, 0
  },
  {                              //0xd9=.
    0, 0, 0
  },
  {                              //0xda=.
    0, 0, 0
  },
  {                              //0xdb=.
    0, 0, 0
  },
  {                              //0xdc=.
    0, 0, 0
  },
  {                              //0xdd=.
    0, 0, 0
  },
  {                              //0xde=.
    0, 0, 0
  },
  {                              //0xdf=.
    0, 0, 0
  },
  {                              //0xe0=.
    0, 0, 0
  },
  {                              //0xe1=.
    0, 0, 0
  },
  {                              //0xe2=.
    0, 0, 0
  },
  {                              //0xe3=.
    0, 0, 0
  },
  {                              //0xe4=.
    0, 0, 0
  },
  {                              //0xe5=.
    0, 0, 0
  },
  {                              //0xe6=.
    0, 0, 0
  },
  {                              //0xe7=.
    0, 0, 0
  },
  {                              //0xe8=.
    0, 0, 0
  },
  {                              //0xe9=.
    0, 0, 0
  },
  {                              //0xea=.
    0, 0, 0
  },
  {                              //0xeb=.
    0, 0, 0
  },
  {                              //0xec=.
    0, 0, 0
  },
  {                              //0xed=.
    0, 0, 0
  },
  {                              //0xee=.
    0, 0, 0
  },
  {                              //0xef=.
    0, 0, 0
  },
  {                              //0xf0=.
    0, 0, 0
  },
  {                              //0xf1=.
    0, 0, 0
  },
  {                              //0xf2=.
    0, 0, 0
  },
  {                              //0xf3=.
    0, 0, 0
  },
  {                              //0xf4=.
    0, 0, 0
  },
  {                              //0xf5=.
    0, 0, 0
  },
  {                              //0xf6=.
    0, 0, 0
  },
  {                              //0xf7=.
    0, 0, 0
  },
  {                              //0xf8=.
    0, 0, 0
  },
  {                              //0xf9=.
    0, 0, 0
  },
  {                              //0xfa=.
    0, 0, 0
  },
  {                              //0xfb=.
    0, 0, 0
  },
  {                              //0xfc=.
    0, 0, 0
  },
  {                              //0xfd=.
    0, 0, 0
  },
  {                              //0xfe=.
    0, 0, 0
  },
  {                              //0xff=.
    0, 0, 0
  },
};
#endif

//extern "C" double permuter_pending_threshold;

                                 /* Similarity matcher values */
#define SIM_CERTAINTY_SCALE  -10.0
                                 /* Similarity matcher values */
#define SIM_CERTAINTY_OFFSET -10.0
                                 /* Worst E*L product to stop on */
#define SIMILARITY_FLOOR     100.0
/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/

/**********************************************************************
 * get_best_delete_other
 *
 * Returns the best of two choices and deletes the other (worse) choice.
 * A choice is better if it has a non-empty string and has a lower
 * rating than the other choice. If the ratings are the same,
 * choice2 is preferred over choice1.
 **********************************************************************/
WERD_CHOICE *get_best_delete_other(WERD_CHOICE *choice1,
                                   WERD_CHOICE *choice2) {
  if (!choice1) return choice2;
  if (!choice2) return choice1;
  if (choice1->rating() < choice2->rating() || choice2->length() == 0) {
    delete choice2;
    return choice1;
  } else {
    delete choice1;
    return choice2;
  }
}


/**********************************************************************
 * good_choice
 *
 * Return TRUE if a good answer is found for the unknown blob rating.
 **********************************************************************/
int good_choice(const WERD_CHOICE &choice) {
  register float certainty;
  if (similarity_enable) {
    if ((choice.rating() + 1) * choice.certainty() > SIMILARITY_FLOOR)
      return false;
    certainty =
      SIM_CERTAINTY_OFFSET + choice.rating() * SIM_CERTAINTY_SCALE;
  } else {
    certainty = choice.certainty();
  }

  return (certainty > certainty_threshold) ? true : false;
}


/**********************************************************************
 * add_document_word
 *
 * Add a word found on this document to the document specific
 * dictionary.
 **********************************************************************/
namespace tesseract {
void Dict::add_document_word(const WERD_CHOICE &best_choice) {
  char filename[CHARS_PER_LINE];
  FILE *doc_word_file;
  const char *string = best_choice.unichar_string().string();
  int stringlen = best_choice.length();

  if (!doc_dict_enable
    || valid_word (string) || CurrentWordAmbig () || stringlen < 2)
    return;

  if (!good_choice(best_choice) || stringlen == 2) {
    if (best_choice.certainty() < permuter_pending_threshold)
      return;
    if (!word_in_dawg(pending_words, string)) {
      if (stringlen > 2 ||
          (stringlen == 2 &&
           getUnicharset().get_isupper(best_choice.unichar_id(0)) &&
           getUnicharset().get_isupper(best_choice.unichar_id(1)))) {
        add_word_to_dawg(pending_words, string,
                         MAX_DOC_EDGES, RESERVED_DOC_EDGES);
      }
      return;
    }
  }

  if (save_doc_words) {
    strcpy(filename, getImage()->getCCUtil()->imagefile.string());
    strcat (filename, ".doc");
    doc_word_file = open_file (filename, "a");
    fprintf (doc_word_file, "%s\n", string);
    fclose(doc_word_file);
  }
  add_word_to_dawg(document_words, string, MAX_DOC_EDGES, RESERVED_DOC_EDGES);
}


/**********************************************************************
 * adjust_non_word
 *
 * Assign an adjusted value to a string that is a non-word.  The value
 * that this word choice has is based on case and punctuation rules.
 **********************************************************************/
void Dict::adjust_non_word(const char *word, const char *word_lengths,
                           float rating, float *new_rating,
                           float *adjust_factor) {
  if (segment_adjust_debug)
    cprintf("Non-word: %s %4.2f ", word, rating);

  *new_rating = rating + RATING_PAD;
  if (case_ok(word, word_lengths) &&
      punctuation_ok(word, word_lengths) != -1) {
    *new_rating *= segment_penalty_dict_nonword;
    *adjust_factor = segment_penalty_dict_nonword;
    if (segment_adjust_debug)
      cprintf(", %4.2f ", (double)segment_penalty_dict_nonword);
  } else {
    *new_rating *= segment_penalty_garbage;
    *adjust_factor = segment_penalty_garbage;
    if (segment_adjust_debug) {
      if (!case_ok(word, word_lengths))
        cprintf (", C");
      if (punctuation_ok(word, word_lengths) == -1)
        cprintf (", P");
      cprintf (", %4.2f ", (double)segment_penalty_garbage);
    }
  }

  *new_rating -= RATING_PAD;

  if (segment_adjust_debug)
    cprintf (" --> %4.2f\n", *new_rating);
}


/**********************************************************************
 * init_permute
 *
 * Initialize anything that needs to be set up for the permute
 * functions.
 **********************************************************************/
void Dict::init_permute() {
  if (word_dawg != NULL)
    end_permute();
  init_permdawg();
  STRING name;
  name = getImage()->getCCUtil()->language_data_path_prefix;
  name += "word-dawg";
  word_dawg = read_squished_dawg(name.string());

  document_words =
      (EDGE_ARRAY) memalloc (sizeof (EDGE_RECORD) * MAX_DOC_EDGES);
  initialize_dawg(document_words, MAX_DOC_EDGES);

  pending_words =
      (EDGE_ARRAY) memalloc (sizeof (EDGE_RECORD) * MAX_DOC_EDGES);
  initialize_dawg(pending_words, MAX_DOC_EDGES);

  user_words = (EDGE_ARRAY) memalloc (sizeof (EDGE_RECORD) * MAX_USER_EDGES);
  name = getImage()->getCCUtil()->language_data_path_prefix;
  name += "user-words";
  read_word_list(name.string(), user_words, MAX_USER_EDGES, USER_RESERVED_EDGES);
  if (fst_activated) {
    STRING name;
    name = getImage()->getCCUtil()->language_data_path_prefix;
    name += "fst";
    LanguageModel::Instance()->InitWithLanguage(name.string());
    letter_is_okay_ = &tesseract::Dict::fst_letter_is_okay;
  }
}

void Dict::end_permute() {
  if (word_dawg == NULL)
    return;  // Not safe to call twice.
  memfree(word_dawg);
  word_dawg = NULL;
  memfree(document_words);
  document_words =  NULL;
  memfree(pending_words);
  pending_words = NULL;
  memfree(user_words);
  user_words = NULL;
  end_permdawg();
}

/**********************************************************************
 * permute_all
 *
 * Permute all the characters together using all of the different types
 * of permuters/selectors available.  Each of the characters must have
 * a non-NULL choice list.
 *
 * Note: order of applying permuters does matter, since the latter
 * permuter will be recorded if the resulting word ratings are the same.
 **********************************************************************/
WERD_CHOICE *Dict::permute_all(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                               float rating_limit,
                               WERD_CHOICE *raw_choice) {
  A_CHOICE *a_choice;
  WERD_CHOICE *result1;
  WERD_CHOICE *result2 = NULL;
  BOOL8 any_alpha;
  int free_index;
  float top_choice_rating_limit = rating_limit;
  CHOICES_LIST old_char_choices = NULL;

  // Initialize result1 from the result of permute_top_choice.
  result1 = permute_top_choice(char_choices, &top_choice_rating_limit,
                               raw_choice, &any_alpha);

  // Permute character fragments if necessary.
  if (result1 == NULL || result1->fragment_mark()) {
    result2 = top_fragments_permute_and_select(char_choices,
                                               top_choice_rating_limit);
    result1 = get_best_delete_other(result1, result2);
  }

  if (ngram_permuter_activated) {
    old_char_choices = convert_to_choices_list(char_choices, getUnicharset());
    A_CHOICE *ngram_choice =
      ngram_permute_and_select(old_char_choices, rating_limit, word_dawg);
    free_all_choices(old_char_choices, free_index);
    if (ngram_choice == NULL) return NULL;
    result1 = new WERD_CHOICE();
    convert_to_word_choice(ngram_choice, getUnicharset(), result1);
    return result1;
  }

  if (result1 == NULL)
    return (NULL);
  if (permute_only_top)
    return result1;

  old_char_choices = convert_to_choices_list(char_choices, getUnicharset());

  if (display_ratings > 1) {
    int i;
    cprintf("\nold_char_choices in permute_characters: ");
    for_each_choice(old_char_choices, i) {
      print_choices("", (CHOICES) array_index (old_char_choices, i));
      cprintf("\n");
    }
  }

  if (any_alpha && char_choices.length() <= MAX_WERD_LENGTH) {
    a_choice = permute_words(old_char_choices, rating_limit);
    result1 = get_best_delete_other(getUnicharset(), result1, a_choice);
  }

  a_choice = number_permute_and_select(old_char_choices, rating_limit);
  result1 = get_best_delete_other(getUnicharset(), result1, a_choice);

  result2 = permute_compound_words(char_choices, rating_limit);
  result1 = get_best_delete_other(result1, result2);

  free_all_choices(old_char_choices, free_index);
  return (result1);
}


/**********************************************************************
 * permute_characters
 *
 * Permute these characters together according to each of the different
 * permuters that are enabled.
 **********************************************************************/
void Dict::permute_characters(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                              float limit,
                              WERD_CHOICE *best_choice,
                              WERD_CHOICE *raw_choice) {
  float old_raw_choice_rating = raw_choice->rating();

  if (display_ratings > 1) {
    cprintf("\nchar_choices in permute_characters:\n");
    for (int i = 0; i < char_choices.length(); ++i) {
      print_ratings_list("", char_choices.get(i), getUnicharset());
      cprintf("\n");
    }
  }

  permutation_count++;           /* Global counter */

  WERD_CHOICE *this_choice = permute_all(char_choices, limit, raw_choice);

  if (raw_choice->rating() < old_raw_choice_rating) {
    // Populate unichars_ and unichar_lengths_ of raw_choice. This is
    // needed for various components that still work with unichars rather
    // than unichar ids (e.g. AdaptToWord).
    raw_choice->populate_unichars(getUnicharset());
  }
  if (this_choice && this_choice->rating() < best_choice->rating()) {
    *best_choice = *this_choice;
    // Populate unichars_ and unichar_lengths_ of best_choice. This is
    // needed for various components that still work with unichars rather
    // than unichar ids (dawg, *_ok functions, various hard-coded hacks).
    best_choice->populate_unichars(getUnicharset());

    if (display_ratings) {
      cprintf("permute_characters: %s\n",
              best_choice->debug_string(getUnicharset()).string());
    }
  }
  delete this_choice;
}


/**********************************************************************
 * permute_compound_words
 *
 * Return the top choice for each character as the choice for the word.
 **********************************************************************/
WERD_CHOICE *Dict::permute_compound_words(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    float rating_limit) {
  BLOB_CHOICE *first_choice;
  WERD_CHOICE *best_choice = NULL;
  WERD_CHOICE current_word(MAX_WERD_LENGTH);
  int first_index = 0;
  int x;
  BLOB_CHOICE_IT blob_choice_it;

  if (char_choices.length() > MAX_WERD_LENGTH) {
    WERD_CHOICE *bad_word_choice = new WERD_CHOICE();
    bad_word_choice->make_bad();
    return bad_word_choice;
  }

  UNICHAR_ID slash = getUnicharset().unichar_to_id("/");
  UNICHAR_ID dash = getUnicharset().unichar_to_id("-");
  for (x = 0; x < char_choices.length(); ++x) {
    blob_choice_it.set_to_list(char_choices.get(x));
    first_choice = blob_choice_it.data();
    if (first_choice->unichar_id() == slash ||
        first_choice->unichar_id() == dash) {
      if (x > first_index) {
        if (segment_debug)
          cprintf ("Hyphenated word found\n");
        permute_subword(char_choices, rating_limit, first_index,
                        x - 1, &current_word);
        if (current_word.rating() > rating_limit)
          break;
      }
      // Append hyphen/slash separator to current_word.
      current_word.append_unichar_id_space_allocated(
          first_choice->unichar_id(), 1,
          first_choice->rating(), first_choice->certainty());

      first_index = x + 1;  // update first_index
    }
  }

  if (first_index > 0 && first_index < x &&
      current_word.rating() <= rating_limit) {
    permute_subword(char_choices, rating_limit, first_index,
                    x - 1, &current_word);
    best_choice = new WERD_CHOICE(current_word);
    best_choice->set_permuter(COMPOUND_PERM);
  }
  return (best_choice);
}


/**********************************************************************
 * permute_subword
 *
 * Permute a part of a compound word this subword is bounded by hyphens
 * and the start and end of the word.  Call the standard word permute
 * function on a set of choices covering only part of the original
 * word.  When it is done reclaim the memory that was used in the
 * excercise.
 **********************************************************************/
void Dict::permute_subword(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                           float rating_limit,
                           int start,
                           int end,
                           WERD_CHOICE *current_word) {
  int x;
  BLOB_CHOICE_LIST_VECTOR subchoices;
  WERD_CHOICE *best_choice = NULL;
  WERD_CHOICE raw_choice;
  raw_choice.make_bad();

  DisableChoiceAccum();

  for (x = start; x <= end; x++) {
    if (char_choices.get(x) != NULL) {
      subchoices += char_choices.get(x);
    }
  }

  if (!subchoices.empty()) {
    if (segment_debug)
      segment_dawg_debug.set_value(true);
    best_choice = permute_all(subchoices, rating_limit, &raw_choice);

    if (segment_debug)
      segment_dawg_debug.set_value(false);

    if (best_choice && best_choice->length() > 0) {
      *current_word += *best_choice;
    } else {
      current_word->set_rating(MAX_FLOAT32);
    }
  } else {
    current_word->set_rating(MAX_FLOAT32);
  }

  if (best_choice)
    delete best_choice;

  if (segment_debug && current_word->rating() < MAX_FLOAT32) {
    cprintf ("Subword permuted = %s, %5.2f, %5.2f\n\n",
             current_word->debug_string(getUnicharset()).string(),
             current_word->rating(), current_word->certainty());
  }

  EnableChoiceAccum();
}


/**********************************************************************
 * permute_top_choice
 *
 * Return the top choice for each character as the choice for the word.
 * In addition a choice is created for the best lower and upper case
 * non-words.  In each character position the best lower (or upper) case
 * character is substituted for the best overall character.
 **********************************************************************/
WERD_CHOICE *Dict::permute_top_choice(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    float* rating_limit,
    WERD_CHOICE *raw_choice,
    BOOL8 *any_alpha) {
  BLOB_CHOICE *first_choice;
  const char *first_char;             //first choice
  const char *second_char;            //second choice
  const char *third_char;             //third choice
  char prev_char[UNICHAR_LEN + 1];    //prev in word
  const char *next_char = "";         //next in word
  const char *next_next_char = "";    //after next next in word

  WERD_CHOICE word(MAX_PERM_LENGTH);
  word.set_permuter(TOP_CHOICE_PERM);
  WERD_CHOICE capital_word(MAX_PERM_LENGTH);
  capital_word.set_permuter(UPPER_CASE_PERM);
  WERD_CHOICE lower_word(MAX_PERM_LENGTH);
  lower_word.set_permuter(LOWER_CASE_PERM);

  int x;
  BOOL8 char_alpha;

  float first_rating = 0;

  float new_rating;
  float adjust_factor;

  float certainties[MAX_PERM_LENGTH + 1];
  float lower_certainties[MAX_PERM_LENGTH + 1];
  float upper_certainties[MAX_PERM_LENGTH + 1];

  BLOB_CHOICE_IT blob_choice_it;
  UNICHAR_ID temp_id;
  UNICHAR_ID unichar_id;
  UNICHAR_ID space = getUnicharset().unichar_to_id(" ");
  register const char* ch;
  register inT8 lower_done;
  register inT8 upper_done;

  STRING unichar_str;
  STRING unichar_lengths;

  prev_char[0] = '\0';

  if (any_alpha != NULL)
    *any_alpha = FALSE;

  if (char_choices.length() > MAX_PERM_LENGTH) {
    return (NULL);
  }

  for (x = 0; x < char_choices.length(); ++x) {
    if (x + 1 < char_choices.length()) {
      blob_choice_it.set_to_list(char_choices.get(x+1));
      unichar_id = blob_choice_it.data()->unichar_id();
      next_char = unichar_id != INVALID_UNICHAR_ID ?
        getUnicharset().id_to_unichar(unichar_id) : "";
    } else {
      next_char = "";
    }

    if (x + 2 < char_choices.length()) {
      blob_choice_it.set_to_list(char_choices.get(x+2));
      unichar_id = blob_choice_it.data()->unichar_id();
      next_next_char = unichar_id != INVALID_UNICHAR_ID ?
        getUnicharset().id_to_unichar(unichar_id) : "";
    } else {
      next_next_char = "";
    }

    blob_choice_it.set_to_list(char_choices.get(x));
    first_choice = NULL;
    for (blob_choice_it.mark_cycle_pt(); !blob_choice_it.cycled_list();
         blob_choice_it.forward()) {  // find the best non-fragment char choice
      temp_id = blob_choice_it.data()->unichar_id();
      if (!(getUnicharset().get_fragment(temp_id))) {
        first_choice = blob_choice_it.data();
        break;
      } else if (char_choices.length() > 1) {
        word.set_fragment_mark(true);
        capital_word.set_fragment_mark(true);
        lower_word.set_fragment_mark(true);
      }
    }
    if (first_choice == NULL) {
      cprintf("Permuter found only fragments for"
              " character at position %d; word=%s\n",
              x, word.debug_string(getUnicharset()).string());
    }
    ASSERT_HOST(first_choice != NULL);

    unichar_id = first_choice->unichar_id() != INVALID_UNICHAR_ID ?
      first_choice->unichar_id() : space;
    first_char = getUnicharset().id_to_unichar(unichar_id);
    first_rating = first_choice->rating();
    word.append_unichar_id_space_allocated(
        unichar_id, 1, first_choice->rating(), first_choice->certainty());
    capital_word.append_unichar_id_space_allocated(
        unichar_id, 1, first_choice->rating(), first_choice->certainty());
    lower_word.append_unichar_id_space_allocated(
        unichar_id, 1, first_choice->rating(), first_choice->certainty());

    certainties[x] = first_choice->certainty();
    lower_certainties[x] = first_choice->certainty();
    upper_certainties[x] = first_choice->certainty();

    lower_done = FALSE;
    upper_done = FALSE;
    char_alpha = FALSE;
    second_char = "";
    third_char = "";
    for (; !blob_choice_it.cycled_list(); blob_choice_it.forward()) {
      unichar_id = blob_choice_it.data()->unichar_id();
      if (getUnicharset().eq(unichar_id, "l") && !blob_choice_it.at_last() &&
          blob_choice_it.data_relative(1)->rating() == first_rating) {
        blob_choice_it.forward();
        temp_id = blob_choice_it.data()->unichar_id();
        if (getUnicharset().eq(temp_id, "1") ||
            getUnicharset().eq(temp_id, "I")) {
          second_char = getUnicharset().id_to_unichar(temp_id);
          if (!blob_choice_it.at_last() &&
              blob_choice_it.data_relative(1)->rating() == first_rating) {
            blob_choice_it.forward();
            temp_id = blob_choice_it.data()->unichar_id();
            if (getUnicharset().eq(temp_id, "1") ||
                getUnicharset().eq(temp_id, "I")) {
              third_char = getUnicharset().id_to_unichar(temp_id);
              blob_choice_it.forward();
            }
          }
          ch = choose_il1 (first_char, second_char, third_char,
            prev_char, next_char, next_next_char);
          unichar_id = (ch != NULL && *ch != '\0') ?
            getUnicharset().unichar_to_id(ch) : INVALID_UNICHAR_ID;
          if (strcmp(ch, "l") != 0 &&
              getUnicharset().eq(word.unichar_id(x), "l")) {
            word.set_unichar_id(unichar_id, x);
            lower_word.set_unichar_id(unichar_id, x);
            capital_word.set_unichar_id(unichar_id, x);
          }
        }
      }
      if (unichar_id != INVALID_UNICHAR_ID) {
        /* Find lower case */
        if (!lower_done &&
            (getUnicharset().get_islower(unichar_id) ||
             (getUnicharset().get_isupper(unichar_id) && x == 0))) {
          lower_word.set_unichar_id(unichar_id, x);
          lower_word.set_rating(lower_word.rating() -
            first_choice->rating() + blob_choice_it.data()->rating());
          if (blob_choice_it.data()->certainty() < lower_word.certainty()) {
            lower_word.set_certainty(blob_choice_it.data()->certainty());
          }
          lower_certainties[x] = blob_choice_it.data()->certainty();
          lower_done = TRUE;
        }
        /* Find upper case */
        if (!upper_done && getUnicharset().get_isupper(unichar_id)) {
          capital_word.set_unichar_id(unichar_id, x);
          capital_word.set_rating(capital_word.rating() -
            first_choice->rating() + blob_choice_it.data()->rating());
          if (blob_choice_it.data()->certainty() < capital_word.certainty()) {
            capital_word.set_certainty(blob_choice_it.data()->certainty());
          }
          upper_certainties[x] = blob_choice_it.data()->certainty();
          upper_done = TRUE;
        }
        if (!char_alpha) {
          const CHAR_FRAGMENT *fragment =
            getUnicharset().get_fragment(unichar_id);
          temp_id = !fragment ? unichar_id :
            getUnicharset().unichar_to_id(fragment->get_unichar());
          if (getUnicharset().get_isalpha(temp_id)) {
            char_alpha = TRUE;
          }
        }
        if (lower_done && upper_done)
          break;
      }
    }
    if (char_alpha && any_alpha != NULL)
      *any_alpha = TRUE;

    if (word.rating() > *rating_limit)
      return (NULL);

    *prev_char = '\0';
    temp_id = word.unichar_id(word.length()-1);
    if (temp_id != INVALID_UNICHAR_ID) {
      strcpy(prev_char, getUnicharset().id_to_unichar(temp_id));
    }
  }

  if (word.rating() < raw_choice->rating()) {
    *raw_choice = word;
    LogNewRawChoice(*raw_choice, 1.0, certainties);
  }

  if (ngram_permuter_activated)
    return NULL;

  float rating = word.rating();
  word.string_and_lengths(getUnicharset(), &unichar_str, &unichar_lengths);
  adjust_non_word(unichar_str.string(), unichar_lengths.string(),
                  word.rating(), &new_rating, &adjust_factor);
  word.set_rating(new_rating);
  LogNewWordChoice(word, adjust_factor, certainties);

  float lower_rating = lower_word.rating();
  lower_word.string_and_lengths(getUnicharset(),
                                &unichar_str, &unichar_lengths);
  adjust_non_word(unichar_str.string(), unichar_lengths.string(),
                  lower_word.rating(), &new_rating, &adjust_factor);
  lower_word.set_rating(new_rating);
  LogNewWordChoice(lower_word, adjust_factor, lower_certainties);

  float upper_rating = capital_word.rating();
  capital_word.string_and_lengths(getUnicharset(),
                                  &unichar_str, &unichar_lengths);
  adjust_non_word(unichar_str.string(), unichar_lengths.string(),
                  capital_word.rating(), &new_rating, &adjust_factor);
  capital_word.set_rating(new_rating);
  LogNewWordChoice(capital_word, adjust_factor, upper_certainties);


  WERD_CHOICE *best_choice = &word;
  *rating_limit = rating;
  if (lower_word.rating() < best_choice->rating()) {
    best_choice = &lower_word;
    *rating_limit = lower_rating;
  }
  if (capital_word.rating() < best_choice->rating()) {
    best_choice = &capital_word;
    *rating_limit = upper_rating;
  }
  return new WERD_CHOICE(*best_choice);
}


/**********************************************************************
 * choose_il1
 *
 * Choose between the candidate il1 chars.
 **********************************************************************/
const char* Dict::choose_il1(const char *first_char,        //first choice
                             const char *second_char,       //second choice
                             const char *third_char,        //third choice
                             const char *prev_char,         //prev in word
                             const char *next_char,         //next in word
                             const char *next_next_char) {  //after next next in word
  inT32 type1;                   //1/I/l type of first choice
  inT32 type2;                   //1/I/l type of second choice
  inT32 type3;                   //1/I/l type of third choice

  int first_char_length = strlen(first_char);
  int prev_char_length = strlen(prev_char);
  int next_char_length = strlen(next_char);
  int next_next_char_length = strlen(next_next_char);

  if (*first_char == 'l' && *second_char != '\0') {
    if (*second_char == 'I'
        && (((prev_char_length != 0 &&
            getUnicharset().get_isupper (prev_char, prev_char_length)) &&
            (next_char_length == 0 ||
             !getUnicharset().get_islower (next_char, next_char_length)) &&
            (next_char_length == 0 ||
             !getUnicharset().get_isdigit (next_char, next_char_length))) ||
            ((next_char_length != 0 &&
             getUnicharset().get_isupper (next_char, next_char_length)) &&
            (prev_char_length == 0 ||
             !getUnicharset().get_islower (prev_char, prev_char_length)) &&
            (prev_char_length == 0 ||
             !getUnicharset().get_isdigit (prev_char, prev_char_length)))))
      first_char = second_char;  //override
    else if (*second_char == '1' || *third_char == '1') {
      if ((next_char_length != 0 &&
           getUnicharset().get_isdigit (next_char, next_char_length)) ||
          (prev_char_length != 0 &&
           getUnicharset().get_isdigit (prev_char, prev_char_length))
          || (*next_char == 'l' &&
          (next_next_char_length != 0 &&
           getUnicharset().get_isdigit (next_next_char,
                                        next_next_char_length)))) {
        first_char = "1";
        first_char_length = 1;
      }
      else if ((prev_char_length == 0 ||
                !getUnicharset().get_islower (prev_char, prev_char_length)) &&
               ((next_char_length == 0 ||
                 !getUnicharset().get_islower (next_char, next_char_length)) ||
                (*next_char == 's' &&
                *next_next_char == 't'))) {
        if (((*prev_char != '\'' && *prev_char != '`') || *next_char != '\0')
            && ((*next_char != '\'' && *next_char != '`')
                || *prev_char != '\0')) {
          first_char = "1";
          first_char_length = 1;
        }
      }
    }
    if (*first_char == 'l' && *next_char != '\0' &&
        (prev_char_length == 0 ||
         !getUnicharset().get_isalpha (prev_char, prev_char_length))) {
      type1 = 2;

      if (*second_char == '1')
        type2 = 0;
      else if (*second_char == 'I')
        type2 = 1;
      else if (*second_char == 'l')
        type2 = 2;
      else
        type2 = type1;

      if (*third_char == '1')
        type3 = 0;
      else if (*third_char == 'I')
        type3 = 1;
      else if (*third_char == 'l')
        type3 = 2;
      else
        type3 = type1;

#if 0
      if (bigram_counts[*next_char][type2] >
      bigram_counts[*next_char][type1]) {
        first_char = second_char;
        type1 = type2;
      }
      if (bigram_counts[*next_char][type3] >
      bigram_counts[*next_char][type1]) {
        first_char = third_char;
      }
#endif
    }
  }
  return first_char;
}

/**********************************************************************
 * permute_words
 *
 * Permute all the characters together using the dawg to prune all
 * but the valid words.
 **********************************************************************/
A_CHOICE *Dict::permute_words(CHOICES_LIST char_choices, float rating_limit) {
  A_CHOICE *best_choice;

  best_choice = new_choice (NULL, NULL, rating_limit, -MAX_FLOAT32, -1, NO_PERM);

  if (hyphen_base_size() + array_count (char_choices) > MAX_WERD_LENGTH) {
    class_rating (best_choice) = MAX_FLOAT32;
  }
  else {

    dawg_permute_and_select ("system words:", word_dawg, SYSTEM_DAWG_PERM,
      char_choices, best_choice, TRUE);

    dawg_permute_and_select ("document_words", document_words,
      DOC_DAWG_PERM, char_choices, best_choice,
      FALSE);

    dawg_permute_and_select ("user words", user_words, USER_DAWG_PERM,
      char_choices, best_choice, FALSE);
  }

  return (best_choice);
}


/**********************************************************************
 * valid_word
 *
 * Check all the DAWGs to see if this word is in any of them.
 **********************************************************************/
int Dict::valid_word(const char *string) {
  int result = NO_PERM;

  if (word_in_dawg (word_dawg, string))
    result = SYSTEM_DAWG_PERM;
  else {
    if (word_in_dawg (document_words, string))
      result = DOC_DAWG_PERM;
    else if (word_in_dawg (user_words, string))
      result = USER_DAWG_PERM;
  }
  return (result);
}

/**********************************************************************
 * fragment_state
 *
 * Given the current char choice and information about previously seen
 * fragments, determines whether adjacent character fragments are
 * present and whether they can be concatenated.
 *
 * The given prev_char_frag_info contains:
 *  -- fragment: if not NULL contains information about immediately
 *     preceeding fragmented character choice
 *  -- num_fragments: number of fragments that have been used so far
 *     to construct a character
 *  -- certainty: certainty of the current choice or minimum
 *     certainty of all fragments concatenated so far
 *  -- rating: rating of the current choice or sum of fragment
 *     ratings concatenated so far
 *
 * The output char_frag_info is filled in as follows:
 * -- character: is set to be NULL if the choice is a non-matching
 *    or non-ending fragment piece; is set to unichar of the given choice
 *    if it represents a regular character or a matching ending fragment
 * -- fragment,num_fragments,certainty,rating are set as described above
 *
 * Returns false if a non-matching fragment is discovered, true otherwise.
 **********************************************************************/
bool Dict::fragment_state_okay(UNICHAR_ID curr_unichar_id,
                               float curr_rating, float curr_certainty,
                               const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                               const char *debug, int word_ending,
                               CHAR_FRAGMENT_INFO *char_frag_info) {
  const CHAR_FRAGMENT *this_fragment =
    getUnicharset().get_fragment(curr_unichar_id);
  const CHAR_FRAGMENT *prev_fragment =
    prev_char_frag_info != NULL ? prev_char_frag_info->fragment : NULL;

  // Print debug info for fragments.
  if (debug && (prev_fragment || this_fragment)) {
    cprintf("%s check fragments: choice=%s word_ending=%d\n", debug,
            getUnicharset().debug_str(curr_unichar_id).string(),
            word_ending);
    if (prev_fragment) {
      cprintf("prev_fragment %s\n", prev_fragment->to_string().string());
    }
    if (this_fragment) {
      cprintf("this_fragment %s\n", this_fragment->to_string().string());
    }
  }

  char_frag_info->unichar_id = curr_unichar_id;
  char_frag_info->fragment = this_fragment;
  char_frag_info->rating = curr_rating;
  char_frag_info->certainty = curr_certainty;
  char_frag_info->num_fragments = 1;
  if (prev_fragment && !this_fragment) {
    if (debug) cprintf("Skip choice with incomplete fragment\n");
    return false;
  }
  if (this_fragment) {
    // We are dealing with a fragment.
    char_frag_info->unichar_id = INVALID_UNICHAR_ID;
    if (prev_fragment) {
      if (!this_fragment->is_continuation_of(prev_fragment)) {
        if (debug) cprintf("Non-matching fragment piece\n");
        return false;
      }
      if (this_fragment->is_ending()) {
        char_frag_info->unichar_id =
          getUnicharset().unichar_to_id(this_fragment->get_unichar());
        char_frag_info->fragment = NULL;
        if (debug) {
          cprintf("Built character %s from fragments\n",
                  getUnicharset().debug_str(
                      char_frag_info->unichar_id).string());
        }
      } else {
        if (debug) cprintf("Record fragment continuation\n");
        char_frag_info->fragment = this_fragment;
      }
      // Update certainty and rating.
      char_frag_info->rating =
        prev_char_frag_info->rating + curr_rating;
      char_frag_info->num_fragments = prev_char_frag_info->num_fragments + 1;
      char_frag_info->certainty =
        MIN(curr_certainty, prev_char_frag_info->certainty);
    } else {
      if (this_fragment->is_beginning()) {
        if (debug) cprintf("Record fragment beginning\n");
      } else {
        if (debug) {
          cprintf("Non-starting fragment piece with no prev_fragment\n");
        }
        return false;
      }
    }
  }
  if (word_ending && char_frag_info->fragment) {
    if (debug) cprintf("Word can not end with a fragment\n");
    return false;
  }
  return true;
}
/**********************************************************************
 * top_fragments_permute_and_select
 *
 * Creates a copy of character choices list that contain only fragments
 * and the best non-fragmented character choice.
 * Permutes character in this shortened list, builds characters from
 * fragments if possible and returns a better choice if found.
 *
 * TODO(daria): part of this code is similar to the code in permdag,
 * permngram and permnum - need to figure out a way to combine them.
 **********************************************************************/
WERD_CHOICE *Dict::top_fragments_permute_and_select(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    float rating_limit) {
  if (char_choices.length() <= 1 ||
      char_choices.length() > MAX_PERM_LENGTH) {
    return NULL;
  }
  // See it would be possible to benefit from permuting fragments.
  int x;
  float min_rating = 0.0;
  BLOB_CHOICE_IT blob_choice_it;
  for (x = 0; x < char_choices.length(); ++x) {
    blob_choice_it.set_to_list(char_choices.get(x));
    if (blob_choice_it.data()) {
      min_rating += blob_choice_it.data()->rating();
    }
    if (min_rating >= rating_limit) {
      return NULL;
    }
  }
  if (fragments_debug > 1) {
    tprintf("A choice with fragment beats top choice\n");
    tprintf("Running fragment permuter...\n");
  }

  // Construct a modified choices list that contains (for each position):
  // the best choice, all fragments and at least one choice for
  // a non-fragmented character.
  BLOB_CHOICE_LIST_VECTOR frag_char_choices(char_choices.length());
  for (x = 0; x < char_choices.length(); ++x) {
    bool need_nonfrag_char = true;
    BLOB_CHOICE_LIST *frag_choices = new BLOB_CHOICE_LIST();
    BLOB_CHOICE_IT frag_choices_it;
    frag_choices_it.set_to_list(frag_choices);
    blob_choice_it.set_to_list(char_choices.get(x));
    for (blob_choice_it.mark_cycle_pt(); !blob_choice_it.cycled_list();
         blob_choice_it.forward()) {
      if (getUnicharset().get_fragment(blob_choice_it.data()->unichar_id())) {
        frag_choices_it.add_after_then_move(
            new BLOB_CHOICE(*(blob_choice_it.data())));
      } else if (need_nonfrag_char) {
        frag_choices_it.add_after_then_move(
            new BLOB_CHOICE(*(blob_choice_it.data())));
        need_nonfrag_char = false;
      }
    }
    frag_char_choices += frag_choices;
  }

  WERD_CHOICE *best_choice = new WERD_CHOICE();
  best_choice->make_bad();
  WERD_CHOICE word(MAX_PERM_LENGTH);
  word.set_permuter(TOP_CHOICE_PERM);
  float certainties[MAX_PERM_LENGTH];
  top_fragments_permute(frag_char_choices, 0, min_rating, NULL,
                        &word, certainties, &rating_limit, best_choice);

  frag_char_choices.delete_data_pointers();
  return best_choice;
}

/**********************************************************************
 * top_fragments_permute
 *
 * Try to put together fragments that could have a better rating
 * than non-fragmented choices.
 *
 * TODO(daria): this code is very similar to the code in permdag,
 * permngram and permnum - need to figure out a way to combine them.
 **********************************************************************/
void Dict::top_fragments_permute(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    int char_choice_index,
    float min_rating,
    const CHAR_FRAGMENT_INFO *prev_char_frag_info,
    WERD_CHOICE *word,
    float certainties[],
    float *limit,
    WERD_CHOICE *best_choice) {
  if (fragments_debug > 1) {
    tprintf("top_fragments_permute: char_choice_index=%d"
            " limit=%4.2f rating=%4.f, certainty=%4.f word=%s\n",
            char_choice_index, *limit,
            word->rating(), word->certainty(),
            word->debug_string(getUnicharset()).string());
  }
  if (char_choice_index < char_choices.length()) {
    BLOB_CHOICE_IT blob_choice_it;
    blob_choice_it.set_to_list(char_choices.get(char_choice_index));
    for (blob_choice_it.mark_cycle_pt(); !blob_choice_it.cycled_list();
         blob_choice_it.forward()) {
      append_top_fragments_choices(char_choices, *(blob_choice_it.data()),
                                   char_choice_index, min_rating,
                                   prev_char_frag_info, word, certainties,
                                   limit, best_choice);

    }
  }
}

/**********************************************************************
 * append_top_fragments_choices
 *
 * Check to see whether or not the next choice is worth appending to
 * the string being generated.  If so then keep going deeper into the
 * word.
 *
 * TODO(daria): this code is very similar to the code in permdag,
 * permngram and permnum - need to figure out a way to combine them.
 **********************************************************************/
void Dict::append_top_fragments_choices(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    const BLOB_CHOICE &blob_choice,
    int char_choice_index,
    float min_rating,
    const CHAR_FRAGMENT_INFO *prev_char_frag_info,
    WERD_CHOICE *word,
    float certainties[],
    float *limit,
    WERD_CHOICE *best_choice) {
  int word_ending =
    (char_choice_index == char_choices.length() - 1) ? TRUE : FALSE;

  /* Deal with fragments */
  CHAR_FRAGMENT_INFO char_frag_info;
  if (!fragment_state_okay(blob_choice.unichar_id(), blob_choice.rating(),
                           blob_choice.certainty(), prev_char_frag_info,
                           (fragments_debug > 1) ? "fragments_debug" : NULL,
                           word_ending, &char_frag_info)) {
    return;  // blob_choice must be an invalid fragment
  }
  // Search the next letter if this character is a fragment.
  if (char_frag_info.unichar_id == INVALID_UNICHAR_ID) {
    top_fragments_permute(char_choices, char_choice_index + 1,
                          min_rating, &char_frag_info,
                          word, certainties, limit, best_choice);
    return;
  }

  /* Add new character */
  word->append_unichar_id_space_allocated(
      char_frag_info.unichar_id, char_frag_info.num_fragments,
      char_frag_info.rating, char_frag_info.certainty);
  certainties[word->length()-1] = char_frag_info.certainty;

  if (word->rating() < *limit) {
    if (word_ending) {
      if (fragments_debug > 1) {
        tprintf("new choice = %s\n",
                word->debug_string(getUnicharset()).string());
      }
      *limit = word->rating();

      // TODO(daria): modify this when adjust_non_word
      // is updated to use unichar ids.
      STRING word_str;
      STRING word_lengths_str;
      word->string_and_lengths(getUnicharset(), &word_str, &word_lengths_str);
      float new_rating;
      float adjust_factor;
      adjust_non_word(word_str.string(), word_lengths_str.string(),
                      word->rating(), &new_rating, &adjust_factor);
      word->set_rating(new_rating);
      LogNewWordChoice(*word, adjust_factor, certainties);

      if (word->rating() < best_choice->rating()) {
        *best_choice = *word;
      }
    } else {
      top_fragments_permute(char_choices, char_choice_index + 1,
                            min_rating, &char_frag_info, word,
                            certainties, limit, best_choice);
    }
  } else {
    if (fragments_debug > 1) {
      tprintf("pruned word (%s, rating=%4.2f, limit=%4.2f)\n",
              word->debug_string(getUnicharset()).string(),
              word->rating(), *limit);
    }
  }
}
}  // namespace tesseract
