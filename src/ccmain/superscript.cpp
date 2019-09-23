/******************************************************************
 * File:        superscript.cpp
 * Description: Correction pass to fix superscripts and subscripts.
 * Author:      David Eger
 *
 * (C) Copyright 2012, Google, Inc.
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

#include "normalis.h"
#include "tesseractclass.h"

static int LeadingUnicharsToChopped(WERD_RES *word, int num_unichars) {
  int num_chopped = 0;
  for (int i = 0; i < num_unichars; i++)
    num_chopped += word->best_state[i];
  return num_chopped;
}

static int TrailingUnicharsToChopped(WERD_RES *word, int num_unichars) {
  int num_chopped = 0;
  for (int i = 0; i < num_unichars; i++)
    num_chopped += word->best_state[word->best_state.size() - 1 - i];
  return num_chopped;
}


namespace tesseract {

/**
 * Given a recognized blob, see if a contiguous collection of sub-pieces
 * (chopped blobs) starting at its left might qualify as being a subscript
 * or superscript letter based only on y position.  Also do this for the
 * right side.
 */
static void YOutlierPieces(WERD_RES *word, int rebuilt_blob_index,
                           int super_y_bottom, int sub_y_top,
                           ScriptPos *leading_pos, int *num_leading_outliers,
                           ScriptPos *trailing_pos,
                           int *num_trailing_outliers) {
  ScriptPos sp_unused1, sp_unused2;
  int unused1, unused2;
  if (!leading_pos) leading_pos = &sp_unused1;
  if (!num_leading_outliers) num_leading_outliers = &unused1;
  if (!trailing_pos) trailing_pos = &sp_unused2;
  if (!num_trailing_outliers) num_trailing_outliers = &unused2;

  *num_leading_outliers = *num_trailing_outliers = 0;
  *leading_pos = *trailing_pos = SP_NORMAL;

  int chopped_start = LeadingUnicharsToChopped(word, rebuilt_blob_index);
  int num_chopped_pieces = word->best_state[rebuilt_blob_index];
  ScriptPos last_pos = SP_NORMAL;
  int trailing_outliers = 0;
  for (int i = 0; i < num_chopped_pieces; i++) {
    TBOX box = word->chopped_word->blobs[chopped_start + i]->bounding_box();
    ScriptPos pos = SP_NORMAL;
    if (box.bottom() >= super_y_bottom) {
      pos = SP_SUPERSCRIPT;
    } else if (box.top() <= sub_y_top) {
      pos = SP_SUBSCRIPT;
    }
    if (pos == SP_NORMAL) {
      if (trailing_outliers == i) {
        *num_leading_outliers = trailing_outliers;
        *leading_pos = last_pos;
      }
      trailing_outliers = 0;
    } else {
      if (pos == last_pos) {
        trailing_outliers++;
      } else {
        trailing_outliers = 1;
      }
    }
    last_pos = pos;
  }
  *num_trailing_outliers = trailing_outliers;
  *trailing_pos = last_pos;
}

/**
 * Attempt to split off any high (or low) bits at the ends of the word with poor
 * certainty and recognize them separately.  If the certainty gets much better
 * and other sanity checks pass, accept.
 *
 * This superscript fix is meant to be called in the second pass of recognition
 * when we have tried once and already have a preliminary answer for word.
 *
 * @return Whether we modified the given word.
 */
