/******************************************************************
 * File:        output.cpp  (Formerly output.c)
 * Description: Output pass
 * Author:					Phil Cheatle
 * Created:					Thu Aug  4 10:56:08 BST 1994
 *
 * (C) Copyright 1994, Hewlett-Packard Ltd.
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "mfcpch.h"
#include          "ocrshell.h"
#include          <string.h>
#include          <ctype.h>
#ifdef __UNIX__
#include          <assert.h>
#include          <unistd.h>
#include                    <errno.h>
#endif
#include          "mainblk.h"
#include          "tfacep.h"
#include          "tessvars.h"
#include          "control.h"
#include          "secname.h"
#include          "reject.h"
#include          "docqual.h"
#include          "output.h"
#include "bestfirst.h"
#include "globals.h"
#include "tesseractclass.h"

#define EXTERN

#define EPAPER_EXT      ".ep"
#define PAGE_YSIZE      3508
#define CTRL_INSET      '\024'   //dc4=text inset
#define CTRL_FONT       '\016'   //so=font change
#define CTRL_DEFAULT      '\017' //si=default font
#define CTRL_SHIFT      '\022'   //dc2=x shift
#define CTRL_TAB        '\011'   //tab
#define CTRL_NEWLINE      '\012' //newline
#define CTRL_HARDLINE   '\015'   //cr

EXTERN BOOL_EVAR (tessedit_write_block_separators, FALSE,
"Write block separators in output");
EXTERN BOOL_VAR (tessedit_write_raw_output, FALSE,
"Write raw stuff to name.raw");
EXTERN BOOL_EVAR (tessedit_write_output, FALSE, "Write text to name.txt");
EXTERN BOOL_EVAR (tessedit_write_ratings, FALSE,
"Return ratings in IPEOCRAPI data");
EXTERN BOOL_EVAR (tessedit_write_txt_map, FALSE,
"Write .txt to .etx map file");
EXTERN BOOL_EVAR (tessedit_write_rep_codes, FALSE,
"Write repetition char code");
EXTERN BOOL_EVAR (tessedit_write_unlv, FALSE, "Write .unlv output file");
EXTERN STRING_EVAR (unrecognised_char, "|",
"Output char for unidentified blobs");
EXTERN INT_EVAR (suspect_level, 99, "Suspect marker level");
EXTERN INT_VAR (suspect_space_level, 100,
"Min suspect level for rejecting spaces");
EXTERN INT_VAR (suspect_short_words, 2,
"Dont Suspect dict wds longer than this");
EXTERN BOOL_VAR (suspect_constrain_1Il, FALSE,
"UNLV keep 1Il chars rejected");
EXTERN double_VAR (suspect_rating_per_ch, 999.9,
"Dont touch bad rating limit");
EXTERN double_VAR (suspect_accept_rating, -999.9, "Accept good rating limit");

EXTERN BOOL_EVAR (tessedit_minimal_rejection, FALSE,
"Only reject tess failures");
EXTERN BOOL_VAR (tessedit_zero_rejection, FALSE, "Dont reject ANYTHING");
EXTERN BOOL_VAR (tessedit_word_for_word, FALSE,
"Make output have exactly one word per WERD");
EXTERN BOOL_VAR (tessedit_zero_kelvin_rejection, FALSE,
"Dont reject ANYTHING AT ALL");
EXTERN BOOL_VAR (tessedit_consistent_reps, TRUE,
"Force all rep chars the same");

FILE *txt_mapfile = NULL;        //reject map
FILE *unlv_file = NULL;          //reject map

/**********************************************************************
 * pixels_to_pts
 *
 * Convert an integer number of pixels to the nearest integer
 * number of points.
 **********************************************************************/

inT32 pixels_to_pts(               //convert coords
                    inT32 pixels,
                    inT32 pix_res  //resolution
                   ) {
  float pts;                     //converted value

  pts = pixels * 72.0 / pix_res;
  return (inT32) (pts + 0.5);    //round it
}

