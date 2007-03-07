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

#include          "clst.h"
#include          "werd.h"
#include          "notdll.h"

class BLOB_CHOICE:public ELIST_LINK
{
  public:
    BLOB_CHOICE() {  //empty
    }
    BLOB_CHOICE(                   //constructor
                char src_class,    //character
                float src_rating,  //rating
                float src_cert,    //certainty
                INT8 src_config);  //config (font)

    void set_class(  //change it
                   char newchar) {
      blob_class = newchar;
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
                    INT8 newfont) {
      blob_config = newfont;
    }

    char char_class() const {  //access function
      return blob_class;
    }
    float rating() const {  //access function
      return blob_rating;
    }
    float certainty() const {  //access function
      return blob_certainty;
    }
    INT8 config() const {  //access function
      return blob_config;
    }

    NEWDELETE private:
    char blob_class;             //char code
    char blob_config;            //char config (font)
    INT16 junk2;
    float blob_rating;           //size related
    float blob_certainty;        //absolute
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
    }
    WERD_CHOICE(                         //constructor
                const char *src_string,  //word string
                float src_rating,        //rating
                float src_cert,          //certainty
                UINT8 src_permuter);     //permuter code

                                 //access function
    const STRING &string() const { 
      return word_string;
    }

    float rating() const {  //access function
      return word_rating;
    }
    float certainty() const {  //access function
      return word_certainty;
    }
    UINT8 permuter() const {  //access function
      return word_permuter;
    }
    void set_permuter(  //Override
                      UINT8 perm) {
      word_permuter = perm;
    }

    WERD_CHOICE & operator+= (   //concatanate
      const WERD_CHOICE & second);//second on first

    NEWDELETE private:
    STRING word_string;          //text
    float word_rating;           //size related
    float word_certainty;        //absolute
    UINT8 word_permuter;         //permuter code
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
typedef void (*POLY_TESTER) (PBLOB *, DENORM *, BOOL8, char *, INT32,
BLOB_CHOICE_LIST *);
#endif