bool Tesseract::SubAndSuperscriptFix(WERD_RES *word) {
  if (word->tess_failed || word->word->flag(W_REP_CHAR) ||
      !word->best_choice) {
    return false;
  }
  int num_leading, num_trailing;
  ScriptPos sp_leading, sp_trailing;
  float leading_certainty, trailing_certainty;
  float avg_certainty, unlikely_threshold;

  // Calculate the number of whole suspicious characters at the edges.
  GetSubAndSuperscriptCandidates(
          word, &num_leading, &sp_leading, &leading_certainty,
          &num_trailing, &sp_trailing, &trailing_certainty,
          &avg_certainty, &unlikely_threshold);

  const char *leading_pos = sp_leading == SP_SUBSCRIPT ? "sub" : "super";
  const char *trailing_pos = sp_trailing == SP_SUBSCRIPT ? "sub" : "super";

  int num_blobs = word->best_choice->length();

  // Calculate the remainder (partial characters) at the edges.
  // This accounts for us having classified the best version of
  // a word as [speaker?'] when it was instead [speaker.^{21}]
  // (that is we accidentally thought the 2 was attached to the period).
  int num_remainder_leading = 0, num_remainder_trailing = 0;
  if (num_leading + num_trailing < num_blobs && unlikely_threshold < 0.0) {
    int super_y_bottom =
        kBlnBaselineOffset + kBlnXHeight * superscript_min_y_bottom;
    int sub_y_top =
        kBlnBaselineOffset + kBlnXHeight * subscript_max_y_top;
    int last_word_char = num_blobs - 1 - num_trailing;
    float last_char_certainty = word->best_choice->certainty(last_word_char);
    if (word->best_choice->unichar_id(last_word_char) != 0 &&
        last_char_certainty <= unlikely_threshold) {
      ScriptPos rpos;
      YOutlierPieces(word, last_word_char, super_y_bottom, sub_y_top,
                     nullptr, nullptr, &rpos, &num_remainder_trailing);
      if (num_trailing > 0 && rpos != sp_trailing) num_remainder_trailing = 0;
      if (num_remainder_trailing > 0 &&
          last_char_certainty < trailing_certainty) {
        trailing_certainty = last_char_certainty;
      }
    }
    bool another_blob_available = (num_remainder_trailing == 0) ||
        num_leading + num_trailing + 1 < num_blobs;
    int first_char_certainty = word->best_choice->certainty(num_leading);
    if (another_blob_available &&
        word->best_choice->unichar_id(num_leading) != 0 &&
        first_char_certainty <= unlikely_threshold) {
      ScriptPos lpos;
      YOutlierPieces(word, num_leading, super_y_bottom, sub_y_top,
                     &lpos, &num_remainder_leading, nullptr, nullptr);
      if (num_leading > 0 && lpos != sp_leading) num_remainder_leading = 0;
      if (num_remainder_leading > 0 &&
          first_char_certainty < leading_certainty) {
        leading_certainty = first_char_certainty;
      }
    }
  }

  // If nothing to do, bail now.
  if (num_leading + num_trailing +
      num_remainder_leading + num_remainder_trailing == 0) {
    return false;
  }

  if (superscript_debug >= 1) {
    tprintf("Candidate for superscript detection: %s (",
            word->best_choice->unichar_string().c_str());
    if (num_leading || num_remainder_leading) {
      tprintf("%d.%d %s-leading ", num_leading, num_remainder_leading,
              leading_pos);
    }
    if (num_trailing || num_remainder_trailing) {
      tprintf("%d.%d %s-trailing ", num_trailing, num_remainder_trailing,
              trailing_pos);
    }
    tprintf(")\n");
  }
  if (superscript_debug >= 3) {
    word->best_choice->print();
  }
  if (superscript_debug >= 2) {
    tprintf(" Certainties -- Average: %.2f  Unlikely thresh: %.2f  ",
            avg_certainty, unlikely_threshold);
    if (num_leading)
      tprintf("Orig. leading (min): %.2f  ", leading_certainty);
    if (num_trailing)
      tprintf("Orig. trailing (min): %.2f  ", trailing_certainty);
    tprintf("\n");
  }

  // We've now calculated the number of rebuilt blobs we want to carve off.
  // However, split_word() works from TBLOBs in chopped_word, so we need to
  // convert to those.
  int num_chopped_leading =
      LeadingUnicharsToChopped(word, num_leading) + num_remainder_leading;
  int num_chopped_trailing =
      TrailingUnicharsToChopped(word, num_trailing) + num_remainder_trailing;

  int retry_leading = 0;
  int retry_trailing = 0;
  bool is_good = false;
  WERD_RES *revised = TrySuperscriptSplits(
      num_chopped_leading, leading_certainty, sp_leading,
      num_chopped_trailing, trailing_certainty, sp_trailing,
      word, &is_good, &retry_leading, &retry_trailing);
  if (is_good) {
    word->ConsumeWordResults(revised);
  } else if (retry_leading || retry_trailing) {
    int retry_chopped_leading =
        LeadingUnicharsToChopped(revised, retry_leading);
    int retry_chopped_trailing =
        TrailingUnicharsToChopped(revised, retry_trailing);
    WERD_RES *revised2 = TrySuperscriptSplits(
        retry_chopped_leading, leading_certainty, sp_leading,
        retry_chopped_trailing, trailing_certainty, sp_trailing,
        revised, &is_good, &retry_leading, &retry_trailing);
    if (is_good) {
      word->ConsumeWordResults(revised2);
    }
    delete revised2;
  }
  delete revised;
  return is_good;
}