namespace tesseract {
void Tesseract::output_pass(  //Tess output pass //send to api
                            PAGE_RES_IT &page_res_it,
                            BOOL8 write_to_shm,
                            TBOX *target_word_box) {
  BLOCK_RES *block_of_last_word;
  inT16 block_id;
  BOOL8 force_eol;               //During output
  BLOCK *nextblock;              //block of next word
  WERD *nextword;                //next word

  if (tessedit_write_txt_map)
    txt_mapfile = open_outfile (".map");

  page_res_it.restart_page ();
  block_of_last_word = NULL;
  while (page_res_it.word () != NULL) {
    check_debug_pt (page_res_it.word (), 120);

	if (target_word_box)
	{

		TBOX current_word_box=page_res_it.word ()->word->bounding_box();
		FCOORD center_pt((current_word_box.right()+current_word_box.left())/2,(current_word_box.bottom()+current_word_box.top())/2);
		if (!target_word_box->contains(center_pt))
		{
			page_res_it.forward ();
			continue;
		}

	}
    if (tessedit_write_block_separators &&
    block_of_last_word != page_res_it.block ()) {
      block_of_last_word = page_res_it.block ();
      block_id = block_of_last_word->block->index();
      if (!wordrec_no_block)
        fprintf (textfile, "|^~tr%d\n", block_id);
      fprintf (txt_mapfile, "|^~tr%d\n", block_id);
    }

    force_eol = (tessedit_write_block_separators &&
      (page_res_it.block () != page_res_it.next_block ())) ||
      (page_res_it.next_word () == NULL);

    if (page_res_it.next_word () != NULL)
      nextword = page_res_it.next_word ()->word;
    else
      nextword = NULL;
    if (page_res_it.next_block () != NULL)
      nextblock = page_res_it.next_block ()->block;
    else
      nextblock = NULL;
                                 //regardless of tilde crunching
    write_results (page_res_it, determine_newline_type (page_res_it.word ()->word, page_res_it.block ()->block, nextword, nextblock), force_eol,
      write_to_shm);
    page_res_it.forward ();
  }
  if (write_to_shm)
    ocr_send_text(FALSE);
  if (tessedit_write_block_separators) {
    if (!wordrec_no_block)
      fprintf (textfile, "|^~tr\n");
    fprintf (txt_mapfile, "|^~tr\n");
  }
  if (tessedit_write_txt_map) {
    fprintf (txt_mapfile, "\n"); //because txt gets one
    #ifdef __UNIX__
    fsync (fileno (txt_mapfile));
    #endif
    fclose(txt_mapfile);
  }
}


/*************************************************************************
 * write_results()
 *
 * All recognition and rejection has now been done. Generate the following:
 *   .txt file     - giving the final best choices with NO highlighting
 *   .raw file     - giving the tesseract top choice output for each word
 *   .map file     - showing how the .txt file has been rejected in the .ep file
 *   epchoice list - a list of one element per word, containing the text for the
 *                   epaper. Reject strings are inserted.
 *   inset list    - a list of bounding boxes of reject insets - indexed by the
 *                   reject strings in the epchoice text.
 *************************************************************************/

void Tesseract::write_results(                        //output a word
                                                      //full info
                              PAGE_RES_IT &page_res_it,
                              char newline_type,      //type of newline
                                                      //override tilde crunch?
                              BOOL8 force_eol,
                              BOOL8 write_to_shm      //send to api
                  ) {
                                 //word to do
  WERD_RES *word = page_res_it.word ();
//   WERD_CHOICE *ep_choice;        //ep format
  STRING repetition_code;
  const STRING *wordstr;
  STRING wordstr_lengths;
  int i;
  char unrecognised = STRING (unrecognised_char)[0];
  char ep_chars[32];             //Only for unlv_tilde_crunch
  int ep_chars_index = 0;
  char txt_chs[32];              //Only for unlv_tilde_crunch
  char map_chs[32];              //Only for unlv_tilde_crunch
  int txt_index = 0;
  static BOOL8 tilde_crunch_written = FALSE;
  static BOOL8 last_char_was_newline = TRUE;
  static BOOL8 last_char_was_tilde = FALSE;
  static BOOL8 empty_block = TRUE;
  BOOL8 need_reject = FALSE;
  PBLOB_IT blob_it;              //blobs
  UNICHAR_ID space = unicharset.unichar_to_id(" ");

  /*	if (word->best_choice->string().length() == 0)
    {
      tprintf("No output: to output\n");
    }
    else if (word->best_choice->string()[0]==' ')
    {
      tprintf("spaceword to output\n");
    }
    else if (word->best_choice->string()[0]=='\0')
    {
      tprintf("null to output\n");
    }*/
  if (word->unlv_crunch_mode != CR_NONE
  && !tessedit_zero_kelvin_rejection && !tessedit_word_for_word) {
    if ((word->unlv_crunch_mode != CR_DELETE) &&
      (!tilde_crunch_written ||
      ((word->unlv_crunch_mode == CR_KEEP_SPACE) &&
      (word->word->space () > 0) &&
      !word->word->flag (W_FUZZY_NON) &&
    !word->word->flag (W_FUZZY_SP)))) {
      if (!word->word->flag (W_BOL) &&
        (word->word->space () > 0) &&
        !word->word->flag (W_FUZZY_NON) &&
      !word->word->flag (W_FUZZY_SP)) {
        /* Write a space to separate from preceeding good text */
        txt_chs[txt_index] = ' ';
        map_chs[txt_index++] = '1';
        ep_chars[ep_chars_index++] = ' ';
        last_char_was_tilde = FALSE;
      }
      need_reject = TRUE;
    }
    if ((need_reject && !last_char_was_tilde) || (force_eol && empty_block)) {
      /* Write a reject char - mark as rejected unless zero_rejection mode */
      last_char_was_tilde = TRUE;
      txt_chs[txt_index] = unrecognised;
      if (tessedit_zero_rejection || (suspect_level == 0)) {
        map_chs[txt_index++] = '1';
        ep_chars[ep_chars_index++] = unrecognised;
      }
      else {
        map_chs[txt_index++] = '0';
        /*
           The ep_choice string is a faked reject to allow newdiff to sync the
           .etx with the .txt and .map files.
         */
        ep_chars[ep_chars_index++] = CTRL_INSET;
        //escape code
                                 //dummy reject
        ep_chars[ep_chars_index++] = 1;
                                 //dummy reject
        ep_chars[ep_chars_index++] = 1;
                                 //type
        ep_chars[ep_chars_index++] = 2;
                                 //dummy reject
        ep_chars[ep_chars_index++] = 1;
                                 //dummy reject
        ep_chars[ep_chars_index++] = 1;
      }
      tilde_crunch_written = TRUE;
      last_char_was_newline = FALSE;
      empty_block = FALSE;
    }

    if ((word->word->flag (W_EOL) && !last_char_was_newline) || force_eol) {
      /* Add a new line output */
      txt_chs[txt_index] = '\n';
      map_chs[txt_index++] = '\n';
                                 //end line
      ep_chars[ep_chars_index++] = newline_type;

                                 //Cos of the real newline
      tilde_crunch_written = FALSE;
      last_char_was_newline = TRUE;
      last_char_was_tilde = FALSE;
    }
    txt_chs[txt_index] = '\0';
    map_chs[txt_index] = '\0';
                                 //xiaofan
    if (tessedit_write_output && !wordrec_no_block)
      fprintf (textfile, "%s", txt_chs);

    if (tessedit_write_txt_map)
      fprintf (txt_mapfile, "%s", map_chs);

                                 //terminate string
    ep_chars[ep_chars_index] = '\0';
    word->ep_choice = new WERD_CHOICE(ep_chars, unicharset);

    if (force_eol)
      empty_block = TRUE;
    return;
  }

  /* NORMAL PROCESSING of non tilde crunched words */

  tilde_crunch_written = FALSE;
  if (newline_type)
    last_char_was_newline = TRUE;
  else
    last_char_was_newline = FALSE;
  empty_block = force_eol;       //About to write a real word

  if (unlv_tilde_crunching &&
      last_char_was_tilde &&
      (word->word->space() == 0) &&
      !(word->word->flag(W_REP_CHAR) && tessedit_write_rep_codes) &&
      (word->best_choice->unichar_id(0) == space)) {
    /* Prevent adjacent tilde across words - we know that adjacent tildes within
       words have been removed */
    word->best_choice->remove_unichar_id(0);
    word->best_choice->populate_unichars(getDict().getUnicharset());
    word->reject_map.remove_pos (0);
    blob_it = word->outword->blob_list ();
    delete blob_it.extract ();   //get rid of reject blob
  }
  if (newline_type ||
    (word->word->flag (W_REP_CHAR) && tessedit_write_rep_codes))
    last_char_was_tilde = FALSE;
  else {
    if (word->reject_map.length () > 0) {
      if (word->best_choice->unichar_id(word->reject_map.length() - 1) == space)
        last_char_was_tilde = TRUE;
      else
        last_char_was_tilde = FALSE;
    }
    else if (word->word->space () > 0)
      last_char_was_tilde = FALSE;
    /* else it is unchanged as there are no output chars */
  }

  ASSERT_HOST (word->best_choice->length() == word->reject_map.length());

  if (word->word->flag (W_REP_CHAR) && tessedit_consistent_reps)
    ensure_rep_chars_are_consistent(word);

  set_unlv_suspects(word);
  check_debug_pt (word, 120);
  if (tessedit_rejection_debug) {
    tprintf ("Dict word: \"%s\": %d\n",
             word->best_choice->debug_string(unicharset).string(),
             dict_word(*(word->best_choice)));
  }

#if 0
  if (tessedit_write_unlv) {
    write_unlv_text(word);
  }
#endif

  if (word->word->flag (W_REP_CHAR) && tessedit_write_rep_codes) {
    repetition_code = "|^~R";
    wordstr_lengths = "\001\001\001\001";
    repetition_code += unicharset.id_to_unichar(get_rep_char (word));
    wordstr_lengths += strlen(unicharset.id_to_unichar(get_rep_char (word)));
    wordstr = &repetition_code;
  }
  else {
    if (tessedit_zero_rejection) {
      /* OVERRIDE ALL REJECTION MECHANISMS - ONLY REJECT TESS FAILURES */
      for (i = 0; i < word->best_choice->length(); ++i) {
        if (word->reject_map[i].rejected())
          word->reject_map[i].setrej_minimal_rej_accept();
      }
    }
    if (tessedit_minimal_rejection) {
      /* OVERRIDE ALL REJECTION MECHANISMS - ONLY REJECT TESS FAILURES */
      for (i = 0; i < word->best_choice->length(); ++i) {
        if ((word->best_choice->unichar_id(i) != space) &&
            word->reject_map[i].rejected())
          word->reject_map[i].setrej_minimal_rej_accept();
      }
    }
  }

  if (write_to_shm)
    write_shm_text (word, page_res_it.block ()->block,
      page_res_it.row (), *wordstr, wordstr_lengths);

#if 0
  if (tessedit_write_output)
    write_cooked_text (word->word, *wordstr, TRUE, FALSE, textfile);

  if (tessedit_write_raw_output)
    write_cooked_text (word->word, word->raw_choice->string (),
      TRUE, FALSE, rawfile);

  if (tessedit_write_txt_map)
    write_map(txt_mapfile, word);

  ep_choice = make_epaper_choice (word, newline_type);
  word->ep_choice = ep_choice;
#endif

  character_count += word->best_choice->length();
  word_count++;
}
}  // namespace tesseract

