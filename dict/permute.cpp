/* -*-C-*-
 ********************************************************************************
 *
 * File:        permute.c  (Formerly permute.c)
 * Description:  Handle the new ratings choices for Wise Owl
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
#include "permute.h"
#include "globals.h"
#include "permdawg.h"
#include "debug.h"
#include "tordvars.h"
#include "hyphen.h"
#include "stopper.h"
#include "trie.h"
#include "context.h"
#include "permnum.h"
#include "freelist.h"
#include "callcpp.h"
#include "permngram.h"

#include <math.h>

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

make_toggle_var (adjust_debug, 0, make_adjust_debug,
8, 13, set_adjust_debug, "Adjustment Debug");

make_toggle_var (compound_debug, 0, make_compound_debug,
8, 14, set_compound_debug, "Compound Debug");

make_float_var (non_word, NON_WERD, make_non_word,
8, 20, set_non_word, "Non-word adjustment");

make_float_var (garbage, GARBAGE_STRING, make_garbage,
8, 21, set_garbage, "Garbage adjustment");

make_toggle_var (save_doc_words, 0, make_doc_words,
8, 22, set_doc_words, "Save Document Words ");

make_toggle_var (doc_dict_enable, 1, make_doc_dict,
8, 25, set_doc_dict, "Enable Document Dictionary ");
/* PREV DEFAULT 0 */

BOOL_VAR(ngram_permuter_activated, FALSE,
         "Activate character-level n-gram-based permuter");

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
 * good_choice
 *
 * Return TRUE if a good answer is found for the unknown blob rating.
 **********************************************************************/
int good_choice(A_CHOICE *choice) {
  register float certainty;
  if (choice == NULL)
    return (FALSE);
  if (similarity_enable) {
    if ((class_probability (choice) + 1) * class_certainty (choice) >
      SIMILARITY_FLOOR)
      return (FALSE);
    certainty =
      SIM_CERTAINTY_OFFSET +
      class_probability (choice) * SIM_CERTAINTY_SCALE;
  }

  else {
    certainty = class_certainty (choice);
  }
  if (certainty > certainty_threshold) {
    return (TRUE);
  }

  else {
    return (FALSE);
  }
}


/**********************************************************************
 * add_document_word
 *
 * Add a word found on this document to the document specific
 * dictionary.
 **********************************************************************/