/**
 * Determine how many characters (rebuilt blobs) on each end of a given word
 * might plausibly be superscripts so SubAndSuperscriptFix can try to
 * re-recognize them.  Even if we find no whole blobs at either end,
 * we will set *unlikely_threshold to a certainty that might be used to
 * select "bad enough" outlier characters.  If *unlikely_threshold is set to 0,
 * though, there's really no hope.
 *
 * @param[in]  word    The word to examine.
 * @param[out] num_rebuilt_leading   the number of rebuilt blobs at the start
 *                                   of the word which are all up or down and
 *                                   seem badly classified.
 * @param[out] leading_pos        "super" or "sub" (for debugging)
 * @param[out] leading_certainty  the worst certainty in the leading blobs.
 * @param[out] num_rebuilt_trailing   the number of rebuilt blobs at the end
 *                                    of the word which are all up or down and
 *                                    seem badly classified.
 * @param[out] trailing_pos        "super" or "sub" (for debugging)
 * @param[out] trailing_certainty  the worst certainty in the trailing blobs.
 * @param[out] avg_certainty       the average certainty of "normal" blobs in
 *                                 the word.
 * @param[out] unlikely_threshold  the threshold (on certainty) we used to
 *                                 select "bad enough" outlier characters.
 */
void Tesseract::GetSubAndSuperscriptCandidates(const WERD_RES *word,
                                               int *num_rebuilt_leading,
                                               ScriptPos *leading_pos,
                                               float *leading_certainty,
                                               int *num_rebuilt_trailing,
                                               ScriptPos *trailing_pos,
                                               float *trailing_certainty,
                                               float *avg_certainty,
                                               float *unlikely_threshold) {
  *avg_certainty = *unlikely_threshold = 0.0f;
  *num_rebuilt_leading = *num_rebuilt_trailing = 0;
  *leading_certainty = *trailing_certainty = 0.0f;

  int super_y_bottom =
      kBlnBaselineOffset + kBlnXHeight * superscript_min_y_bottom;
  int sub_y_top =
      kBlnBaselineOffset + kBlnXHeight * subscript_max_y_top;

  // Step one: Get an average certainty for "normally placed" characters.

  // Counts here are of blobs in the rebuild_word / unichars in best_choice.
  *leading_pos = *trailing_pos = SP_NORMAL;
  int leading_outliers = 0;
  int trailing_outliers = 0;
  int num_normal = 0;
  float normal_certainty_total = 0.0f;
  float worst_normal_certainty = 0.0f;
  ScriptPos last_pos = SP_NORMAL;
  int num_blobs = word->rebuild_word->NumBlobs();
  for (int b = 0; b < num_blobs; ++b) {
    TBOX box = word->rebuild_word->blobs[b]->bounding_box();
    ScriptPos pos = SP_NORMAL;
    if (box.bottom() >= super_y_bottom) {
      pos = SP_SUPERSCRIPT;
    } else if (box.top() <= sub_y_top) {
      pos = SP_SUBSCRIPT;
    }
    if (pos == SP_NORMAL) {
      if (word->best_choice->unichar_id(b) != 0) {
        float char_certainty = word->best_choice->certainty(b);
        if (char_certainty < worst_normal_certainty) {
          worst_normal_certainty = char_certainty;
        }
        num_normal++;
        normal_certainty_total += char_certainty;
      }
      if (trailing_outliers == b) {
        leading_outliers = trailing_outliers;
        *leading_pos = last_pos;
      }
      trailing_outliers = 0;
    } else {
      if (last_pos == pos) {
        trailing_outliers++;
      } else {
        trailing_outliers = 1;
      }
    }
    last_pos = pos;
  }
  *trailing_pos = last_pos;
  if (num_normal >= 3) {  // throw out the worst as an outlier.
    num_normal--;
    normal_certainty_total -= worst_normal_certainty;
  }
  if (num_normal > 0) {
    *avg_certainty = normal_certainty_total / num_normal;
    *unlikely_threshold = superscript_worse_certainty * (*avg_certainty);
  }
  if (num_normal == 0 ||
      (leading_outliers == 0 && trailing_outliers == 0)) {
    return;
  }

  // Step two: Try to split off bits of the word that are both outliers
  //           and have much lower certainty than average
  // Calculate num_leading and leading_certainty.
  for (*leading_certainty = 0.0f, *num_rebuilt_leading = 0;
       *num_rebuilt_leading < leading_outliers;
       (*num_rebuilt_leading)++) {
    float char_certainty = word->best_choice->certainty(*num_rebuilt_leading);
    if (char_certainty > *unlikely_threshold) {
      break;
    }
    if (char_certainty < *leading_certainty) {
      *leading_certainty = char_certainty;
    }
  }

  // Calculate num_trailing and trailing_certainty.
  for (*trailing_certainty = 0.0f, *num_rebuilt_trailing = 0;
       *num_rebuilt_trailing < trailing_outliers;
       (*num_rebuilt_trailing)++) {
    int blob_idx = num_blobs - 1 - *num_rebuilt_trailing;
    float char_certainty = word->best_choice->certainty(blob_idx);
    if (char_certainty > *unlikely_threshold) {
      break;
    }
    if (char_certainty < *trailing_certainty) {
      *trailing_certainty = char_certainty;
    }
  }
}