/**********************************************************************
 * make_epaper_choice
 *
 * Construct the epaper text string for a word, using the reject map to
 * determine whether each blob should be rejected.
 **********************************************************************/

#if 0
WERD_CHOICE *make_epaper_choice(                   //convert one word
                                WERD_RES *word,    //word to do
                                char newline_type  //type of newline
                               ) {
  inT16 index = 0;               //to string
  inT16 blobindex;               //to word
  inT16 prevright = 0;           //right of previous blob
  inT16 nextleft;                //left of next blob
  PBLOB *blob;
  TBOX inset_box;                 //bounding box
  PBLOB_IT blob_it;              //blob iterator
  char word_string[MAX_PATH];    //converted string
  BOOL8 force_total_reject;
  char unrecognised = STRING (unrecognised_char)[0];

  blob_it.set_to_list (word->outword->blob_list ());

  ASSERT_HOST (word->reject_map.length () ==
    word->best_choice->string ().length ());
  /*
  tprintf( "\"%s\" -> length: %d;  blobcount: %d (%d)\n",
      word->best_choice->string().string(),
        word->best_choice->string().length(),
      blob_it.length(),
        blob_count( word->outword ) );
  */

  if (word->best_choice->string ().length () == 0)
    force_total_reject = TRUE;
  else {
    force_total_reject = FALSE;
    ASSERT_HOST (blob_it.length () ==
      word->best_choice->string ().length ());
  }
  if (!blob_it.empty ()) {
    for (index = 0; index < word->word->space (); index++)
      word_string[index] = ' ';  //leading blanks
  }
  /* Why does this generate leading blanks regardless of whether the
  word_choice string is empty, when write_cooked_text ony generates leading
  blanks when the string is NOT empty???. */

  if (word->word->flag (W_REP_CHAR) && tessedit_write_rep_codes) {
    strcpy (word_string + index, "|^~R");
    index += 4;
    strcpy(word_string + index, unicharset.id_to_unichar(get_rep_char (word)));
    index += strlen(unicharset.id_to_unichar(get_rep_char (word)));
  }
  else {
    if (!blob_it.empty ())
      prevright = blob_it.data ()->bounding_box ().left ();
    //actually first left
    for (blobindex = 0, blob_it.mark_cycle_pt ();
    !blob_it.cycled_list (); blobindex++, blob_it.forward ()) {
      blob = blob_it.data ();
      if (word->reject_map[blobindex].accepted ()) {
        if (word->best_choice->string ()[blobindex] == ' ')
                                 //but not rejected!!
          word_string[index++] = unrecognised;
        else
          word_string[index++] =
            word->best_choice->string ()[blobindex];
      }
      else {                     // start reject
        inset_box = blob->bounding_box ();
        /* Extend reject box to include rejected neighbours */
        while (!blob_it.at_last () &&
          (force_total_reject ||
        (word->reject_map[blobindex + 1].rejected ()))) {
          blobindex++;
          blob = blob_it.forward ();
                                 //get total box
          inset_box += blob->bounding_box ();
        }
        if (blob_it.at_last ())
          nextleft = inset_box.right ();
        else
          nextleft = blob_it.data_relative (1)->bounding_box ().left ();

        //       tprintf("Making reject from (%d,%d)->(%d,%d)\n",
        //          inset_box.left(),inset_box.bottom(),
        //          inset_box.right(),inset_box.top());

        index += make_reject (&inset_box, prevright, nextleft,
          &word->denorm, &word_string[index]);
      }
      prevright = blob->bounding_box ().right ();
    }
  }
  if (newline_type)
                                 //end line
    word_string[index++] = newline_type;
  word_string[index] = '\0';     //terminate string
  if (strlen (word_string) != index) {
    tprintf ("ASSERT ABOUT TO FAIL: %s, index %d len %d\n",
      word_string, index, strlen (word_string));
  }
                                 //don't pass any zeros
  ASSERT_HOST (strlen (word_string) == index);
  return new WERD_CHOICE (word_string, 0, 0, NO_PERM);
}
#endif

