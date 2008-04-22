#ifndef TFACE_H
#define TFACE_H

#include "host.h"
#include "choicearr.h"
#include "tessclas.h"
#include "cutil.h"

/*----------------------------------------------------------------------------
          Function Prototypes
----------------------------------------------------------------------------*/
int start_recog(const char *configfile, const char *imagebase);

void program_editup(const char *configfile);

void program_editup2(const char *imagebase);

void edit_with_ocr(const char *imagename);

int end_recog();

void program_editdown(inT32 elasped_time);

void set_pass1();

void set_pass2();

CHOICES_LIST cc_recog(TWERD *tessword,
                      A_CHOICE *best_choice,
                      A_CHOICE *best_raw_choice,
                      BOOL8 tester,
                      BOOL8 trainer);

int dict_word(const char *word);
#endif
