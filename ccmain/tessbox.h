/**********************************************************************
 * File:        tessbox.h  (Formerly tessbox.h)
 * Description: Black boxed Tess for developing a resaljet.
 * Author:					Ray Smith
 * Created:					Thu Apr 23 11:03:36 BST 1992
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

#ifndef           TESSBOX_H
#define           TESSBOX_H

#include          "ratngs.h"
#include          "notdll.h"

WERD_CHOICE *tess_segment_pass1(                           //recog one word
                                WERD *word,                //bln word to do
                                DENORM *denorm,            //de-normaliser
                                POLY_MATCHER matcher,      //matcher function
                                WERD_CHOICE *&raw_choice,  //raw result //list of blob lists
                                BLOB_CHOICE_LIST_CLIST *blob_choices,
                                WERD *&outword             //bln word output
                               );
WERD_CHOICE *tess_segment_pass2(                           //recog one word
                                WERD *word,                //bln word to do
                                DENORM *denorm,            //de-normaliser
                                POLY_MATCHER matcher,      //matcher function
                                WERD_CHOICE *&raw_choice,  //raw result //list of blob lists
                                BLOB_CHOICE_LIST_CLIST *blob_choices,
                                WERD *&outword             //bln word output
                               );
                                 //recog one word
WERD_CHOICE *correct_segment_pass2(WERD *word,                //bln word to do
                                   DENORM *denorm,            //de-normaliser
                                   POLY_MATCHER matcher,      //matcher function
                                   POLY_TESTER tester,        //tester function
                                   WERD_CHOICE *&raw_choice,  //raw result //list of blob lists
                                   BLOB_CHOICE_LIST_CLIST *blob_choices,
                                   WERD *&outword             //bln word output
                                  );
WERD_CHOICE *test_segment_pass2(                           //recog one word
                                WERD *word,                //bln word to do
                                DENORM *denorm,            //de-normaliser
                                POLY_MATCHER matcher,      //matcher function
                                POLY_TESTER tester,        //tester function
                                WERD_CHOICE *&raw_choice,  //raw result //list of blob lists
                                BLOB_CHOICE_LIST_CLIST *blob_choices,
                                WERD *&outword             //bln word output
                               );
BOOL8 tess_acceptable_word(                           //test acceptability
                           WERD_CHOICE *word_choice,  //after context
                           WERD_CHOICE *raw_choice    //before context
                          );
BOOL8 tess_adaptable_word(                           //test adaptability
                          WERD *word,                //word to test
                          WERD_CHOICE *word_choice,  //after context
                          WERD_CHOICE *raw_choice    //before context
                         );
void tess_cn_matcher(                           //call tess
                     PBLOB *pblob,              //previous blob
                     PBLOB *blob,               //blob to match
                     PBLOB *nblob,              //next blob
                     WERD *word,                //word it came from
                     DENORM *denorm,            //de-normaliser
                     BLOB_CHOICE_LIST &ratings  //list of results
                    );
void tess_bn_matcher(                           //call tess
                     PBLOB *pblob,              //previous blob
                     PBLOB *blob,               //blob to match
                     PBLOB *nblob,              //next blob
                     WERD *word,                //word it came from
                     DENORM *denorm,            //de-normaliser
                     BLOB_CHOICE_LIST &ratings  //list of results
                    );
void tess_default_matcher(                           //call tess
                          PBLOB *pblob,              //previous blob
                          PBLOB *blob,               //blob to match
                          PBLOB *nblob,              //next blob
                          WERD *word,                //word it came from
                          DENORM *denorm,            //de-normaliser
                          BLOB_CHOICE_LIST &ratings  //list of results
                         );
void tess_training_tester(                           //call tess
                          PBLOB *blob,               //blob to match
                          DENORM *denorm,            //de-normaliser
                          BOOL8 correct,             //ly segmented
                          char *text,                //correct text
                          inT32 count,               //chars in text
                          BLOB_CHOICE_LIST *ratings  //list of results
                         );
void tess_adapter(                         //adapt to word
                  WERD *word,              //bln word
                  DENORM *denorm,          //de-normalise
                  const WERD_CHOICE& choice,      //string for word
                  const WERD_CHOICE& raw_choice,  //before context
                  const char *rejmap);
void tess_add_doc_word(                          //test acceptability
                       WERD_CHOICE *word_choice  //after context
                      );
#endif
