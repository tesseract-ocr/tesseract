/**********************************************************************
 * File:        tessio.h  (Formerly tessread.h)
 * Description: Read/write Tesseract format row files.
 * Author:		Ray Smith
 * Created:		Wed Oct 09 15:02:46 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#ifndef           TESSIO_H
#define           TESSIO_H

#include          <stdio.h>
#include          "tessclas.h"
#include          "notdll.h"

/** 
 * open read & close
 * @param name file name
 * @param topright corner
 */
TEXTROW *get_tess_row_file(
                           const char *name,
                           TPOINT *topright
                          );
/** 
 * open read & close
 * @param name file name
 * @param topright corner
 */
TBLOB *get_tess_blob_file(
                          const char *name,
                          TPOINT *topright
                         );
/** 
 * read row file
 * @param gphfd file to read
 * @param count number expected
 * @param imagesize size of image
 */
TEXTROW *readrows(
                  int gphfd,
                  int count,
                  TPOINT *imagesize
                 );
/** 
 * read some words
 * @param gphfd file to read
 * @param count number expected
 * @param row row it comes from
 * @param imagesize size of image
 */
TWERD *readwords(
                 int gphfd,
                 int count,
                 TEXTROW *row,
                 TPOINT *imagesize
                );
/** 
 * read some blobs
 * @param gphfd file to read
 * @param count number expected
 * @param imagesize size of image
 */
TBLOB *readblobs(
                 int gphfd,
                 int count,
                 TPOINT *imagesize
                );
/** 
 * get a string
 * @param gphfd file to read
 * @param ratingspace size to read
 */
char *readratings(
                  int gphfd,
                  int ratingspace
                 );
/** 
 * read some outlines
 * @param gphfd file to read
 * @param outlines array of ptrs
 * @param outlinecount no to read
 */
void readoutlines(
                  int gphfd,
                  TESSLINE **outlines,
                  int outlinecount
                 );
/** 
 * read with testing
 * @param fd file to read
 * @param start buffer to write
 * @param size amount to write
 * @param checkeof give error on eof?
 */
int readgph(
            int fd,
            void *start,
            int size,
            int checkeof
           );
/** 
 * write a row
 * @param name file name
 * @param row row to write
 */
void write_row(
               FILE *name,
               TEXTROW *row
              );
/** 
 * write special row
 * @param name file name
 * @param row row to write
 * @param wordcount number of words to go
 */
void write_error_row(
                     FILE *name,
                     TEXTROW *row,
                     int wordcount
                    );
/** 
 * write special blob
 * @param name file name
 * @param blob blob to write
 * @param charlist true chars
 * @param charcount number of true chars
 */
void write_error_blob(
                      FILE *name,
                      TBLOB *blob,
                      char *charlist,
                      int charcount
                     );
/** 
 * write special word
 * @param name file name
 * @param word word to write
 * @param charlist true chars
 * @param charcount number of true chars
 */
void write_error_word(
                      FILE *name,
                      TWERD *word,
                      char *charlist,
                      int charcount
                     );
/** 
 * write a blob
 * @param name file to write
 * @param blob blob to write
 */
void writeblob(
               FILE *name,
               TBLOB *blob
              );
/** 
 * serialize
 * @param name file to write to
 * @param blob current blob
 * @param outline current outline
 * @param outlineno current serial no
 */
void serial_outlines(
                     FILE *name,
                     TBLOB *blob,
                     register TESSLINE *outline,
                     int *outlineno
                    );
/** 
 * count loopsize
 * @param vector vectors to count
 */
int countloop(
              register BYTEVEC *vector
             );
/** 
 * get serial no
 * @param outline start of search
 * @param target outline to find
 * @param serial serial no so far
 */
int outlineserial(
                  register TESSLINE *outline,
                  register TESSLINE *target,
                  int serial
                 );
/** 
 * Interface to fwrite 
 * @param name file to write
 * @param start buffer to write
 * @param size amount to write
 */
void writegph(
              FILE *name,
              void *start,
              int size
             );
#endif