/**
 * Try splitting off the given number of (chopped) blobs from the front and
 * back of the given word and recognizing the pieces.
 *
 * @param[in]  num_chopped_leading   how many chopped blobs from the left
 *                    end of the word to chop off and try recognizing as a
 *                    superscript (or subscript)
 * @param[in]  leading_certainty     the (minimum) certainty had by the
 *                    characters in the original leading section.
 * @param[in]  leading_pos    "super" or "sub" (for debugging)
 * @param[in]  num_chopped_trailing  how many chopped blobs from the right
 *                    end of the word to chop off and try recognizing as a
 *                    superscript (or subscript)
 * @param[in]  trailing_certainty    the (minimum) certainty had by the
 *                    characters in the original trailing section.
 * @param[in]  trailing_pos      "super" or "sub" (for debugging)
 * @param[in]  word              the word to try to chop up.
 * @param[out] is_good           do we believe our result?
 * @param[out] retry_rebuild_leading, retry_rebuild_trailing
 *         If non-zero, and !is_good, then the caller may have luck trying
 *         to split the returned word with this number of (rebuilt) leading
 *         and trailing blobs / unichars.
 * @return A word which is the result of re-recognizing as asked.
 */
WERD_RES *Tesseract::TrySuperscriptSplits(
    int num_chopped_leading, float leading_certainty, ScriptPos leading_pos,
    int num_chopped_trailing, float trailing_certainty,
    ScriptPos trailing_pos,
    WERD_RES *word,
    bool *is_good,
    int *retry_rebuild_leading, int *retry_rebuild_trailing) {
  int num_chopped = word->chopped_word->NumBlobs();

  *retry_rebuild_leading = *retry_rebuild_trailing = 0;

  // Chop apart the word into up to three pieces.

  BlamerBundle *bb0 = nullptr;
  BlamerBundle *bb1 = nullptr;
  WERD_RES *prefix = nullptr;
  WERD_RES *core = nullptr;
  WERD_RES *suffix = nullptr;
  if (num_chopped_leading > 0) {
    prefix = new WERD_RES(*word);
    split_word(prefix, num_chopped_leading, &core, &bb0);
  } else {
    core = new WERD_RES(*word);
  }

  if (num_chopped_trailing > 0) {
    int split_pt = num_chopped - num_chopped_trailing - num_chopped_leading;
    split_word(core, split_pt, &suffix, &bb1);
  }

  //  Recognize the pieces in turn.
  int saved_cp_multiplier = classify_class_pruner_multiplier;
  int saved_im_multiplier = classify_integer_matcher_multiplier;
  if (prefix) {
    // Turn off Tesseract's y-position penalties for the leading superscript.
    classify_class_pruner_multiplier.set_value(0);
    classify_integer_matcher_multiplier.set_value(0);

    // Adjust our expectations about the baseline for this prefix.
    if (superscript_debug >= 3) {
      tprintf(" recognizing first %d chopped blobs\n", num_chopped_leading);
    }
    recog_word_recursive(prefix);
    if (superscript_debug >= 2) {
      tprintf(" The leading bits look like %s %s\n",
              ScriptPosToString(leading_pos),
              prefix->best_choice->unichar_string().c_str());
    }

    // Restore the normal y-position penalties.
    classify_class_pruner_multiplier.set_value(saved_cp_multiplier);
    classify_integer_matcher_multiplier.set_value(saved_im_multiplier);
  }

  if (superscript_debug >= 3) {
    tprintf(" recognizing middle %d chopped blobs\n",
            num_chopped - num_chopped_leading - num_chopped_trailing);
  }

  if (suffix) {
    // Turn off Tesseract's y-position penalties for the trailing superscript.
    classify_class_pruner_multiplier.set_value(0);
    classify_integer_matcher_multiplier.set_value(0);

    if (superscript_debug >= 3) {
      tprintf(" recognizing last %d chopped blobs\n", num_chopped_trailing);
    }
    recog_word_recursive(suffix);
    if (superscript_debug >= 2) {
      tprintf(" The trailing bits look like %s %s\n",
              ScriptPosToString(trailing_pos),
              suffix->best_choice->unichar_string().c_str());
    }

    // Restore the normal y-position penalties.
    classify_class_pruner_multiplier.set_value(saved_cp_multiplier);
    classify_integer_matcher_multiplier.set_value(saved_im_multiplier);
  }

  // Evaluate whether we think the results are believably better
  // than what we already had.
  bool good_prefix = !prefix || BelievableSuperscript(
      superscript_debug >= 1, *prefix,
      superscript_bettered_certainty * leading_certainty,
      retry_rebuild_leading, nullptr);
  bool good_suffix = !suffix || BelievableSuperscript(
      superscript_debug >= 1, *suffix,
      superscript_bettered_certainty * trailing_certainty,
      nullptr, retry_rebuild_trailing);

  *is_good = good_prefix && good_suffix;
  if (!*is_good && !*retry_rebuild_leading && !*retry_rebuild_trailing) {
    // None of it is any good. Quit now.
    delete core;
    delete prefix;
    delete suffix;
    delete bb1;
    return nullptr;
  }
  recog_word_recursive(core);

  // Now paste the results together into core.
  if (suffix) {
    suffix->SetAllScriptPositions(trailing_pos);
    join_words(core, suffix, bb1);
  }
  if (prefix) {
    prefix->SetAllScriptPositions(leading_pos);
    join_words(prefix, core, bb0);
    core = prefix;
    prefix = nullptr;
  }

  if (superscript_debug >= 1) {
    tprintf("%s superscript fix: %s\n", *is_good ? "ACCEPT" : "REJECT",
            core->best_choice->unichar_string().c_str());
  }
  return core;
}