/**********************************************************************
 * make_reject
 *
 * Add the escape code to the string for the reject.
 **********************************************************************/

inT16
make_reject (                    //make reject code
TBOX * inset_box,                 //bounding box
inT16 prevright,                 //previous char
inT16 nextleft,                  //next char
DENORM * denorm,                 //de-normalizer
char word_string[]               //output string
) {
  inT16 index;                   //to string
  inT16 xpos;                    //start of inset
  inT16 ypos;
  inT16 width;                   //size of inset
  inT16 height;
  inT16 left_offset;             //shift form prev char
  inT16 right_offset;            //shift to next char
  inT16 baseline_offset;         //shift from baseline
  inT16 inset_index = 0;         //number of inset
  inT16 min_chars;               //min width estimate
  inT16 max_chars;               //max width estimate
  float x_centre;                //centre of box

  index = 0;
  x_centre = (inset_box->left () + inset_box->right ()) / 2.0;
  left_offset =
    (inT16) (denorm->x (inset_box->left ()) - denorm->x (prevright));
  right_offset =
    (inT16) (denorm->x (nextleft) - denorm->x (inset_box->right ()));
  xpos = (inT16) floor (denorm->x (inset_box->left ()));
  width = (inT16) ceil (denorm->x (inset_box->right ())) - xpos;
  ypos = (inT16) floor (denorm->y (inset_box->bottom (), x_centre));
  height = (inT16) ceil (denorm->y (inset_box->top (), x_centre)) - ypos;
  baseline_offset = ypos - (inT16) denorm->y (bln_baseline_offset, x_centre);
                                 //escape code
  word_string[index++] = CTRL_INSET;
  min_chars = (inT16) ceil (0.27 * width / denorm->row ()->x_height ());
  max_chars = (inT16) floor (1.8 * width / denorm->row ()->x_height ());
  /*
  Ensure min_chars and max_chars are in the range 0..254. This ensures that
  we can add 1 to them to avoid putting \0 in a string, and still not exceed
  the max value in a byte.
  */
  if (min_chars < 0)
    min_chars = 0;
  if (min_chars > 254)
    min_chars = 254;
  if (max_chars < min_chars)
    max_chars = min_chars;
  if (max_chars > 254)
    max_chars = 254;
                                 //min chars
  word_string[index++] = min_chars + 1;
                                 //max chars
  word_string[index++] = max_chars + 1;
  word_string[index++] = 2;      //type?
                                 //store index
  word_string[index++] = inset_index / 255 + 1;
  word_string[index++] = inset_index % 255 + 1;
  return index;                  //size of string
}


