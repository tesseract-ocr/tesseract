/**********************************************************************
 * File:        ratngs.h  (Formerly ratings.h)
 * Description: Definition of the WERD_CHOICE and BLOB_CHOICE classes.
 * Author:      Ray Smith
 * Created:     Thu Apr 23 11:40:38 BST 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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

#ifndef           RATNGS_H
#define           RATNGS_H

#include <assert.h>

#include "clst.h"
#include "genericvector.h"
#include "notdll.h"
#include "unichar.h"
#include "unicharset.h"
#include "werd.h"

class BLOB_CHOICE: public ELIST_LINK
{
  public:
    BLOB_CHOICE() {
      unichar_id_ = INVALID_UNICHAR_ID;
      fontinfo_id_ = -1;
      fontinfo_id2_ = -1;
      rating_ = MAX_FLOAT32;
      certainty_ = -MAX_FLOAT32;
      script_id_ = -1;
      language_model_state_ = NULL;
    }
    BLOB_CHOICE(UNICHAR_ID src_unichar_id,  // character id
                float src_rating,          // rating
                float src_cert,            // certainty
                inT16 src_fontinfo_id,      // font
                inT16 src_fontinfo_id2,     // 2nd choice font
                int script_id);            // script
    BLOB_CHOICE(const BLOB_CHOICE &other);
    ~BLOB_CHOICE() {}

    UNICHAR_ID unichar_id() const {
      return unichar_id_;
    }
    float rating() const {
      return rating_;
    }
    float certainty() const {
      return certainty_;
    }
    inT16 fontinfo_id() const {
      return fontinfo_id_;
    }
    inT16 fontinfo_id2() const {
      return fontinfo_id2_;
    }
    int script_id() const {
      return script_id_;
    }
    void *language_model_state() {
      return language_model_state_;
    }
    inT16 xgap_before() {
      return xgap_before_;
    }
    inT16 xgap_after() {
      return xgap_after_;
    }

    void set_unichar_id(UNICHAR_ID newunichar_id) {
      unichar_id_ = newunichar_id;
    }
    void set_rating(float newrat) {
      rating_ = newrat;
    }
    void set_certainty(float newrat) {
      certainty_ = newrat;
    }
    void set_fontinfo_id(inT16 newfont) {
      fontinfo_id_ = newfont;
    }
    void set_fontinfo_id2(inT16 newfont) {
      fontinfo_id2_ = newfont;
    }
    void set_script(int newscript_id) {
      script_id_ = newscript_id;
    }
    void set_language_model_state(void *language_model_state) {
      language_model_state_ = language_model_state;
    }
    void set_xgap_before(inT16 gap) {
      xgap_before_ = gap;
    }
    void set_xgap_after(inT16 gap) {
      xgap_after_ = gap;
    }
    static BLOB_CHOICE* deep_copy(const BLOB_CHOICE* src) {
      BLOB_CHOICE* choice = new BLOB_CHOICE;
      *choice = *src;
      return choice;
    }
    void print(const UNICHARSET *unicharset) {
      tprintf("r%.2f c%.2f : %d %s", rating_, certainty_, unichar_id_,
              (unicharset == NULL) ? "" :
              unicharset->debug_str(unichar_id_).string());
    }

 private:
  UNICHAR_ID unichar_id_;          // unichar id
  inT16 fontinfo_id_;              // char font information
  inT16 fontinfo_id2_;             // 2nd choice font information
  float rating_;                  // size related
  float certainty_;               // absolute
  int script_id_;
  // Stores language model information about this BLOB_CHOICE. Used during
  // the segmentation search for BLOB_CHOICEs in BLOB_CHOICE_LISTs that are
  // recorded in the ratings matrix.
  // The pointer is owned/managed by the segmentation search.
  void *language_model_state_;
  inT16 xgap_before_;
  inT16 xgap_after_;
};

// Make BLOB_CHOICE listable.
ELISTIZEH (BLOB_CHOICE) CLISTIZEH (BLOB_CHOICE_LIST)

// Permuter codes used in WERD_CHOICEs.
enum PermuterType {
  NO_PERM,            // 0
  PUNC_PERM,          // 1
  TOP_CHOICE_PERM,    // 2
  LOWER_CASE_PERM,    // 3
  UPPER_CASE_PERM,    // 4
  NGRAM_PERM,         // 5
  NUMBER_PERM,        // 6
  USER_PATTERN_PERM,  // 7
  SYSTEM_DAWG_PERM,   // 8
  DOC_DAWG_PERM,      // 9
  USER_DAWG_PERM,     // 10
  FREQ_DAWG_PERM,     // 11
  COMPOUND_PERM,      // 12
};

class WERD_CHOICE {
 public:
  static const float kBadRating;

  WERD_CHOICE() { this->init(8); }
  WERD_CHOICE(int reserved) { this->init(reserved); }
  WERD_CHOICE(const char *src_string,
              const char *src_lengths,
              float src_rating,
              float src_certainty,
              uinT8 src_permuter,
              const UNICHARSET &unicharset) {
    this->init(src_string, src_lengths, src_rating,
               src_certainty, src_permuter, unicharset);
  }
  WERD_CHOICE (const char *src_string, const UNICHARSET &unicharset);
  WERD_CHOICE(const WERD_CHOICE &word) {
    this->init(word.length());
    this->operator=(word);
  }
  ~WERD_CHOICE();

  inline int length() const {
    return length_;
  }
  inline const UNICHAR_ID *unichar_ids() const {
    return unichar_ids_;
  }
  inline const UNICHAR_ID unichar_id(int index) const {
    assert(index < length_);
    return unichar_ids_[index];
  }
  inline const char *fragment_lengths() const {
    return fragment_lengths_;
  }
  inline const char fragment_length(int index) const {
    assert(index < length_);
    return fragment_lengths_[index];
  }
  inline float rating() const {
    return rating_;
  }
  inline float certainty() const {
    return certainty_;
  }
  inline uinT8 permuter() const {
    return permuter_;
  }
  inline bool fragment_mark() const {
    return fragment_mark_;
  }
  inline BLOB_CHOICE_LIST_CLIST* blob_choices() {
    return blob_choices_;
  }
  inline void set_unichar_id(UNICHAR_ID unichar_id, int index) {
    assert(index < length_);
    unichar_ids_[index] = unichar_id;
  }
  inline void set_fragment_length(char flen, int index) {
    assert(index < length_);
    fragment_lengths_[index] = flen;
  }
  inline void set_rating(float new_val) {
    rating_ = new_val;
  }
  inline void set_certainty(float new_val) {
    certainty_ = new_val;
  }
  inline void set_permuter(uinT8 perm) {
    permuter_ = perm;
  }
  inline void set_fragment_mark(bool new_fragment_mark) {
    fragment_mark_ = new_fragment_mark;
  }
  // Note: this function should only be used if all the fields
  // are populated manually with set_* functions (rather than
  // (copy)constructors and append_* functions).
  inline void set_length(int len) {
    ASSERT_HOST(reserved_ >= len);
    length_ = len;
  }
  void set_blob_choices(BLOB_CHOICE_LIST_CLIST *blob_choices);

  /// Make more space in unichar_id_ and fragment_lengths_ arrays.
  inline void double_the_size() {
    unichar_ids_ = GenericVector<UNICHAR_ID>::double_the_size_memcpy(
        reserved_, unichar_ids_);
    fragment_lengths_ = GenericVector<char>::double_the_size_memcpy(
        reserved_, fragment_lengths_);
    reserved_ *= 2;
  }

  /// Initializes WERD_CHOICE - reserves length slots in unichar_ids_ and
  /// fragment_length_ arrays. Sets other values to default (blank) values.
  inline void init(int reserved) {
    reserved_ = reserved;
    unichar_ids_ = new UNICHAR_ID[reserved];
    fragment_lengths_ = new char[reserved];
    length_ = 0;
    rating_ = 0.0;
    certainty_ = MAX_FLOAT32;
    permuter_ = NO_PERM;
    fragment_mark_ = false;
    blob_choices_ = NULL;
    unichar_string_ = "";
    unichar_lengths_ = "";
  }

  /// Helper function to build a WERD_CHOICE from the given string,
  /// fragment lengths, rating, certainty and permuter.
  /// The function assumes that src_string is not NULL.
  /// src_lengths argument could be NULL, in which case the unichars
  /// in src_string are assumed to all be of length 1.
  void init(const char *src_string, const char *src_lengths,
            float src_rating, float src_certainty,
            uinT8 src_permuter, const UNICHARSET &current_unicharset);

  /// Set the fields in this choice to be default (bad) values.
  inline void make_bad() {
    length_ = 0;
    rating_ = kBadRating;
    certainty_ = -MAX_FLOAT32;
    fragment_mark_ = false;
    unichar_string_ = "";
    unichar_lengths_ = "";
  }

  /// This function assumes that there is enough space reserved
  /// in the WERD_CHOICE for adding another unichar.
  /// This is an efficient alternative to append_unichar_id().
  inline void append_unichar_id_space_allocated(
      UNICHAR_ID unichar_id, char fragment_length,
      float rating, float certainty) {
    assert(reserved_ > length_);
    length_++;
    this->set_unichar_id(unichar_id, fragment_length,
                         rating, certainty, length_-1);
  }

  void append_unichar_id(UNICHAR_ID unichar_id, char fragment_length,
                         float rating, float certainty);

  inline void set_unichar_id(UNICHAR_ID unichar_id, char fragment_length,
                             float rating, float certainty, int index) {
    assert(index < length_);
    unichar_ids_[index] = unichar_id;
    fragment_lengths_[index] = fragment_length;
    rating_ += rating;
    if (certainty < certainty_) {
      certainty_ = certainty;
    }
  }

  bool contains_unichar_id(UNICHAR_ID unichar_id) const;
  void remove_unichar_ids(int index, int num);
  inline void remove_last_unichar_id() { --length_; }
  inline void remove_unichar_id(int index) { this->remove_unichar_ids(index, 1); }
  void string_and_lengths(const UNICHARSET &current_unicharset,
                          STRING *word_str, STRING *word_lengths_str) const;
  const STRING debug_string(const UNICHARSET &current_unicharset) const {
    STRING word_str;
    for (int i = 0; i < length_; ++i) {
      word_str += current_unicharset.debug_str(unichar_ids_[i]);
      word_str += " ";
    }
    return word_str;
  }
  /// Since this function walks over the whole word to convert unichar ids
  /// to unichars, it is best to call it once, e.g. after all changes to
  /// unichar_ids_ in WERD_CHOICE are finished.
  void populate_unichars(const UNICHARSET &current_unicharset) {
    this->string_and_lengths(current_unicharset, &unichar_string_,
                             &unichar_lengths_);
  }
  /// Undoes populate_unichars, so that unichar_string_ and unichar_lengths_
  /// are empty.
  void depopulate_unichars() {
    unichar_string_ = "";
    unichar_lengths_ = "";
  }
  /// This function should only be called if populate_unichars()
  /// was called and WERD_CHOICE did not change since then.
  const STRING &unichar_string() const {
    assert(unichar_string_.length() <= 0 ||
           unichar_string_.length() >= length_);  // sanity check
    return unichar_string_;
  }
  /// This function should only be called if populate_unichars()
  /// was called and WERD_CHOICE did not change since then.
  const STRING &unichar_lengths() const {
    assert(unichar_lengths_.length() <= 0 ||
           unichar_lengths_.length() == length_);  // sanity check
    return unichar_lengths_;
  }
  const void print() const { this->print(""); }
  const void print(const char *msg) const;

  WERD_CHOICE& operator+= (     // concatanate
    const WERD_CHOICE & second);// second on first

  WERD_CHOICE& operator= (const WERD_CHOICE& source);

 private:
  UNICHAR_ID *unichar_ids_;  // unichar ids that represent the text of the word
  char *fragment_lengths_;   // number of fragments in each unichar
  int reserved_;             // size of the above arrays
  int length_;               // word length
  float rating_;             // size related
  float certainty_;          // absolute
  uinT8 permuter_;           // permuter code
  bool fragment_mark_;       // if true, indicates that this choice
                             // was chosen over a better one that
                             // contained a fragment
  BLOB_CHOICE_LIST_CLIST *blob_choices_;  // best choices for each blob

  // The following variables are only populated by calling populate_unichars().
  // They are not synchronized with the values in unichar_ids otherwise.
  STRING unichar_string_;
  STRING unichar_lengths_;
  bool unichar_info_present;

 private:
  void delete_blob_choices();
};

// Make WERD_CHOICE listable.
ELISTIZEH (WERD_CHOICE)
typedef GenericVector<BLOB_CHOICE_LIST *> BLOB_CHOICE_LIST_VECTOR;
typedef GenericVector<WERD_CHOICE_LIST *> WERD_CHOICE_LIST_VECTOR;

void print_ratings_list(const char *msg, BLOB_CHOICE_LIST *ratings);
void print_ratings_list(
    const char *msg,                      // intro message
    BLOB_CHOICE_LIST *ratings,            // list of results
    const UNICHARSET &current_unicharset  // unicharset that can be used
                                          // for id-to-unichar conversion
    );
void print_ratings_info(
    FILE *fp,                             // file to use
    BLOB_CHOICE_LIST *ratings,            // list of results
    const UNICHARSET &current_unicharset  // unicharset that can be used
                                          // for id-to-unichar conversion
    );
void print_char_choices_list(
    const char *msg,
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    const UNICHARSET &current_unicharset,
    BOOL8 detailed
    );

#endif