/**
 * Return whether this is believable superscript or subscript text.
 *
 * We insist that:
 *   + there are no punctuation marks.
 *   + there are no italics.
 *   + no normal-sized character is smaller than superscript_scaledown_ratio
 *     of what it ought to be, and
 *   + each character is at least as certain as certainty_threshold.
 *
 *  @param[in]  debug  If true, spew debug output
 *  @param[in]  word   The word whose best_choice we're evaluating
 *  @param[in]  certainty_threshold   If any of the characters have less
 *                    certainty than this, reject.
 *  @param[out]  left_ok  How many left-side characters were ok?
 *  @param[out]  right_ok  How many right-side characters were ok?
 *  @return  Whether the complete best choice is believable as a superscript.
 */
bool Tesseract::BelievableSuperscript(bool debug,
                                      const WERD_RES &word,
                                      float certainty_threshold,
                                      int *left_ok,
                                      int *right_ok) const {
  int initial_ok_run_count = 0;
  int ok_run_count = 0;
  float worst_certainty = 0.0f;
  const WERD_CHOICE &wc = *word.best_choice;

  const UnicityTable<FontInfo>& fontinfo_table = get_fontinfo_table();
  for (int i = 0; i < wc.length(); i++) {
    TBLOB *blob = word.rebuild_word->blobs[i];
    UNICHAR_ID unichar_id = wc.unichar_id(i);
    float char_certainty = wc.certainty(i);
    bool bad_certainty = char_certainty < certainty_threshold;
    bool is_punc = wc.unicharset()->get_ispunctuation(unichar_id);
    bool is_italic = word.fontinfo && word.fontinfo->is_italic();
    BLOB_CHOICE *choice = word.GetBlobChoice(i);
    if (choice && fontinfo_table.size() > 0) {
      // Get better information from the specific choice, if available.
      int font_id1 = choice->fontinfo_id();
      bool font1_is_italic = font_id1 >= 0
          ? fontinfo_table.get(font_id1).is_italic() : false;
      int font_id2 = choice->fontinfo_id2();
      is_italic = font1_is_italic &&
          (font_id2 < 0 || fontinfo_table.get(font_id2).is_italic());
    }

    float height_fraction = 1.0f;
    float char_height = blob->bounding_box().height();
    float normal_height = char_height;
    if (wc.unicharset()->top_bottom_useful()) {
      int min_bot, max_bot, min_top, max_top;
      wc.unicharset()->get_top_bottom(unichar_id,
                                      &min_bot, &max_bot,
                                      &min_top, &max_top);
      float hi_height = max_top - max_bot;
      float lo_height = min_top - min_bot;
      normal_height = (hi_height + lo_height) / 2;
      if (normal_height >= kBlnXHeight) {
        // Only ding characters that we have decent information for because
        // they're supposed to be normal sized, not tiny specks or dashes.
        height_fraction = char_height / normal_height;
      }
    }
    bool bad_height = height_fraction < superscript_scaledown_ratio;

    if (debug) {
      if (is_italic) {
        tprintf(" Rejecting: superscript is italic.\n");
      }
      if (is_punc) {
        tprintf(" Rejecting: punctuation present.\n");
      }
      const char *char_str = wc.unicharset()->id_to_unichar(unichar_id);
      if (bad_certainty) {
        tprintf(" Rejecting: don't believe character %s with certainty %.2f "
                "which is less than threshold %.2f\n", char_str,
                char_certainty, certainty_threshold);
      }
      if (bad_height) {
        tprintf(" Rejecting: character %s seems too small @ %.2f versus "
                "expected %.2f\n", char_str, char_height, normal_height);
      }
    }
    if (bad_certainty || bad_height || is_punc || is_italic) {
      if (ok_run_count == i) {
        initial_ok_run_count = ok_run_count;
      }
      ok_run_count = 0;
    } else {
      ok_run_count++;
    }
    if (char_certainty < worst_certainty) {
      worst_certainty = char_certainty;
    }
  }
  bool all_ok = ok_run_count == wc.length();
  if (all_ok && debug) {
    tprintf(" Accept: worst revised certainty is %.2f\n", worst_certainty);
  }
  if (!all_ok) {
    if (left_ok) *left_ok = initial_ok_run_count;
    if (right_ok) *right_ok = ok_run_count;
  }
  return all_ok;
}


}  // namespace tesseract