/**********************************************************************
 * determine_newline_type
 *
 * Find whether we have a wrapping or hard newline.
 * Return FALSE if not at end of line.
 **********************************************************************/

char determine_newline_type(                   //test line ends
                            WERD *word,        //word to do
                            BLOCK *block,      //current block
                            WERD *next_word,   //next word
                            BLOCK *next_block  //block of next word
                           ) {
  inT16 end_gap;                 //to right edge
  inT16 width;                   //of next word
  TBOX word_box;                  //bounding
  TBOX next_box;                  //next word
  TBOX block_box;                 //block bounding

  if (!word->flag (W_EOL))
    return FALSE;                //not end of line
  if (next_word == NULL || next_block == NULL || block != next_block)
    return CTRL_NEWLINE;
  if (next_word->space () > 0)
    return CTRL_HARDLINE;        //it is tabbed
  word_box = word->bounding_box ();
  next_box = next_word->bounding_box ();
  block_box = block->bounding_box ();
                                 //gap to eol
  end_gap = block_box.right () - word_box.right ();
  end_gap -= (inT32) block->space ();
  width = next_box.right () - next_box.left ();
  //      tprintf("end_gap=%d-%d=%d, width=%d-%d=%d, nl=%d\n",
  //              block_box.right(),word_box.right(),end_gap,
  //              next_box.right(),next_box.left(),width,
  //              end_gap>width ? CTRL_HARDLINE : CTRL_NEWLINE);
  return end_gap > width ? CTRL_HARDLINE : CTRL_NEWLINE;
}

/**********************************************************************
 * write_shm_text
 *
 * Write the cooked text to the shared memory for the api.
 **********************************************************************/

