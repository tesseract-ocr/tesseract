// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// C includes
// NOTE: To be mobile compatible, we do not include C++ file I/O.
#include <stdio.h>
#include <ctype.h>

// Local includes
#include "debugging.h"
#include "helium_cluster.h"
#include "mathfunctions.h"
#include "stringutils.h"
#include "textareas.h"

// NOTE: Find other solution here!
#ifdef EMBEDDED
#include "scanutils.h"
#endif

namespace helium {

const int TextAreas::kMinAreaWidth = 0;
const int TextAreas::kMinAreaHeight = 0;
const int TextAreas::kMissingConf = -1;

// Returns a copy of text but replaces all \n with a space and removes starting
// and trailing spaces.  The caller must deallocat the buffer afterwards.
// Note: Is't not clear what is the right amount of "clean up" we should do
// before saving the string to a file.  Replacing newlines is a MUST due to
// the way we process .dat files, but stripping spaces on ends is not required.
static char* CopyToOneLine(const char* text) {
  int len = strlen(text);
  char* linetext = new char[len+1];
  char* wpt = linetext;
  const char* rpt = text;
  while (*rpt) {
    if (!isspace(*rpt)) break;
    rpt++;
  }
  while (*rpt) {
    *wpt++ = (*rpt == '\n') ? ' ' : *rpt;
    rpt++;
  }
  *wpt = '\0';
  while (--wpt >= linetext) {
    if (!isspace(*wpt)) break;
    *wpt = '\0';
  }
  return linetext;
}

TextAreas::TextAreas() : boxes_(4), text_(4), conf_(4) {
}

TextAreas::TextAreas(const TextAreas& areas) {
  this->Copy(areas);
}

TextAreas::TextAreas(const Array<Box>& boxes)
  : boxes_(boxes.size()), text_(boxes.size()), conf_(boxes.size()) {
  for (unsigned i = 0; i < boxes.size(); i++)
    if (IsValidArea(boxes.ValueAt(i)))
      AddArea(boxes.ValueAt(i), NULL);
}

TextAreas::TextAreas(const Array<Cluster*>& clusters)
  : boxes_(clusters.size()), text_(clusters.size()), conf_(clusters.size()) {
  for (unsigned i = 0; i < clusters.size(); i++) {
    Box box = clusters.ValueAt(i)->CalculateBounds();
    if (IsValidArea(box))
      AddArea(box, NULL);
  }
}

TextAreas::TextAreas(const Array<Box>& boxes, const Array<const char*>& text)
  : boxes_(boxes.size()), text_(text.size()), conf_(text.size()) {
  ASSERT(boxes.size() == text.size());
  for (unsigned i = 0; i < boxes.size(); i++) {
    if (IsValidArea(boxes.ValueAt(i))) {
      AddArea(boxes.ValueAt(i), text.ValueAt(i), kMissingConf);
    }
  }
}

TextAreas::TextAreas(const Array<Box>& boxes, const Array<const char*>& text,
                     const Array<int>& conf)
  : boxes_(boxes.size()), text_(text.size()), conf_(text.size()) {
  ASSERT(boxes.size() == text.size());
  ASSERT(boxes.size() == conf.size());
  for (unsigned i = 0; i < boxes.size(); i++) {
    if (IsValidArea(boxes.ValueAt(i))) {
      AddArea(boxes.ValueAt(i), text.ValueAt(i), conf.ValueAt(i));
    }
  }
}

void TextAreas::Clear() {
  for (unsigned i = 0; i < text_.size(); ++i) {
    delete[] text_.ValueAt(i);
    text_.ValueAt(i) = NULL;
  }
  boxes_.Clear();
  text_.Clear();
  conf_.Clear();
}

void TextAreas::Copy(const TextAreas& areas) {
  ASSERT(areas.boxes().size() == areas.text().size());
  ASSERT(areas.boxes().size() == areas.conf().size());
  Clear();
  for (unsigned i = 0; i < areas.boxes_.size(); i++)
    AddArea(areas.boxes_.ValueAt(i), areas.text_.ValueAt(i),
            areas.conf_.ValueAt(i));
}

bool TextAreas::IsValidArea(const Box& box) {
  return (box.width() >= kMinAreaWidth && box.height() >= kMinAreaHeight);
}

bool TextAreas::ReadDatFile(const char* path, int conf_thresh) {
  FILE* data_file = fopen(path, "r");
  if (!data_file) return false;
  if (conf_thresh < 0) conf_thresh = 0;

  // Get file size and restore file pointer to beginning
  fseek(data_file, 0, SEEK_END);
  int file_size = ftell(data_file);
  fseek(data_file, 0, SEEK_SET);

  const int kBufferLength = file_size + 1;
  char text[kBufferLength];
  int xmin = 0;
  int xmax = 0;
  int ymin = 0;
  int ymax = 0;
  const char* format_bounds = "%d %d %d %d %d\n";

  char buffer[kBufferLength];
  while (fgets(buffer, kBufferLength, data_file)) {
    // Find first and last position of '\"' character in line, and extract
    // everything in between them as the text associated with this textarea.
    char *left = strchr(buffer, '\"');
    char *right = strrchr(buffer, '\"');
    if (left < right - 1) {
      strncpy(text, left + 1, static_cast<int>(right - left - 1));
      text[right - left - 1] = '\0';
    } else {
      text[0] = '\0';
    }
    int conf = kMissingConf;
    // Parse the bounds
    int n = sscanf(right + 1, format_bounds, &xmin, &xmax, &ymin, &ymax, &conf);
    if (n < 4) {
      fclose(data_file);
      return false;  // Parse error
    }
    // Add bound and a new copy of the text string to respective arrays.
    Box text_bounds(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);
    if (IsValidArea(text_bounds)) {
      if (conf == kMissingConf)
        AddArea(text_bounds, text);  // no conf loaded, don't apply threshold
      else if (conf >= conf_thresh)
        AddArea(text_bounds, text, conf);  // has conf and conf > threshold
      // else: has conf and conf < threshold, text area is ignored
    }
  }
  fclose(data_file);
  return true;
}

void TextAreas::ScaleBoxes(float scale) {
  for (unsigned i = 0; i < boxes_.size(); i++)
    boxes_.ValueAt(i) = ScaleBox(boxes_.ValueAt(i), scale);
}

bool TextAreas::WriteDatFile(const char* path) const {
  FILE* data_file = fopen(path, "w");
  if (!data_file) return false;

  const char* area_format_noconf = "\"%s\" %d %d %d %d\n";
  const char* area_format = "\"%s\" %d %d %d %d %d\n";
  for (unsigned i = 0; i < boxes_.size(); i++) {
    const Box& cur_box = boxes_.ValueAt(i);
    int xmin = cur_box.left();
    int xmax = cur_box.right();
    int ymin = cur_box.top();
    int ymax = cur_box.bottom();
    char* text = CopyToOneLine(text_.ValueAt(i));
    int conf = conf_.ValueAt(i);
    if (conf == kMissingConf)
      fprintf(data_file, area_format_noconf, text, xmin, xmax, ymin, ymax);
    else
      fprintf(data_file, area_format, text, xmin, xmax, ymin, ymax, conf);
    delete [] text;
  }
  fclose(data_file);

  return true;
}

void TextAreas::AddArea(const Box& bounds, const char* text, const int conf) {
  ASSERT(boxes_.size() == text_.size());
  ASSERT(boxes_.size() == conf_.size());
  boxes_.Add(bounds);
  // Create a copy of this text string and store the pointer. The memory will be
  // freed in the destructor.
  int slen = (text) ? strlen(text) : 0;
  char *text_copy = new char[slen + 1];
  strncpy(text_copy, text, slen);
  text_copy[slen] = '\0';  // strncpy does not ensure terminating with null.
  text_.Add(text_copy);
  conf_.Add(conf);
}

void TextAreas::BoxCoverage(const TextAreas& groundtruth,
                            float& overlap,
                            float& hallucination) const {
  // Calculate primitives (unions and intersections)
  Array<Box> intersection(2);
  BoxSetsIntersection(groundtruth.boxes(), boxes(), intersection);

  unsigned intersect_area = Area(intersection);
  unsigned truth_area = Area(groundtruth.boxes());
  unsigned result_area = Area(boxes());
  unsigned super_area = result_area - intersect_area;

  // Calculate percentage of these areas
  overlap = (truth_area
    ?  static_cast<float>(intersect_area) / static_cast<float>(truth_area)
    :  0);

  hallucination = (result_area
    ?  static_cast<float>(super_area) / static_cast<float>(result_area)
    :  0);
}

void TextAreas::BoxHits(const TextAreas& groundtruth,
                        unsigned& hits,
                        unsigned& misses,
                        unsigned& false_positives) const {
  misses = 0;
  false_positives = 0;
  hits = 0;

  // Find hits & misses
  for (unsigned i = 0; i < groundtruth.boxes().size(); i++)
    if (IntersectsWith(groundtruth.boxes().ValueAt(i), boxes()))
      hits++;
    else
      misses++;

  // Find false positives
  for (unsigned i = 0; i < boxes().size(); i++)
    if (!IntersectsWith(boxes().ValueAt(i), groundtruth.boxes()))
      false_positives++;
}


// Returns the number of words in setA that are in words in setB.
static int ContainedWords(const Array<String>& words_A,
                          const Array<String>& words_B) {
  int hits = 0;
  for (unsigned i = 0; i < words_A.size(); ++i) {
    String cur_truth = words_A.ValueAt(i);
    bool found = false;
    for (unsigned j = 0; j < words_B.size(); ++j) {
      String cur_result = words_B.ValueAt(j);
      if (cur_result.Contains(cur_truth)) {
        found = true;
        break;
      }
    }
    if (found) hits++;
  }
  return hits;
}

static int Sum(const int *x, const int n) {
  int sum = 0;
  for (int i = 0; i < n; i++) sum += x[i];
  return sum;
}


// Split the text string in each area into a list of words, and return an
// array of lists.  Calling function should free up the memory.
static Array<String>* GetListOfWords(const TextAreas& areas) {
  Array<String>* list_words = new Array<String>[areas.boxes().size()];
  for (unsigned i = 0; i < areas.boxes().size(); i++) {
    Array<String>& words = list_words[i];
    String cur_str = areas.text().ValueAt(i);
    cur_str.GetWords(words);
  }
  return list_words;
}


// The original text evaluation scheme is strange.  The algorithm is as follows
//   break GT strings into words, test each is contained in result strings
//   if X of N GT words contained, found=X, miss=N-X
//   test if each result string is contained in GT words
//   if Y of M result strings contained, false_positive=M-Y
// A few things don't make sense
// (1) Ground truth is broken into words, but not result strings
// (2) missed words are double-counted as false_positive
// (3) multiple occurences of words in GT maybe over-counted
//
// The new algorithm is as follows.
//   Text comparison done at word bases
//   if a GT word is found in an overlapping box, it's a hit,
//   all other words in the result box are misses
//   the remaining result words are false positives
void TextAreas::TextHits(const TextAreas& groundtruth,
                         unsigned& hits,
                         unsigned& misses,
                         unsigned& false_positives) const {
  Array<String>* list_truth_words = GetListOfWords(groundtruth);
  Array<String>* list_result_words = GetListOfWords(*this);
  int* result_words_cnt = new int[boxes().size()];
  for (unsigned j = 0; j < boxes().size(); j++)
    result_words_cnt[j] = static_cast<int>(list_result_words[j].size());
  int n_result_words = Sum(result_words_cnt, boxes().size());

  hits = 0;
  misses = 0;
  false_positives = 0;
  for (unsigned i = 0; i < groundtruth.boxes().size(); i++) {
    const Array<String>& true_words = list_truth_words[i];
    int box_word_hits = 0;
    for (unsigned j = 0; j < boxes().size(); j++) {
      if (Intersection(groundtruth.boxes().ValueAt(i),
                       boxes().ValueAt(j)).Area() > 0) {
        result_words_cnt[j] = 0;
        const Array<String>& result_words = list_result_words[j];
        box_word_hits += ContainedWords(true_words, result_words);
      }
    }
    if (box_word_hits > true_words.size()) box_word_hits = true_words.size();
    hits += box_word_hits;
    misses += true_words.size() - box_word_hits;
  }
  false_positives = n_result_words - hits;
  delete [] result_words_cnt;
  delete [] list_truth_words;
  delete [] list_result_words;
}


bool TextAreas::IntersectsWith(const Box& box, const Array<Box>& box_list) {
  for (unsigned i = 0; i < box_list.size(); i++)
    if (Intersection(box, box_list.ValueAt(i)).Area() > 0)
      return true;
  return false;
}
}  // namespace