void add_document_word(A_CHOICE *best_choice) {
  char filename[CHARS_PER_LINE];
  FILE *doc_word_file;
  char *string;
  char *lengths;
  int stringlen;                 //length of word

  string = class_string (best_choice);
  lengths = class_lengths (best_choice);
  stringlen = strlen (lengths);

  // Skip if using external dictionary.
  if (letter_is_okay != &def_letter_is_okay) return;

  if (!doc_dict_enable
    || valid_word (string) || CurrentWordAmbig () || stringlen < 2)
    return;

  if (!good_choice (best_choice) || stringlen == 2) {
    if (class_certainty (best_choice) < permuter_pending_threshold)
      return;
    if (!word_in_dawg (pending_words, string)) {
      if (stringlen > 2 ||
          (stringlen >= 2 && unicharset.get_isupper (string, lengths[0]) &&
           unicharset.get_isupper (string + lengths[0], lengths[1])))
        add_word_to_dawg(pending_words,
                         string,
                         MAX_DOC_EDGES,
                         RESERVED_DOC_EDGES);
      return;
    }
  }

  if (save_doc_words) {
    strcpy(filename, imagefile);
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
void
adjust_non_word (A_CHOICE * best_choice, float certainties[]) {
  char *this_word;
  float adjust_factor;

  if (adjust_debug)
    cprintf ("%s %4.2f ",
      class_string (best_choice), class_probability (best_choice));

  this_word = class_string (best_choice);

  class_probability (best_choice) += RATING_PAD;
  if (case_ok (this_word, class_lengths (best_choice))
      && punctuation_ok (this_word, class_lengths (best_choice)) != -1) {
    class_probability (best_choice) *= non_word;
    adjust_factor = non_word;
    if (adjust_debug)
      cprintf (", %4.2f ", non_word);
  }
  else {
    class_probability (best_choice) *= garbage;
    adjust_factor = garbage;
    if (adjust_debug) {
      if (!case_ok (this_word, class_lengths (best_choice)))
        cprintf (", C");
      if (punctuation_ok (this_word, class_lengths (best_choice)) == -1)
        cprintf (", P");
      cprintf (", %4.2f ", garbage);
    }
  }

  class_probability (best_choice) -= RATING_PAD;

  LogNewWordChoice(best_choice, adjust_factor, certainties);

  if (adjust_debug)
    cprintf (" --> %4.2f\n", class_probability (best_choice));
}


/**********************************************************************
 * init_permute
 *
 * Initialize anything that needs to be set up for the permute
 * functions.
 **********************************************************************/
void init_permute_vars() {
  make_adjust_debug();
  make_compound_debug();
  make_non_word();
  make_garbage();
  make_doc_words();
  make_doc_dict();

  init_permdawg_vars();
  init_permnum();
}

void init_permute() {
  if (word_dawg != NULL)
    end_permute();
  init_permdawg();
  STRING name;
  name = language_data_path_prefix;
  name += "word-dawg";
  word_dawg = read_squished_dawg(name.string());

  document_words =
    (EDGE_ARRAY) memalloc (sizeof (EDGE_RECORD) * MAX_DOC_EDGES);
  initialize_dawg(document_words, MAX_DOC_EDGES);

  pending_words =
    (EDGE_ARRAY) memalloc (sizeof (EDGE_RECORD) * MAX_DOC_EDGES);
  initialize_dawg(pending_words, MAX_DOC_EDGES);

  user_words = (EDGE_ARRAY) memalloc (sizeof (EDGE_RECORD) * MAX_USER_EDGES);
  name = language_data_path_prefix;
  name += "user-words";
  read_word_list(name.string(), user_words, MAX_USER_EDGES, USER_RESERVED_EDGES);
}

void end_permute() {
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
 * a non-NIL choice list.
 **********************************************************************/
A_CHOICE *permute_all(CHOICES_LIST char_choices,
                      float rating_limit,
                      A_CHOICE *raw_choice) {
  A_CHOICE *result_1;
  A_CHOICE *result_2 = NULL;
  BOOL8 any_alpha;

  result_1 = permute_top_choice (char_choices, rating_limit, raw_choice,
    &any_alpha);

  if (ngram_permuter_activated)
    return ngram_permute_and_select(char_choices, rating_limit, word_dawg);

  if (result_1 == NULL)
    return (NULL);
  if (permute_only_top)
    return result_1;
  if (any_alpha && array_count (char_choices) <= MAX_WERD_LENGTH) {
    result_2 = permute_words (char_choices, rating_limit);
    if (class_probability (result_1) < class_probability (result_2)
    || class_string (result_2) == NULL) {
      free_choice(result_2);
    }
    else {
      free_choice(result_1);
      result_1 = result_2;
    }
  }

  result_2 = number_permute_and_select (char_choices, rating_limit);

  if (class_probability (result_1) < class_probability (result_2)
  || class_string (result_2) == NULL) {
    free_choice(result_2);
  }
  else {
    free_choice(result_1);
    result_1 = result_2;
  }

  result_2 = permute_compound_words (char_choices, rating_limit);

  if (!result_2 ||
    class_probability (result_1) < class_probability (result_2)
  || class_string (result_2) == NULL) {
    free_choice(result_2);
  }
  else {
    free_choice(result_1);
    result_1 = result_2;
  }

  return (result_1);
}


/**********************************************************************
 * permute_characters
 *
 * Permute these characters together according to each of the different
 * permuters that are enabled.
 **********************************************************************/
void permute_characters(CHOICES_LIST char_choices,
                        float limit,
                        A_CHOICE *best_choice,
                        A_CHOICE *raw_choice) {
  A_CHOICE *this_choice;

  permutation_count++;           /* Global counter */

  this_choice = permute_all (char_choices, limit, raw_choice);

  if (this_choice &&
  class_probability (this_choice) < class_probability (best_choice)) {
    clone_choice(best_choice, this_choice);
  }
  free_choice(this_choice);

  if (display_ratings)
    print_word_choice("permute_characters", best_choice);
}


/**********************************************************************
 * permute_compound_word
 *
 * Return the top choice for each character as the choice for the word.
 **********************************************************************/
A_CHOICE *permute_compound_words(CHOICES_LIST character_choices,
                                 float rating_limit) {
  A_CHOICE *first_choice;
  A_CHOICE *best_choice = NULL;
  char word[UNICHAR_LEN * MAX_WERD_LENGTH + 1];
  char unichar_lengths[MAX_WERD_LENGTH + 1];
  float rating = 0;
  float certainty = 10000;
  char char_choice;
  int x;
  int first_index = 0;
  char *ptr;

  word[0] = '\0';
  unichar_lengths[0] = 0;

  if (array_count (character_choices) > MAX_WERD_LENGTH) {
    return (new_choice (NULL, NULL, MAX_FLOAT32, -MAX_FLOAT32, -1, NO_PERM));
  }

  array_loop(character_choices, x) {

    first_choice =
      (A_CHOICE *) first_node ((CHOICES) array_value (character_choices, x));

    ptr = class_string (first_choice);
    char_choice = ptr != NULL ? *ptr : '\0';
    if (x > first_index && (char_choice == '-' || char_choice == '/')) {
      if (compound_debug)
        cprintf ("Hyphenated word found\n");

      permute_subword (character_choices, rating_limit,
        first_index, x - 1, word, unichar_lengths,
                       &rating, &certainty);

      if (rating > rating_limit)
        break;
      first_index = x + 1;

      strcat(word, class_string (first_choice));
      char length[] = {strlen(class_string (first_choice)), 0};
      strcat(unichar_lengths + x, length);
      rating += class_probability (first_choice);
      certainty = min (class_certainty (first_choice), certainty);
    }
  }

  if (first_index > 0 && first_index < x && rating <= rating_limit) {
    permute_subword (character_choices, rating_limit,
                     first_index, x - 1, word, unichar_lengths,
                     &rating, &certainty);

    best_choice = new_choice (word, unichar_lengths, rating,
                              certainty, -1, COMPOUND_PERM);
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
void permute_subword(CHOICES_LIST character_choices,
                     float rating_limit,
                     int start,
                     int end,
                     char *word,
                     char unichar_lengths[],
                     float *rating,
                     float *certainty) {
  int x;
  A_CHOICE *best_choice = NULL;
  A_CHOICE raw_choice;
  CHOICES_LIST subchoices;
  CHOICES choices;
  char this_char;
  char *ptr;

  DisableChoiceAccum();
  raw_choice.string = NULL;
  raw_choice.lengths = NULL;
  raw_choice.rating = MAX_INT16;
  raw_choice.certainty = -MAX_INT16;

  subchoices = new_choice_list ();
  for (x = start; x <= end; x++) {
    choices = (CHOICES) array_value (character_choices, x);
    ptr = best_string (choices);
    this_char = ptr != NULL ? *ptr : '\0';
    if (this_char != '-' && this_char != '/') {
      subchoices = array_push (subchoices, choices);
    } else {
      const char* str = best_string(choices);
      strcat(word, str);
      char length[] = {strlen(str), 0};
      strcat(unichar_lengths + x, length);
    }
  }

  if (array_count (subchoices)) {
    if (compound_debug)
      dawg_debug = TRUE;
    best_choice = permute_all (subchoices, rating_limit, &raw_choice);
    if (compound_debug)
      dawg_debug = FALSE;

    if (best_choice && class_string (best_choice)) {
      strcat (word, class_string (best_choice));
      strcat (unichar_lengths, class_lengths (best_choice));
      *rating += class_probability (best_choice);
      *certainty = min (class_certainty (best_choice), *certainty);
    }
    else {
      *rating = MAX_FLOAT32;
    }
  }
  else {
    *rating = MAX_FLOAT32;
  }

  free_choice_list(subchoices);
  if (best_choice)
    free_choice(best_choice);

  if (compound_debug && *rating < MAX_FLOAT32) {
    cprintf ("Subword permuted = %s, %5.2f, %5.2f\n\n",
      word, *rating, *certainty);
  }
  if (raw_choice.string)
    strfree(raw_choice.string);
  if (raw_choice.lengths)
    strfree(raw_choice.lengths);

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
A_CHOICE *permute_top_choice(CHOICES_LIST character_choices,
                             float rating_limit,
                             A_CHOICE *raw_choice,
                             BOOL8 *any_alpha) {
  CHOICES char_list;
  A_CHOICE *first_choice;
  A_CHOICE *best_choice;
  A_CHOICE *other_choice;
  const char *ptr;
  const char *first_char;             //first choice
  const char *second_char;            //second choice
  const char *third_char;             //third choice
  char prev_char[UNICHAR_LEN + 1];    //prev in word
  const char *next_char = "";         //next in word
  const char *next_next_char = "";    //after next next in word

  char word[UNICHAR_LEN * MAX_PERM_LENGTH + 1];
  char capital_word[UNICHAR_LEN * MAX_PERM_LENGTH + 1];
  char lower_word[UNICHAR_LEN * MAX_PERM_LENGTH + 1];

  char word_lengths[MAX_PERM_LENGTH + 1];
  char capital_word_lengths[MAX_PERM_LENGTH + 1];
  char lower_word_lengths[MAX_PERM_LENGTH + 1];

  int x;
  int x_word = 0;
  int x_capital_word = 0;
  int x_lower_word = 0;
  BOOL8 char_alpha;

  float rating = 0;
  float upper_rating = 0;
  float lower_rating = 0;
  float first_rating = 0;

  float certainty = 10000;
  float upper_certainty = 10000;
  float lower_certainty = 10000;

  float certainties[MAX_PERM_LENGTH + 1];
  float lower_certainties[MAX_PERM_LENGTH + 1];
  float upper_certainties[MAX_PERM_LENGTH + 1];

  register CHOICES this_char;
  register const char* ch;
  register inT8 lower_done;
  register inT8 upper_done;

  prev_char[0] = '\0';

  if (any_alpha != NULL)
    *any_alpha = FALSE;

  if (array_count (character_choices) > MAX_PERM_LENGTH) {
    return (NULL);
  }

  array_loop(character_choices, x) {
    if (x + 1 < array_count (character_choices)) {
      char_list = (CHOICES) array_value (character_choices, x + 1);
      first_choice = (A_CHOICE *) first_node (char_list);

      ptr = class_string (first_choice);
      next_char = (ptr != NULL && *ptr != '\0') ? ptr : " ";
    }
    else
      next_char = "";
    if (x + 2 < array_count (character_choices)) {
      char_list = (CHOICES) array_value (character_choices, x + 2);
      first_choice = (A_CHOICE *) first_node (char_list);

      ptr = class_string (first_choice);
      next_next_char = (ptr != NULL && *ptr != '\0') ? ptr : " ";
    }
    else
      next_next_char = "";

    char_list = (CHOICES) array_value (character_choices, x);
    first_choice = (A_CHOICE *) first_node (char_list);

    ptr = class_string (first_choice);
    if (ptr != NULL && *ptr != '\0')
    {
      strcpy(word + x_word, ptr);
      word_lengths[x] = strlen(ptr);

      strcpy(capital_word + x_capital_word, ptr);
      capital_word_lengths[x] = strlen(ptr);

      strcpy(lower_word + x_lower_word, ptr);
      lower_word_lengths[x] = strlen(ptr);
    }
    else
    {
      word[x_word] = ' ';
      word_lengths[x] = 1;

      capital_word[x_capital_word] = ' ';
      capital_word_lengths[x] = 1;

      lower_word[x_lower_word] = ' ';
      lower_word_lengths[x] = 1;
    }

    first_char = (ptr != NULL && *ptr != '\0') ? ptr : " ";
    first_rating = class_probability (first_choice);
    upper_rating += class_probability (first_choice);
    lower_rating += class_probability (first_choice);
    lower_certainty = min (class_certainty (first_choice), lower_certainty);
    upper_certainty = min (class_certainty (first_choice), upper_certainty);

    certainties[x] = class_certainty (first_choice);
    lower_certainties[x] = class_certainty (first_choice);
    upper_certainties[x] = class_certainty (first_choice);

    lower_done = FALSE;
    upper_done = FALSE;
    char_alpha = FALSE;
    second_char = "";
    third_char = "";
    iterate_list(this_char, char_list) {
      ptr = best_string (this_char);
      ch = ptr != NULL ? ptr : "";
      if (strcmp(ch, "l") == 0 && rest (this_char) != NULL
      && best_probability (rest (this_char)) == first_rating) {
        ptr = best_string (rest (this_char));
        if (ptr != NULL && (strcmp(ptr, "1") == 0 || strcmp(ptr, "I") == 0)) {
          second_char = ptr;
          this_char = rest (this_char);
          if (rest (this_char) != NULL
          && best_probability (rest (this_char)) == first_rating) {
            ptr = best_string (rest (this_char));
            if (ptr != NULL && (strcmp(ptr, "1") == 0 || strcmp(ptr, "I") == 0)) {
              third_char = ptr;
              this_char = rest (this_char);
            }
          }
          ch = choose_il1 (first_char, second_char, third_char,
            prev_char, next_char, next_next_char);
          if (strcmp(ch, "l") != 0 && word_lengths[x] == 1 &&
              word[x_word] == 'l') {
            word[x_word] = *ch;
            lower_word[x_lower_word] = *ch;
            capital_word[x_capital_word] = *ch;
          }
        }
      }
      if (ch != NULL && *ch != '\0') {
        /* Find lower case */
        if (!lower_done && (unicharset.get_islower(ch) ||
                            (unicharset.get_isupper(ch) && x == 0))) {
          strcpy(lower_word + x_lower_word, ch);
          lower_word_lengths[x] = strlen(ch);
          lower_rating += best_probability (this_char);
          lower_rating -= class_probability (first_choice);
          lower_certainty = min (best_certainty (this_char), lower_certainty);
          lower_certainties[x] = best_certainty (this_char);
          lower_done = TRUE;
        }
        /* Find upper case */
        if (!upper_done && unicharset.get_isupper(ch)) {
          strcpy(capital_word + x_capital_word, ch);
          capital_word_lengths[x] = strlen(ch);
          upper_rating += best_probability (this_char);
          upper_rating -= class_probability (first_choice);
          upper_certainty = min (best_certainty (this_char), upper_certainty);
          upper_certainties[x] = best_certainty (this_char);
          upper_done = TRUE;
        }
        if (!char_alpha && unicharset.get_isalpha(ch))
          char_alpha = TRUE;
        if (lower_done && upper_done)
          break;
      }
    }
    if (char_alpha && any_alpha != NULL)
      *any_alpha = TRUE;

    if (first_choice == NULL) {
      cprintf ("Permuter giving up due to null choices list");
      word[x_word + 1] = '$';
      word[x_word + 2] = '\0';
      word_lengths[x + 1] = 1;
      word_lengths[x + 2] = 0;
      cprintf (" word=%s\n", word);
      return (NULL);
    }

    rating += class_probability (first_choice);
    if (rating > rating_limit)
      return (NULL);

    certainty = min (class_certainty (first_choice), certainty);

    strncpy(prev_char, word + x_word, word_lengths[x]);
    prev_char[word_lengths[x]] = '\0';

    x_word += word_lengths[x];
    x_capital_word += capital_word_lengths[x];
    x_lower_word += lower_word_lengths[x];
  }

  word[x_word] = '\0';
  word_lengths[x] = 0;

  capital_word[x_capital_word] = '\0';
  capital_word_lengths[x] = 0;

  lower_word[x_lower_word] = '\0';
  lower_word_lengths[x] = 0;

  if (rating < class_probability (raw_choice)) {
    if (class_string (raw_choice))
      strfree (class_string (raw_choice));
    if (class_lengths (raw_choice))
      strfree (class_lengths (raw_choice));

    class_probability (raw_choice) = rating;
    class_certainty (raw_choice) = certainty;
    class_string (raw_choice) = strsave (word);
    class_lengths (raw_choice) = strsave (word_lengths);
    class_permuter (raw_choice) = TOP_CHOICE_PERM;

    LogNewRawChoice (raw_choice, 1.0, certainties);
  }

  if (ngram_permuter_activated)
    return NULL;

  best_choice = new_choice (word, word_lengths,
                            rating, certainty, -1, TOP_CHOICE_PERM);
  adjust_non_word(best_choice, certainties);

  other_choice = new_choice (lower_word, lower_word_lengths,
                             lower_rating, lower_certainty,
                             -1, LOWER_CASE_PERM);
  adjust_non_word(other_choice, lower_certainties);
  if (class_probability (best_choice) > class_probability (other_choice)) {
    clone_choice(best_choice, other_choice);
  }
  free_choice(other_choice);

  other_choice = new_choice (capital_word, capital_word_lengths,
                             upper_rating, upper_certainty,
                             -1, UPPER_CASE_PERM);
  adjust_non_word(other_choice, upper_certainties);
  if (class_probability (best_choice) > class_probability (other_choice)) {
    clone_choice(best_choice, other_choice);
  }
  free_choice(other_choice);
  return (best_choice);
}


/**********************************************************************
 * choose_il1
 *
 * Choose between the candidate il1 chars.
 **********************************************************************/
const char* choose_il1(const char *first_char,        //first choice
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
            unicharset.get_isupper (prev_char, prev_char_length)) &&
            (next_char_length == 0 ||
             !unicharset.get_islower (next_char, next_char_length)) &&
            (next_char_length == 0 ||
             !unicharset.get_isdigit (next_char, next_char_length))) ||
            ((next_char_length != 0 &&
             unicharset.get_isupper (next_char, next_char_length)) &&
            (prev_char_length == 0 ||
             !unicharset.get_islower (prev_char, prev_char_length)) &&
            (prev_char_length == 0 ||
             !unicharset.get_isdigit (prev_char, prev_char_length)))))
      first_char = second_char;  //override
    else if (*second_char == '1' || *third_char == '1') {
      if ((next_char_length != 0 &&
           unicharset.get_isdigit (next_char, next_char_length)) ||
          (prev_char_length != 0 &&
           unicharset.get_isdigit (prev_char, prev_char_length))
          || (*next_char == 'l' &&
          (next_next_char_length != 0 &&
           unicharset.get_isdigit (next_next_char, next_next_char_length)))) {
        first_char = "1";
        first_char_length = 1;
      }
      else if ((prev_char_length == 0 ||
                !unicharset.get_islower (prev_char, prev_char_length)) &&
               ((next_char_length == 0 ||
                 !unicharset.get_islower (next_char, next_char_length)) ||
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
         !unicharset.get_isalpha (prev_char, prev_char_length))) {
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
A_CHOICE *permute_words(CHOICES_LIST char_choices, float rating_limit) {
  A_CHOICE *best_choice;

  best_choice = new_choice (NULL, NULL, rating_limit, -MAX_FLOAT32, -1, NO_PERM);

  if (hyphen_base_size() + array_count (char_choices) > MAX_WERD_LENGTH) {
    class_probability (best_choice) = MAX_FLOAT32;
  }
  else {

    dawg_permute_and_select ("system words:", word_dawg, SYSTEM_DAWG_PERM,
      char_choices, best_choice);

    dawg_permute_and_select ("document_words", document_words,
      DOC_DAWG_PERM, char_choices, best_choice);

    dawg_permute_and_select ("user words", user_words, USER_DAWG_PERM,
      char_choices, best_choice);
  }

  return (best_choice);
}


/**********************************************************************
 * valid_word
 *
 * Check all the DAWGs to see if this word is in any of them.
 **********************************************************************/
int valid_word(const char *string) {
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