void write_shm_text(                    //write output
                    WERD_RES *word,     //word to do
                    BLOCK *block,       //block it is from
                    ROW_RES *row,       //row it is from
                    const STRING &text, //text to write
                    const STRING &text_lengths
                   ) {
  inT32 index;                   //char counter
  inT32 index2;                  //char counter
  inT32 length;                  //chars in word
  inT32 ptsize;                  //font size
  inT8 blanks;                   //blanks in word
  uinT8 enhancement;             //bold etc
  uinT8 font;                    //font index
  char unrecognised = STRING (unrecognised_char)[0];
  PBLOB *blob;
  TBOX blob_box;                  //bounding box
  PBLOB_IT blob_it;              //blob iterator
  WERD copy_outword;             // copy to denorm
  uinT32 rating;                 //of char
  BOOL8 lineend;                 //end of line
  int offset;
  int offset2;

                                 //point size
  ptsize = pixels_to_pts ((inT32) (row->row->x_height () + row->row->ascenders () - row->row->descenders ()), 300);
  if (word->word->flag (W_BOL) && ocr_char_space () < 128
    && ocr_send_text (TRUE) != OKAY)
    return;                      //release failed
  copy_outword = *(word->outword);
  copy_outword.baseline_denormalise (&word->denorm);
  blob_it.set_to_list (copy_outword.blob_list ());
  length = text_lengths.length ();

  if (length > 0) {
    blanks = word->word->space ();
    if (blanks == 0 && tessedit_word_for_word && !word->word->flag (W_BOL))
      blanks = 1;
    for (index = 0, offset = 0; index < length;
         offset += text_lengths[index++], blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();

      enhancement = 0;
      if (word->italic > 0 || (word->italic == 0 && row->italic > 0))
        enhancement |= EUC_ITALIC;
      if (word->bold > 0 || (word->bold == 0 && row->bold > 0))
        enhancement |= EUC_BOLD;
      if (tessedit_write_ratings)
        rating = (uinT32) (-word->best_choice->certainty () / 0.035);
      else if (tessedit_zero_rejection)
        rating = text[offset] == ' ' ? 100 : 0;
      else
        rating = word->reject_map[index].accepted ()? 0 : 100;
      if (rating > 255)
        rating = 255;
      if (word->font1_count > 2)
        font = word->font1;
      else if (row->font1_count > 8)
        font = row->font1;
      else
                                 //font index
        font = word->word->flag (W_DONT_CHOP) ? 0 : 1;

      lineend = word->word->flag (W_EOL) && index == length - 1;
      if (word->word->flag (W_EOL) && tessedit_zero_rejection
      && index < length - 1 && text[index + text_lengths[index]] == ' ') {
        for (index2 = index + 1, offset2 = offset + text_lengths[index];
             index2 < length && text[offset2] == ' ';
             offset2 += text_lengths[index2++]);
        if (index2 == length)
          lineend = TRUE;
      }

      if (!tessedit_zero_rejection || text[offset] != ' '
      || tessedit_word_for_word) {
                                 //confidence
        if (text[offset] == ' ') {
        ocr_append_char (unrecognised,
                         blob_box.left (), blob_box.right (),
                         page_image.get_ysize () - 1 - blob_box.top (),
                         page_image.get_ysize () - 1 - blob_box.bottom (),
                         font, (uinT8) rating,
                         ptsize,                //point size
                         blanks, enhancement,   //enhancement
                         OCR_CDIR_LEFT_RIGHT,
                         OCR_LDIR_DOWN_RIGHT,
                         lineend ? OCR_NL_NEWLINE : OCR_NL_NONE);
        } else {
          for (int suboffset = 0; suboffset < text_lengths[index]; ++suboffset)
            ocr_append_char (static_cast<unsigned char>(text[offset+suboffset]),
                             blob_box.left (), blob_box.right (),
                             page_image.get_ysize () - 1 - blob_box.top (),
                             page_image.get_ysize () - 1 - blob_box.bottom (),
                             font, (uinT8) rating,
                             ptsize,                //point size
                             blanks, enhancement,   //enhancement
                             OCR_CDIR_LEFT_RIGHT,
                             OCR_LDIR_DOWN_RIGHT,
                             lineend ? OCR_NL_NEWLINE : OCR_NL_NONE);
        }
        blanks = 0;
      }

    }
  }
  else if (tessedit_word_for_word) {
    blanks = word->word->space ();
    if (blanks == 0 && !word->word->flag (W_BOL))
      blanks = 1;
    blob_box = word->word->bounding_box ();

    enhancement = 0;
    if (word->italic > 0)
      enhancement |= EUC_ITALIC;
    if (word->bold > 0)
      enhancement |= EUC_BOLD;
    rating = 100;
    if (word->font1_count > 2)
      font = word->font1;
    else if (row->font1_count > 8)
      font = row->font1;
    else
                                 //font index
      font = word->word->flag (W_DONT_CHOP) ? 0 : 1;

    lineend = word->word->flag (W_EOL);

                                 //font index
    ocr_append_char (unrecognised,
                     blob_box.left (), blob_box.right (),
                     page_image.get_ysize () - 1 - blob_box.top (),
                     page_image.get_ysize () - 1 - blob_box.bottom (),
                     font,
                     rating,                    //confidence
                     ptsize,                    //point size
                     blanks, enhancement,       //enhancement
                     OCR_CDIR_LEFT_RIGHT,
                     OCR_LDIR_DOWN_RIGHT,
                     lineend ? OCR_NL_NEWLINE : OCR_NL_NONE);
  }
}


/**********************************************************************
 * write_map
 *
 * Write a map file of 0's and 1'a which associates characters from the .txt
 * file with those in the .etx file. 0 = .txt char was deleted. 1 = .txt char
 * is kept.  Note that there may be reject regions in the .etx file WITHOUT
 * .txt chars being rejected.  The map file should be the same length, and
 * the same number of lines as the .txt file
 *
 * The paramaterised input is because I thought I might be able to generate
 * multiple map files in a single run.  However, it didn't work because
 * newdiff needs etx files!
 **********************************************************************/

#if 0
void write_map(                //output a map file
               FILE *mapfile,  //mapfile to write to
               WERD_RES *word) {
  inT16 index;
  int status;
  STRING mapstr = "";

  if (word->best_choice->string ().length () > 0) {
    for (index = 0; index < word->word->space (); index++) {
      if (word->reject_spaces &&
        (suspect_level >= suspect_space_level) &&
        !tessedit_minimal_rejection && !tessedit_zero_rejection)
        /* Write rejected spaces to .map file ONLY. Newdiff converts these back to
        accepted spaces AFTER generating basic space stats but BEFORE using .etx */
        status = fprintf (mapfile, "0");
      else
        status = fprintf (mapfile, "1");
      if (status < 0)
        WRITEFAILED.error ("write_map", EXIT, "Space Errno: %d", errno);
    }

    if ((word->word->flag (W_REP_CHAR) && tessedit_write_rep_codes)) {
      for (index = 0; index < 5; index++)
        mapstr += '1';
    }
    else {
      ASSERT_HOST (word->reject_map.length () ==
        word->best_choice->string ().length ());

      for (index = 0; index < word->reject_map.length (); index++) {
        if (word->reject_map[index].accepted ())
          mapstr += '1';
        else
          mapstr += '0';
      }
    }
    status = fprintf (mapfile, "%s", mapstr.string ());
    if (status < 0)
      WRITEFAILED.error ("write_map", EXIT, "Map str Errno: %d", errno);
  }
  if (word->word->flag (W_EOL)) {
    status = fprintf (mapfile, "\n");
    if (status < 0)
      WRITEFAILED.error ("write_map", EXIT, "Newline Errno: %d", errno);
  }
  status = fflush (mapfile);
  if (status != 0)
    WRITEFAILED.error ("write_map", EXIT, "fflush Errno: %d", errno);
}
#endif


