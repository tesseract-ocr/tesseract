/**********************************************************************
 * File:        ratngs.h  (Formerly ratings.h)
 * Description: Definition of the WERD_CHOICE and BLOB_CHOICE classes.
 * Author:					Ray Smith
 * Created:					Thu Apr 23 11:40:38 BST 1992
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

#include "clst.h"
#include "werd.h"
#include "notdll.h"
#include "unichar.h"

class BLOB_CHOICE:public ELIST_LINK
{
  public:
    BLOB_CHOICE() {  //empty
    }
    BLOB_CHOICE(                      //constructor
                char *src_unichar,    //character
                float src_rating,     //rating
                float src_cert,       //certainty
                inT8 src_config,      //config (font)
                const char* script);  //script

    void set_unichar(  //change it
                   char *newunichar) {
      strcpy(blob_unichar, newunichar);
    }
    void set_rating(  //change it
                    float newrat) {
      blob_rating = newrat;
    }
    void set_certainty(  //change it
                       float newrat) {
      blob_certainty = newrat;
    }
    void set_config(  //change it
                    inT8 newfont) {
      blob_config = newfont;
    }
    void set_script(  //change it
                      //newscript is not copied and the structure does not take
                      //the ownership of newscript.
                      //Thus, newscript must outlive the BLOB_CHOICE structure.
                      //The rationale is that the script is obtained from
                      //unicharset that manage itself ownership of returned
                      //pointers.
                    const char* newscript) {
      blob_script = newscript;
    }

    static BLOB_CHOICE* deep_copy(const BLOB_CHOICE* src) {
      BLOB_CHOICE* choice = new BLOB_CHOICE;
      *choice = *src;
      return choice;
    }

    const char* const unichar() const {  //access function
      return blob_unichar;
    }
    float rating() const {  //access function
      return blob_rating;
    }
    float certainty() const {  //access function
      return blob_certainty;
    }
    inT8 config() const {  //access function
      return blob_config;
    }
    const char* script() const {  //access function
      return blob_script;
    }

    NEWDELETE private:
    char blob_unichar[UNICHAR_LEN + 1]; //unichar
    char blob_config;                   //char config (font)
    inT16 junk2;
    float blob_rating;                  //size related
    float blob_certainty;               //absolute
    const char* blob_script;
};

                                 //make them listable
ELISTIZEH (BLOB_CHOICE) CLISTIZEH (BLOB_CHOICE_LIST)
/* permuter codes used in WERD_CHOICEs */
#
#define MIN_PERM      1
#define NO_PERM       0
#define TOP_CHOICE_PERM  1
#define LOWER_CASE_PERM  2
#define UPPER_CASE_PERM  3
#define NUMBER_PERM      4
#define SYSTEM_DAWG_PERM 5
#define DOC_DAWG_PERM    6
#define USER_DAWG_PERM   7
#define FREQ_DAWG_PERM   8
#define COMPOUND_PERM    9
#define MAX_PERM      9
class
WERD_CHOICE
{
  public:
    WERD_CHOICE() {  //empty
      word_string = NULL;
      word_lengths = NULL;
      word_rating = 0.0;
      word_certainty = 0.0;
      word_permuter = 0;
      word_blob_choices = NULL;
    }
    WERD_CHOICE(                         //constructor
                const char *src_string,  //word string
                const char *src_lengths, //unichar lengths
                float src_rating,        //rating
                float src_cert,          //certainty
                uinT8 src_permuter);     //permuter code

    ~WERD_CHOICE();
                                 //access function
    const STRING &string() const {
      return word_string;
    }
                                 //access function
    const STRING &lengths() const {
      return word_lengths;
    }

    float rating() const {  //access function
      return word_rating;
    }
    float certainty() const {  //access function
      return word_certainty;
    }
    void set_certainty(  //change it
        float new_val) {
      word_certainty = new_val;
    }
    uinT8 permuter() const {  //access function
      return word_permuter;
    }
    void set_permuter(  //Override
                      uinT8 perm) {
      word_permuter = perm;
    }

    BLOB_CHOICE_LIST_CLIST* blob_choices() {
      return word_blob_choices;
    }

    void set_blob_choices(BLOB_CHOICE_LIST_CLIST *blob_choices);

    WERD_CHOICE & operator+= (   //concatanate
      const WERD_CHOICE & second);//second on first

    WERD_CHOICE& operator= (const WERD_CHOICE& source);

    NEWDELETE private:
    STRING word_string;          //text
    STRING word_lengths;         //unichar lengths for the string
    float word_rating;           //size related
    float word_certainty;        //absolute
    uinT8 word_permuter;         //permuter code
    BLOB_CHOICE_LIST_CLIST *word_blob_choices; //best choices for each blob

 private:

    void delete_blob_choices();
};

CLISTIZEH (WERD_CHOICE)
void print_ratings_list(                           //print whole list
                        const char *msg,           //intro message
                        BLOB_CHOICE_LIST *ratings  //list of results
                       );
void print_ratings_info(                           //print summary info
                        FILE *fp,                  //file to use
                        BLOB_CHOICE_LIST *ratings  //list of results
                       );
typedef void (*POLY_MATCHER) (PBLOB *, PBLOB *, PBLOB *, WERD *,
DENORM *, BLOB_CHOICE_LIST &);
typedef void (*POLY_TESTER) (PBLOB *, DENORM *, BOOL8, char *, inT32,
BLOB_CHOICE_LIST *);
#endif
