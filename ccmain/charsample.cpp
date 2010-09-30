/**********************************************************************
 * File:        charsample.cpp  (Formerly charsample.c)
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

#include "mfcpch.h"

#include <stdio.h>
#include          <ctype.h>
#include          <math.h>
#ifdef __UNIX__
#include <assert.h>
#include          <unistd.h>
#endif
#include "memry.h"
#include          "tessvars.h"
#include "statistc.h"
#include          "charsample.h"
#include "paircmp.h"
#include "matmatch.h"
#include          "adaptions.h"
#include          "secname.h"
#include          "notdll.h"
#include          "tesseractclass.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

extern inT32 demo_word;          // Hack for demos

ELISTIZE (CHAR_SAMPLE) ELISTIZE (CHAR_SAMPLES) CHAR_SAMPLE::CHAR_SAMPLE () {
  sample_blob = NULL;
  sample_denorm = NULL;
  sample_image = NULL;
  ch = '\0';
  n_samples_matched = 0;
  total_match_scores = 0.0;
  sumsq_match_scores = 0.0;
}


CHAR_SAMPLE::CHAR_SAMPLE(PBLOB *blob, DENORM *denorm, char c) {
  sample_blob = blob;
  sample_denorm = denorm;
  sample_image = NULL;
  ch = c;
  n_samples_matched = 0;
  total_match_scores = 0.0;
  sumsq_match_scores = 0.0;
}


CHAR_SAMPLE::CHAR_SAMPLE(IMAGE *image, char c) {
  sample_blob = NULL;
  sample_denorm = NULL;
  sample_image = image;
  ch = c;
  n_samples_matched = 0;
  total_match_scores = 0.0;
  sumsq_match_scores = 0.0;
}


float CHAR_SAMPLE::match_sample(  // Update match scores
                                CHAR_SAMPLE *test_sample,
                                BOOL8 updating,
                                tesseract::Tesseract* tess) {
  float score1;
  float score2;
  IMAGE *image = test_sample->image ();

  if (sample_blob != NULL && test_sample->blob () != NULL) {
    PBLOB *blob = test_sample->blob ();
    DENORM *denorm = test_sample->denorm ();

    score1 = tess->compare_bln_blobs (sample_blob, sample_denorm, blob, denorm);
    score2 = tess->compare_bln_blobs (blob, denorm, sample_blob, sample_denorm);

    score1 = (score1 > score2) ? score1 : score2;
  }
  else if (sample_image != NULL && image != NULL) {
    CHAR_PROTO *sample = new CHAR_PROTO (this);

    score1 = matrix_match (sample_image, image);
    delete sample;
  }
  else
    return BAD_SCORE;

  if ((tessedit_use_best_sample || tessedit_cluster_debug) && updating) {
    n_samples_matched++;
    total_match_scores += score1;
    sumsq_match_scores += score1 * score1;
  }
  return score1;
}


double CHAR_SAMPLE::mean_score() {
  if (n_samples_matched > 0)
    return (total_match_scores / n_samples_matched);
  else
    return BAD_SCORE;
}


double CHAR_SAMPLE::variance() {
  double mean = mean_score ();

  if (n_samples_matched > 0) {
    return (sumsq_match_scores / n_samples_matched) - mean * mean;
  }
  else
    return BAD_SCORE;
}


void CHAR_SAMPLE::print(FILE *f) {
  if (!tessedit_cluster_debug)
    return;

  if (n_samples_matched > 0)
    fprintf (f,
      "%c - sample matched against " INT32FORMAT
      " blobs, mean: %f, var: %f\n", ch, n_samples_matched,
      mean_score (), variance ());
  else
    fprintf (f, "No matches for this sample (%c)\n", ch);
}


void CHAR_SAMPLE::reset_match_statistics() {
  n_samples_matched = 0;
  total_match_scores = 0.0;
  sumsq_match_scores = 0.0;
}


CHAR_SAMPLES::CHAR_SAMPLES() {
  type = UNKNOWN;
  samples.clear ();
  ch = '\0';
  best_sample = NULL;
  proto = NULL;
}


CHAR_SAMPLES::CHAR_SAMPLES(CHAR_SAMPLE *sample) {
  CHAR_SAMPLE_IT sample_it = &samples;

  ASSERT_HOST (sample->image () != NULL || sample->blob () != NULL);

  if (sample->image () != NULL)
    type = IMAGE_CLUSTER;
  else if (sample->blob () != NULL)
    type = BLOB_CLUSTER;

  samples.clear ();
  sample_it.add_to_end (sample);
  if (tessedit_mm_only_match_same_char)
    ch = sample->character ();
  else
    ch = '\0';
  best_sample = NULL;
  proto = NULL;
}


void CHAR_SAMPLES::add_sample(CHAR_SAMPLE *sample, tesseract::Tesseract* tess) {
  CHAR_SAMPLE_IT sample_it = &samples;

  if (tessedit_use_best_sample || tessedit_cluster_debug)
    for (sample_it.mark_cycle_pt ();
  !sample_it.cycled_list (); sample_it.forward ()) {
    sample_it.data ()->match_sample (sample, TRUE, tess);
    sample->match_sample (sample_it.data (), TRUE, tess);
  }

  sample_it.add_to_end (sample);

  if (tessedit_mm_use_prototypes && type == IMAGE_CLUSTER) {
    if (samples.length () == tessedit_mm_prototype_min_size)
      this->build_prototype ();
    else if (samples.length () > tessedit_mm_prototype_min_size)
      this->add_sample_to_prototype (sample);
  }
}


void CHAR_SAMPLES::add_sample_to_prototype(CHAR_SAMPLE *sample) {
  BOOL8 rebuild = FALSE;
  inT32 new_xsize = proto->x_size ();
  inT32 new_ysize = proto->y_size ();
  inT32 sample_xsize = sample->image ()->get_xsize ();
  inT32 sample_ysize = sample->image ()->get_ysize ();

  if (sample_xsize > new_xsize) {
    new_xsize = sample_xsize;
    rebuild = TRUE;
  }
  if (sample_ysize > new_ysize) {
    new_ysize = sample_ysize;
    rebuild = TRUE;
  }

  if (rebuild)
    proto->enlarge_prototype (new_xsize, new_ysize);

  proto->add_sample (sample);
}


void CHAR_SAMPLES::build_prototype() {
  CHAR_SAMPLE_IT sample_it = &samples;
  CHAR_SAMPLE *sample;
  inT32 proto_xsize = 0;
  inT32 proto_ysize = 0;

  if (type != IMAGE_CLUSTER
    || samples.length () < tessedit_mm_prototype_min_size)
    return;

  for (sample_it.mark_cycle_pt ();
  !sample_it.cycled_list (); sample_it.forward ()) {
    sample = sample_it.data ();
    if (sample->image ()->get_xsize () > proto_xsize)
      proto_xsize = sample->image ()->get_xsize ();
    if (sample->image ()->get_ysize () > proto_ysize)
      proto_ysize = sample->image ()->get_ysize ();
  }

  proto = new CHAR_PROTO (proto_xsize, proto_ysize, 0, 0, '\0');

  for (sample_it.mark_cycle_pt ();
    !sample_it.cycled_list (); sample_it.forward ())
  this->add_sample_to_prototype (sample_it.data ());

}


void CHAR_SAMPLES::find_best_sample() {
  CHAR_SAMPLE_IT sample_it = &samples;
  double score;
  double best_score = MAX_INT32;

  if (ch == '\0' || samples.length () < tessedit_mm_prototype_min_size)
    return;

  for (sample_it.mark_cycle_pt ();
  !sample_it.cycled_list (); sample_it.forward ()) {
    score = sample_it.data ()->mean_score ();
    if (score < best_score) {
      best_score = score;
      best_sample = sample_it.data ();
    }
  }
  #ifndef SECURE_NAMES
  if (tessedit_cluster_debug) {
    tprintf ("Best sample for this %c cluster:\n", ch);
    best_sample->print (debug_fp);
  }
  #endif
}


float CHAR_SAMPLES::match_score(CHAR_SAMPLE *sample,
                                tesseract::Tesseract* tess) {
  if (tessedit_mm_only_match_same_char && sample->character () != ch)
    return BAD_SCORE;

  if (tessedit_use_best_sample && best_sample != NULL)
    return best_sample->match_sample (sample, FALSE, tess);
  else if ((tessedit_mm_use_prototypes
    || tessedit_mm_adapt_using_prototypes) && proto != NULL)
    return proto->match_sample (sample);
  else
    return this->nn_match_score (sample, tess);
}


float CHAR_SAMPLES::nn_match_score(CHAR_SAMPLE *sample,
                                   tesseract::Tesseract* tess) {
  CHAR_SAMPLE_IT sample_it = &samples;
  float score;
  float min_score = MAX_INT32;

  for (sample_it.mark_cycle_pt ();
  !sample_it.cycled_list (); sample_it.forward ()) {
    score = sample_it.data ()->match_sample (sample, FALSE, tess);
    if (score < min_score)
      min_score = score;
  }

  return min_score;
}


void CHAR_SAMPLES::assign_to_char() {
  STATS char_frequency(FIRST_CHAR, LAST_CHAR);
  CHAR_SAMPLE_IT sample_it = &samples;
  inT32 i;
  inT32 max_index = 0;
  inT32 max_freq = 0;

  if (samples.length () == 0 || tessedit_mm_only_match_same_char)
    return;

  for (sample_it.mark_cycle_pt ();
    !sample_it.cycled_list (); sample_it.forward ())
  char_frequency.add ((inT32) sample_it.data ()->character (), 1);

  for (i = FIRST_CHAR; i <= LAST_CHAR; i++)
  if (char_frequency.pile_count (i) > max_freq) {
    max_index = i;
    max_freq = char_frequency.pile_count (i);
  }

  if (samples.length () >= tessedit_cluster_min_size
    && max_freq > samples.length () * tessedit_cluster_accept_fraction)
    ch = (char) max_index;
}


void CHAR_SAMPLES::print(FILE *f) {
  CHAR_SAMPLE_IT sample_it = &samples;

  fprintf (f, "Collected " INT32FORMAT " samples\n", samples.length ());

  #ifndef SECURE_NAMES
  if (tessedit_cluster_debug)
    for (sample_it.mark_cycle_pt ();
    !sample_it.cycled_list (); sample_it.forward ())
  sample_it.data ()->print (f);

  if (ch == '\0')
    fprintf (f, "\nCluster not used for adaption\n");
  else
    fprintf (f, "\nCluster used to adapt to '%c's\n", ch);
  #endif
}


CHAR_PROTO::CHAR_PROTO() {
  xsize = 0;
  ysize = 0;
  ch = '\0';
  nsamples = 0;
  proto_data = NULL;
  proto = NULL;
}


CHAR_PROTO::CHAR_PROTO(inT32 x_size,
                       inT32 y_size,
                       inT32 n_samples,
                       float initial_value,
                       char c) {
  inT32 x;
  inT32 y;

  xsize = x_size;
  ysize = y_size;
  ch = c;
  nsamples = n_samples;

  ALLOC_2D_ARRAY(xsize, ysize, proto_data, proto, float);

  for (y = 0; y < ysize; y++)
    for (x = 0; x < xsize; x++)
      proto[x][y] = initial_value;
}


CHAR_PROTO::CHAR_PROTO(CHAR_SAMPLE *sample) {
  inT32 x;
  inT32 y;
  IMAGELINE imline_s;

  if (sample->image () == NULL) {
    xsize = 0;
    ysize = 0;
    ch = '\0';
    nsamples = 0;
    proto_data = NULL;
    proto = NULL;
  }
  else {
    ch = sample->character ();
    xsize = sample->image ()->get_xsize ();
    ysize = sample->image ()->get_ysize ();
    nsamples = 1;

    ALLOC_2D_ARRAY(xsize, ysize, proto_data, proto, float);

    for (y = 0; y < ysize; y++) {
      sample->image ()->fast_get_line (0, y, xsize, &imline_s);
      for (x = 0; x < xsize; x++)
        if (imline_s.pixels[x] == BINIM_WHITE)
          proto[x][y] = 1.0;
      else
        proto[x][y] = -1.0;
    }
  }
}


CHAR_PROTO::~CHAR_PROTO () {
  if (proto_data != NULL)
    FREE_2D_ARRAY(proto_data, proto);
}


float CHAR_PROTO::match_sample(CHAR_SAMPLE *test_sample) {
  CHAR_PROTO *test_proto;
  float score;

  if (test_sample->image () != NULL) {
    test_proto = new CHAR_PROTO (test_sample);
    if (xsize > test_proto->x_size ())
      score = this->match (test_proto);
    else {
      demo_word = -demo_word;    // Flag different call
      score = test_proto->match (this);
    }
  }
  else
    return BAD_SCORE;

  delete test_proto;

  return score;
}


float CHAR_PROTO::match(CHAR_PROTO *test_proto) {
  inT32 xsize2 = test_proto->x_size ();
  inT32 y_size;
  inT32 y_size2;
  inT32 x_offset;
  inT32 y_offset;
  inT32 x;
  inT32 y;
  CHAR_PROTO *match_proto;
  float score;
  float sum = 0.0;

  ASSERT_HOST (xsize >= xsize2);

  x_offset = (xsize - xsize2) / 2;

  if (ysize < test_proto->y_size ()) {
    y_size = test_proto->y_size ();
    y_size2 = ysize;
    y_offset = (y_size - y_size2) / 2;

    match_proto = new CHAR_PROTO (xsize,
      y_size,
      nsamples * test_proto->n_samples (),
      0, '\0');

    for (y = 0; y < y_offset; y++) {
      for (x = 0; x < xsize2; x++) {
        match_proto->data ()[x + x_offset][y] =
          test_proto->data ()[x][y] * nsamples;
        sum += match_proto->data ()[x + x_offset][y];
      }
    }

    for (y = y_offset + y_size2; y < y_size; y++) {
      for (x = 0; x < xsize2; x++) {
        match_proto->data ()[x + x_offset][y] =
          test_proto->data ()[x][y] * nsamples;
        sum += match_proto->data ()[x + x_offset][y];
      }
    }

    for (y = y_offset; y < y_offset + y_size2; y++) {
      for (x = 0; x < x_offset; x++) {
        match_proto->data ()[x][y] = proto[x][y - y_offset] *
          test_proto->n_samples ();
        sum += match_proto->data ()[x][y];
      }

      for (x = x_offset + xsize2; x < xsize; x++) {
        match_proto->data ()[x][y] = proto[x][y - y_offset] *
          test_proto->n_samples ();
        sum += match_proto->data ()[x][y];
      }

      for (x = x_offset; x < x_offset + xsize2; x++) {
        match_proto->data ()[x][y] =
          proto[x][y - y_offset] * test_proto->data ()[x - x_offset][y];
        sum += match_proto->data ()[x][y];
      }
    }
  }
  else {
    y_size = ysize;
    y_size2 = test_proto->y_size ();
    y_offset = (y_size - y_size2) / 2;

    match_proto = new CHAR_PROTO (xsize,
      y_size,
      nsamples * test_proto->n_samples (),
      0, '\0');

    for (y = 0; y < y_offset; y++)
    for (x = 0; x < xsize; x++) {
      match_proto->data ()[x][y] =
        proto[x][y] * test_proto->n_samples ();
      sum += match_proto->data ()[x][y];
    }

    for (y = y_offset + y_size2; y < y_size; y++)
    for (x = 0; x < xsize; x++) {
      match_proto->data ()[x][y] =
        proto[x][y] * test_proto->n_samples ();
      sum += match_proto->data ()[x][y];
    }

    for (y = y_offset; y < y_offset + y_size2; y++) {
      for (x = 0; x < x_offset; x++) {
        match_proto->data ()[x][y] =
          proto[x][y] * test_proto->n_samples ();
        sum += match_proto->data ()[x][y];
      }

      for (x = x_offset + xsize2; x < xsize; x++) {
        match_proto->data ()[x][y] =
          proto[x][y] * test_proto->n_samples ();
        sum += match_proto->data ()[x][y];
      }

      for (x = x_offset; x < x_offset + xsize2; x++) {
        match_proto->data ()[x][y] = proto[x][y] *
          test_proto->data ()[x - x_offset][y - y_offset];
        sum += match_proto->data ()[x][y];
      }
    }
  }

  score = (1.0 - sum /
    (xsize * y_size * nsamples * test_proto->n_samples ()));

  if (tessedit_mm_debug) {
    if (score < 0) {
      tprintf ("Match score %f\n", score);
      tprintf ("x: %d, y: %d, ns: %d, nt: %d, dx %d, dy: %d\n",
        xsize, y_size, nsamples, test_proto->n_samples (),
        x_offset, y_offset);
      for (y = 0; y < y_size; y++) {
        tprintf ("\n%d", y);
        for (x = 0; x < xsize; x++)
          tprintf ("\t%d", match_proto->data ()[x][y]);

      }
      tprintf ("\n");
      fflush(debug_fp);
    }
  }

#ifndef GRAPHICS_DISABLED
  if (tessedit_display_mm) {
    tprintf ("Match score %f\n", score);
    display_images (this->make_image (),
      test_proto->make_image (), match_proto->make_image ());
  }
  else if (demo_word != 0) {
    if (demo_word > 0)
      display_image (test_proto->make_image (), "Test sample",
        300, 400, FALSE);
    else
      display_image (this->make_image (), "Test sample", 300, 400, FALSE);

    display_image (match_proto->make_image (), "Best match",
      700, 400, TRUE);
  }
#endif

  delete match_proto;

  return score;
}


void CHAR_PROTO::enlarge_prototype(inT32 new_xsize, inT32 new_ysize) {
  float *old_proto_data = proto_data;
  float **old_proto = proto;
  inT32 old_xsize = xsize;
  inT32 old_ysize = ysize;
  inT32 x_offset;
  inT32 y_offset;
  inT32 x;
  inT32 y;

  ASSERT_HOST (new_xsize >= xsize && new_ysize >= ysize);

  xsize = new_xsize;
  ysize = new_ysize;
  ALLOC_2D_ARRAY(xsize, ysize, proto_data, proto, float);
  x_offset = (xsize - old_xsize) / 2;
  y_offset = (ysize - old_ysize) / 2;

  for (y = 0; y < y_offset; y++)
    for (x = 0; x < xsize; x++)
      proto[x][y] = nsamples;

  for (y = y_offset + old_ysize; y < ysize; y++)
    for (x = 0; x < xsize; x++)
      proto[x][y] = nsamples;

  for (y = y_offset; y < y_offset + old_ysize; y++) {
    for (x = 0; x < x_offset; x++)
      proto[x][y] = nsamples;

    for (x = x_offset + old_xsize; x < xsize; x++)
      proto[x][y] = nsamples;

    for (x = x_offset; x < x_offset + old_xsize; x++)
      proto[x][y] = old_proto[x - x_offset][y - y_offset];
  }

  FREE_2D_ARRAY(old_proto_data, old_proto);
}


void CHAR_PROTO::add_sample(CHAR_SAMPLE *sample) {
  inT32 x_offset;
  inT32 y_offset;
  inT32 x;
  inT32 y;
  IMAGELINE imline_s;
  inT32 sample_xsize = sample->image ()->get_xsize ();
  inT32 sample_ysize = sample->image ()->get_ysize ();

  x_offset = (xsize - sample_xsize) / 2;
  y_offset = (ysize - sample_ysize) / 2;

  ASSERT_HOST (x_offset >= 0 && y_offset >= 0);

  for (y = 0; y < y_offset; y++)
    for (x = 0; x < xsize; x++)
      proto[x][y]++;             // Treat pixels outside the
  // range as white
  for (y = y_offset + sample_ysize; y < ysize; y++)
    for (x = 0; x < xsize; x++)
      proto[x][y]++;

  for (y = y_offset; y < y_offset + sample_ysize; y++) {
    sample->image ()->fast_get_line (0,
      y - y_offset, sample_xsize, &imline_s);
    for (x = x_offset; x < x_offset + sample_xsize; x++) {
      if (imline_s.pixels[x - x_offset] == BINIM_WHITE)
        proto[x][y]++;
      else
        proto[x][y]--;
    }

    for (x = 0; x < x_offset; x++)
      proto[x][y]++;

    for (x = x_offset + sample_xsize; x < xsize; x++)
      proto[x][y]++;
  }

  nsamples++;
}


IMAGE *CHAR_PROTO::make_image() {
  IMAGE *image;
  IMAGELINE imline_p;
  inT32 x;
  inT32 y;

  ASSERT_HOST (nsamples != 0);

  image = new (IMAGE);
  image->create (xsize, ysize, 8);

  for (y = 0; y < ysize; y++) {
    image->fast_get_line (0, y, xsize, &imline_p);

    for (x = 0; x < xsize; x++) {
      imline_p.pixels[x] = 128 +
        (uinT8) ((proto[x][y] * 128.0) / (0.00001 + nsamples));
    }

    image->fast_put_line (0, y, xsize, &imline_p);
  }
  return image;
}
