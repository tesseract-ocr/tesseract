/**********************************************************************
 * File:        charsample.h  (Formerly charsample.h)
 * Description: Class to contain character samples and match scores
 *					to be used for adaption
 * Author:      Chris Newton
 * Created:     Thu Oct  7 13:40:37 BST 1993
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#ifndef           CHARSAMPLE_H
#define           CHARSAMPLE_H

#include          "elst.h"
#include          "pageres.h"
#include          "memry.h"
#include          "notdll.h"

#define BAD_SCORE MAX_INT32
#define FIRST_CHAR '!'
#define LAST_CHAR  '~'

namespace tesseract {
  class Tesseract;  // Fwd decl.
}

enum ClusterType
{ UNKNOWN, BLOB_CLUSTER, IMAGE_CLUSTER };

class CHAR_SAMPLE;               //forward decl

ELISTIZEH (CHAR_SAMPLE)
class CHAR_SAMPLES;              //forward decl

ELISTIZEH (CHAR_SAMPLES)
class CHAR_PROTO;                //forward decl

class CHAR_SAMPLE:public ELIST_LINK
{
  public:
    CHAR_SAMPLE();  // empty constructor

    CHAR_SAMPLE(  // simple constructor
                PBLOB *blob,
                DENORM *denorm,
                char c
               );

    CHAR_SAMPLE(  // simple constructor
                IMAGE *image,
                char c
               );

    ~CHAR_SAMPLE () {
      // We own the image, so it has to be deleted.
      if (sample_image != NULL)
        delete sample_image;
    }

    float match_sample(CHAR_SAMPLE *test_sample, BOOL8 updating,
                       tesseract::Tesseract* tess);

    inT32 n_matches() {
      return n_samples_matched;
    }

    IMAGE *image() {
      return sample_image;
    }

    PBLOB *blob() {
      return sample_blob;
    }

    DENORM *denorm() {
      return sample_denorm;
    }

    double mean_score();

    double variance();

    char character() {
      return ch;
    }

    void print(FILE *f);

    void reset_match_statistics();

    NEWDELETE2 (CHAR_SAMPLE) private:
    IMAGE * sample_image;
    PBLOB *sample_blob;
    DENORM *sample_denorm;
    inT32 n_samples_matched;
    double total_match_scores;
    double sumsq_match_scores;
    char ch;
};

class CHAR_SAMPLES:public ELIST_LINK
{
  public:
    CHAR_SAMPLES();  //empty constructor

    CHAR_SAMPLES(CHAR_SAMPLE *sample);

    ~CHAR_SAMPLES () {           //destructor
    }

    inT32 n_samples() {
      return samples.length ();
    }

    void add_sample(CHAR_SAMPLE *sample, tesseract::Tesseract*);

    void build_prototype();

    void rebuild_prototype(inT32 new_xsize, inT32 new_ysize);

    void add_sample_to_prototype(CHAR_SAMPLE *sample);

    CHAR_PROTO *prototype() {
      return proto;
    }

    void find_best_sample();

    float match_score(CHAR_SAMPLE *sample, tesseract::Tesseract* tess);

    float nn_match_score(CHAR_SAMPLE *sample, tesseract::Tesseract* tess);

    char character() {
      return ch;
    }

    void assign_to_char();

    void print(FILE *f);

    NEWDELETE2 (CHAR_SAMPLES) private:
    ClusterType type;
    char ch;
    CHAR_PROTO *proto;
    CHAR_SAMPLE *best_sample;
    CHAR_SAMPLE_LIST samples;
};

class CHAR_PROTO
{
  public:
    CHAR_PROTO();  // empty constructor

    CHAR_PROTO(inT32 x_size,
               inT32 y_size,
               inT32 n_samples,
               float initial_value,
               char c);

    CHAR_PROTO(  // simple constructor
               CHAR_SAMPLE *sample);

    ~CHAR_PROTO ();

    float match_sample(CHAR_SAMPLE *test_sample);

    float match(CHAR_PROTO *test_proto);

    inT32 n_samples() {
      return nsamples;
    }

    inT32 x_size() {
      return xsize;
    }

    inT32 y_size() {
      return ysize;
    }

    float **data() {
      return proto;
    }
    char character() {
      return ch;
    }

    void enlarge_prototype(inT32 new_xsize, inT32 new_ysize);

    void add_sample(CHAR_SAMPLE *sample);

    IMAGE *make_image();

    void print(FILE *f);

    NEWDELETE2 (CHAR_PROTO) private:
    inT32 xsize;
    inT32 ysize;
    float *proto_data;
    float **proto;
    inT32 nsamples;
    char ch;
};
#endif