/*************************************************************************
 * open_file()
 *************************************************************************/

namespace tesseract {
FILE *Tesseract::open_outfile(  //open .map & .unlv file
                   const char *extension) {
  STRING file_name;
  FILE *outfile;

  file_name = imagebasename + extension;
  if (!(outfile = fopen (file_name.string (), "w"))) {
    CANTOPENFILE.error ("open_outfile", EXIT, "%s %d",
      file_name.string (), errno);
  }
  return outfile;
}
}  // namespace tesseract


#if 0
void write_unlv_text(WERD_RES *word) {
  const char *wordstr;

  char buff[512];                //string to output
  int i = 0;
  int j = 0;
  char unrecognised = STRING (unrecognised_char)[0];
  int status;
  char space_str[3];

  wordstr = word->best_choice->string ().string ();

  /* DONT need to do anything special for repeated char words - at this stage
  the repetition char has been identified and any other chars have been
  rejected.
  */

  for (; wordstr[i] != '\0'; i++) {
    if ((wordstr[i] == ' ') ||
      (wordstr[i] == '~') || (wordstr[i] == '^') || (wordstr[i] == '|'))
      buff[j++] = unrecognised;
    else {
      if (word->reject_map[i].rejected ())
        buff[j++] = '^';         //Add suspect marker
      buff[j++] = wordstr[i];
    }
  }
  buff[j] = '\0';

  if (strlen (wordstr) > 0) {
    if (word->reject_spaces &&
      (suspect_level >= suspect_space_level) &&
      !tessedit_minimal_rejection && !tessedit_zero_rejection)
      strcpy (space_str, "^ ");  //Suspect space
    else
      strcpy (space_str, " ");   //Certain space

    for (i = 0; i < word->word->space (); i++) {
      status = fprintf (unlv_file, "%s", space_str);
      if (status < 0)
        WRITEFAILED.error ("write_unlv_text", EXIT,
          "Space Errno: %d", errno);
    }

    status = fprintf (unlv_file, "%s", buff);
    if (status < 0)
      WRITEFAILED.error ("write_unlv_text", EXIT, "Word Errno: %d", errno);
  }
  if (word->word->flag (W_EOL)) {
    status = fprintf (unlv_file, "\n");
    if (status < 0)
      WRITEFAILED.error ("write_unlv_text", EXIT,
        "Newline Errno: %d", errno);
  }
  status = fflush (unlv_file);
  if (status != 0)
    WRITEFAILED.error ("write_unlv_text", EXIT, "Fflush Errno: %d", errno);
}
#endif


/*************************************************************************
 * get_rep_char()
 * Return the first accepted character from the repetition string. This is the
 * character which is repeated - as determined earlier by fix_rep_char()
 *************************************************************************/
namespace tesseract {
UNICHAR_ID Tesseract::get_rep_char(WERD_RES *word) {  // what char is repeated?
  int i;
  for (i = 0; ((i < word->reject_map.length()) &&
               (word->reject_map[i].rejected())); ++i);

  if (i < word->reject_map.length()) {
    return word->best_choice->unichar_id(i);
  } else {
    return unicharset.unichar_to_id(unrecognised_char.string());
  }
}
}  // namespace tesseract

void ensure_rep_chars_are_consistent(WERD_RES *word) {
#if 0
  char rep_char = get_rep_char (word);
  char *ptr;

  ptr = (char *) word->best_choice->string ().string ();
  for (; *ptr != '\0'; ptr++) {
    if (*ptr != rep_char)
      *ptr = rep_char;
  }
#endif

#if 0
  UNICHAR_ID rep_char = get_rep_char (word); //TODO(tkielbus) Reactivate
  int i;
  char *ptr;
  STRING consistent_string;
  STRING consistent_string_lengths;

  ptr = (char *) word->best_choice->string ().string ();
  for (i = 0; *ptr != '\0'; ptr += word->best_choice->lengths()[i++]) {
    consistent_string += unicharset.id_to_unichar(rep_char);
    consistent_string_lengths += strlen(unicharset.id_to_unichar(rep_char));
  }
  word->best_choice->string() = consistent_string;
  word->best_choice->lengths() = consistent_string_lengths;
#endif
}

/*************************************************************************
 * SUSPECT LEVELS
 *
 * 0 - dont reject ANYTHING
 * 1,2 - partial rejection
 * 3 - BEST
 *
 * NOTE: to reject JUST tess failures in the .map file set suspect_level 3 and
 * tessedit_minimal_rejection.
 *************************************************************************/

