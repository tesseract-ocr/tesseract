// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the TextAreas class, which is used to store information
// on where and what text is located within an image. It can read and write
// "dat" files, which are outputted by the Osmium text detector, and by the
// ground-truthing QA system.
// TextAreas are used as the common datastructure for passing information
// of detected of ground-truthed text on an image, and provide functionality
// for reporting the accuracy of detected text areas and given ground-truth.
//
#ifndef HELIUM_TEXTAREAS_H__
#define HELIUM_TEXTAREAS_H__

#include "array.h"
#include "box.h"
#include "clusterer.h"

namespace helium {

// The TextAreas class combines bounding boxes and associated text. It contains
// two member Arrays, one for each. As it is not always possible to specify
// text with the boxes, it is possible to add boxes only. However it is never
// allowed to mix adding only a box, and adding box with text. In other words:
// Either the Box Array is of the same size as the text Array, or the size
// of the text Array is zero.
class TextAreas {
 public:
  // Any box given to constructor whose size is smaller will be ignored.
  static const int kMinAreaWidth;
  static const int kMinAreaHeight;
  static const int kMissingConf;

  // Constructor for a TextAreas object with no boxes or text.
  TextAreas();

  // Deep copy to duplicate text strings.
  explicit TextAreas(const TextAreas& areas);

  // Constructor for a TextAreas object, with the specified Array of Boxes.
  // Note that you may not add any areas with text information after using
  // this constructor!
  explicit TextAreas(const Array<Box>& boxes);

  // Constructor for a TextAreas object, with the specified boxes and the
  // text for each of these boxes. Both Arrays must be of the same size.
  // Note: This function allocates memory for a new copy of each text string
  // and is freed later in the destructor.
  TextAreas(const Array<Box>& boxes, const Array<const char*>& text);
  TextAreas(const Array<Box>& boxes, const Array<const char*>& text,
            const Array<int>& confs);

  // Constructor for creating a TextAreas object from the given ClusterArray.
  // Note, that since Clusters do not provide any textual information, you
  // may not add any areas with text information after using  this
  // constructor!
  explicit TextAreas(const ClusterArray& clusters);

  // Destructor that calls Clear() first to delete memory allocated in text_
  // that would otherwise not be freed by the destructor of Array<const char*>
  ~TextAreas() {
    Clear();
  }

  // Determines if the given box can be a valid text area based on size.
  bool IsValidArea(const Box& box_area);

  // Clears the box and text Array, and deallocates the contained strings.
  void Clear();

  // Clears current object and makes a copy of the given input.  New memory
  // is allocated for the text strings so they can be destroyed independently.
  void Copy(const TextAreas& areas);

  // Read a dat file at the given path, that contains box and text
  // information. The file must be ASCII, and each line must be in the
  // following format:
  // "<text>" <left> <right> <top> <bottom> [conf]
  // Returns true if the file was found and could be parsed.
  // If there is an area confidence in the file, and the value is smaller
  // than conf_thresh, the area is ignored.
  bool ReadDatFile(const char* path, int conf_thresh);
  bool ReadDatFile(const char* path) {
    return ReadDatFile(path, 0);
  }

  // Writes the text areas to the file at the given path. The file will be
  // in ASCII, and for each area a line of the following format is written:
  // "<text>" <left> <right> <top> <bottom>
  // where <text> is empty if there is no text information for the areas.
  // Returns true if the write was successful.
  bool WriteDatFile(const char* path) const;

  // Accessor to the Array of boxes.
  inline const Array<Box>& boxes() const {
    return boxes_;
  }
  // Accessor to the Array of text.
  inline const Array<const char*>& text() const {
    return text_;
  }
  inline const Array<int>& conf() const {
    return conf_;
  }

  // Returns the number of boxes.
  inline unsigned Size() const {
    return boxes_.size();
  }

  // Add a box and the associated text to the TextAreas object. This method
  // may only be called if the already added box information was supplied
  // with text as well (no mixed adding allowed).
  // Note: This function allocates memory for a new copy of the text string
  // and is freed later in the destructor.
  void AddArea(const Box& bounds, const char* text) {
    AddArea(bounds, text, kMissingConf);
  }
  void AddArea(const Box& bounds, const char* text, const int conf);

  // This method scales the text boxes by a given factor. This is used when
  // scaling the input image and reading associated ground-truth data.
  void ScaleBoxes(float scale);

  // This method, used for reporting, specifies how much of the ground-truth
  // box area is covered by the detected area (prescision), and how much of
  // the detected area overlaps with the ground-truth area (recall).
  void BoxCoverage(const TextAreas& groundtruth,
                   float& precision,
                   float& recall) const;

  // This method, used for reporting, specifies how many of the ground-
  // truth boxes were detected. A 'hit' is when a ground truthed box overlaps
  // with a detected box. A 'miss' is when a ground truthed box does not
  // overlap with a detected box. A 'false-positive' is when a detected box
  // does not overlap with a ground truthed box.
  void BoxHits(const TextAreas& groundtruth,
               unsigned& hits,
               unsigned& misses,
               unsigned& false_positives) const;

  // This method, used for reporting, specifies how many words of text were
  // detected. A 'hit' is when a ground truthed word was found in the
  // detected text. A 'miss' is when a ground truthed word was not found in
  // the detected text. A 'false positive' is when a detected word was not
  // found in the ground truth.
  void TextHits(const TextAreas& groundtruth,
                unsigned& hits,
                unsigned& misses,
                unsigned& false_positives) const;

 private:
  // Helper method that returns true, if the given Box overlaps with any of
  // the Boxes in the specified Array.
  static bool IntersectsWith(const Box& box, const Array<Box>& box_list);

  Array<Box> boxes_;
  Array<const char*> text_;
  Array<int> conf_;
};

} // namespace

#endif  // HELIUM_TEXTAREAS_H__
