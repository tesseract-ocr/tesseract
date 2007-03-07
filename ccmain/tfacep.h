/**********************************************************************
 * File:        tfacep.h  (Formerly tfacep.h)
 * Description: Declarations of C functions and C owned data.
 * Author:					Ray Smith
 * Created:					Mon Apr 27 12:51:28 BST 1992
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

#ifndef           TFACEP_H
#define           TFACEP_H

#include          "hosthplb.h"
#include          "tessclas.h"
#include          "tessarray.h"
#include          "tstruct.h"
#include          "notdll.h"
#include "choices.h"
#include "oldlist.h"
#include "hyphen.h"
#include "tface.h"
#include "permute.h"
#include "adaptmatch.h"
#include "blobclass.h"
#include "stopper.h"
#include "associate.h"
#include "chop.h"
#include "expandblob.h"
#include "tordvars.h"
#include "metrics.h"
#include "tface.h"
#include "badwords.h"
#include "structures.h"

#define BLOB_MATCHING_ON
typedef void (*TESS_TESTER) (TBLOB *, BOOL8, char *, INT32, LIST);
typedef LIST (*TESS_MATCHER) (TBLOB *, TBLOB *, TBLOB *, void *, TEXTROW *);

extern "C"
{
  /*
  int							start_recog(				//Real main in C
  int							argc,
  char						*argv[]);
  void						program_editup2(			//afterforking part
  int							argc,
  char**						argv);

  int							end_recog(					//Real main in C
  int							argc,
  char						*argv[]);
  void						set_interactive_pass();
  void						set_pass1();
  void						set_pass2();
  //ARRAY						cc_recog(TWERD*,TESS_CHOICE*,TESS_CHOICE*,TESS_TESTER,
  //										TESS_TESTER);*/
  //void                                          wo_learn_blob(TBLOB*,TEXTROW*,char*,INT32);
  //LIST                                  AdaptiveClassifier(TBLOB*,TBLOB*,TEXTROW*);
  //void                                          LearnBlob(TBLOB*,TEXTROW*,char*,INT32);
  //TWERD                                         *newword();
  //TBLOB                                         *newblob();
  //TESSLINE                                      *newoutline();
  //EDGEPT                                                *newedgept();
  //void                                          oldedgept(EDGEPT*);
  //void                                          destroy_nodes(void*,void (*)(void*));
  //TESS_LIST                                     *append_choice(TESS_LIST*,char*,double,double,char);
  //void                                          fix_quotes (char*);
  //void                                          record_certainty(double,int);
  //int                                                   AcceptableResult(A_CHOICE*,A_CHOICE*);
  //int                                                   AdaptableWord(TWERD*,const char*,const char*);
  //void                                          delete_word(TWERD*);
  //void                                          free_blob(TBLOB*);
  //void                                          add_document_word(A_CHOICE*);
  //void                                          AdaptToWord(TWERD*,TEXTROW*,const char*,const char*,const char*);
  //void                                          SaveBadWord(const char*,double);
  //void                                          free_choice(TESS_CHOICE*);
  //TWERD                                         *newword();
  //TBLOB                                         *newblob();
  //void                                          free_blob(                                      //free a blob
  //      TBLOB                                           *blob);                                         //blob to free

  //int                                                   dict_word( const char* );

  //extern int                                    tess_cn_matching;
  //extern int                                    tess_bn_matching;
  //extern int                                    last_word_on_line;
  extern TEXTROW normalized_row;
  //extern TESS_MATCHER                   blob_matchers[];
  //extern FILE                                   *rawfile;
  //extern FILE                                   *textfile;
  //extern int                                    character_count;
  //extern int                                    word_count;
  //extern int                                    enable_assoc;
  //extern int                                    chop_enable;
  //extern int                                    permute_only_top;
  extern int display_ratings;

};

#if 0
#define strsave(s)    \
	((s) ?  \
	((char*) strcpy ((char*)alloc_string (strlen(s)+1), s))  :  \
	(NULL))
#endif

#define BOLD_ON				"&dB(s3B"
#define BOLD_OFF			"&d@(s0B"
#define UNDERLINE_ON		"&dD"
#define UNDERLINE_OFF		"&d@"
#endif