namespace tesseract {
void Tesseract::set_unlv_suspects(WERD_RES *word_res) {
  int len = word_res->reject_map.length();
  const WERD_CHOICE &word = *(word_res->best_choice);
  int i;
  float rating_per_ch;

  if (suspect_level == 0) {
    for (i = 0; i < len; i++) {
      if (word_res->reject_map[i].rejected())
        word_res->reject_map[i].setrej_minimal_rej_accept();
    }
    return;
  }

  if (suspect_level >= 3)
    return;                      //Use defaults

  /* NOW FOR LEVELS 1 and 2 Find some stuff to unreject*/

  if (safe_dict_word(word) &&
      (count_alphas(word) > suspect_short_words)) {
    /* Unreject alphas in dictionary words */
    for (i = 0; i < len; ++i) {
      if (word_res->reject_map[i].rejected() &&
          unicharset.get_isalpha(word.unichar_id(i)))
        word_res->reject_map[i].setrej_minimal_rej_accept();
    }
  }

  rating_per_ch = word.rating() / word_res->reject_map.length();

  if (rating_per_ch >= suspect_rating_per_ch)
    return;                      //Dont touch bad ratings

  if ((word_res->tess_accepted) || (rating_per_ch < suspect_accept_rating)) {
    /* Unreject any Tess Acceptable word - but NOT tess reject chs*/
    for (i = 0; i < len; ++i) {
      if (word_res->reject_map[i].rejected() &&
          (!unicharset.eq(word.unichar_id(i), " ")))
        word_res->reject_map[i].setrej_minimal_rej_accept();
    }
  }

  for (i = 0; i < len; i++) {
    if (word_res->reject_map[i].rejected()) {
      if (word_res->reject_map[i].flag(R_DOC_REJ))
        word_res->reject_map[i].setrej_minimal_rej_accept();
      if (word_res->reject_map[i].flag(R_BLOCK_REJ))
        word_res->reject_map[i].setrej_minimal_rej_accept();
      if (word_res->reject_map[i].flag(R_ROW_REJ))
        word_res->reject_map[i].setrej_minimal_rej_accept();
    }
  }

  if (suspect_level == 2)
    return;

  if (!suspect_constrain_1Il ||
      (word_res->reject_map.length() <= suspect_short_words)) {
    for (i = 0; i < len; i++) {
      if (word_res->reject_map[i].rejected()) {
        if ((word_res->reject_map[i].flag(R_1IL_CONFLICT) ||
          word_res->reject_map[i].flag(R_POSTNN_1IL)))
          word_res->reject_map[i].setrej_minimal_rej_accept();

        if (!suspect_constrain_1Il &&
          word_res->reject_map[i].flag(R_MM_REJECT))
          word_res->reject_map[i].setrej_minimal_rej_accept();
      }
    }
  }

  if ((acceptable_word_string(word.unichar_string().string(),
                              word.unichar_lengths().string()) !=
       AC_UNACCEPTABLE) ||
      acceptable_number_string(word.unichar_string().string(),
                               word.unichar_lengths().string())) {
    if (word_res->reject_map.length() > suspect_short_words) {
      for (i = 0; i < len; i++) {
        if (word_res->reject_map[i].rejected() &&
          (!word_res->reject_map[i].perm_rejected() ||
           word_res->reject_map[i].flag (R_1IL_CONFLICT) ||
           word_res->reject_map[i].flag (R_POSTNN_1IL) ||
           word_res->reject_map[i].flag (R_MM_REJECT))) {
          word_res->reject_map[i].setrej_minimal_rej_accept();
        }
      }
    }
  }
}

inT16 Tesseract::count_alphas(const WERD_CHOICE &word) {
  int count = 0;
  for (int i = 0; i < word.length(); ++i) {
    if (unicharset.get_isalpha(word.unichar_id(i)))
      count++;
  }
  return count;
}


inT16 Tesseract::count_alphanums(const WERD_CHOICE &word) {
  int count = 0;
  for (int i = 0; i < word.length(); ++i) {
    if (unicharset.get_isalpha(word.unichar_id(i)) ||
        unicharset.get_isdigit(word.unichar_id(i)))
      count++;
  }
  return count;
}


BOOL8 Tesseract::acceptable_number_string(const char *s,
                                          const char *lengths) {
  BOOL8 prev_digit = FALSE;

  if (*lengths == 1 && *s == '(')
    s++;

  if (*lengths == 1 &&
      ((*s == '$') || (*s == '.') || (*s == '+') || (*s == '-')))
    s++;

  for (; *s != '\0'; s += *(lengths++)) {
    if (unicharset.get_isdigit (s, *lengths))
      prev_digit = TRUE;
    else if (prev_digit &&
             (*lengths == 1 && ((*s == '.') || (*s == ',') || (*s == '-'))))
      prev_digit = FALSE;
    else if (prev_digit && *lengths == 1 &&
             (*(s + *lengths) == '\0') && ((*s == '%') || (*s == ')')))
      return TRUE;
    else if (prev_digit &&
             *lengths == 1 && (*s == '%') &&
             (*(lengths + 1) == 1 && *(s + *lengths) == ')') &&
             (*(s + *lengths + *(lengths + 1)) == '\0'))
      return TRUE;
    else
      return FALSE;
  }
  return TRUE;
}
}  // namespace tesseract
