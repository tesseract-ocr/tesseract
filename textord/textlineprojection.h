// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TESSERACT_TEXTORD_TEXTLINEPROJECTION_H_
#define TESSERACT_TEXTORD_TEXTLINEPROJECTION_H_

#include "blobgrid.h"      // For BlobGrid

class DENORM;
struct Pix;
struct TPOINT;

namespace tesseract {

class ColPartition;

// Simple class to encapsulate the computation of an image representing
// local textline density, and function(s) to make use of it.
// The underlying principle is that if you smear connected components
// horizontally (vertically for components on a vertically written textline)
// and count the number of smeared components in an image, then the resulting
// image shows the density of the textlines at each image position.
class TextlineProjection {
 public:
  // The down-scaling factor is computed to obtain a projection resolution
  // of about 100 dpi, whatever the input.
  explicit TextlineProjection(int resolution);
  ~TextlineProjection();

  // Build the projection profile given the input_block containing lists of
  // blobs, a rotation to convert to image coords,
  // and a full-resolution nontext_map, marking out areas to avoid.
  // During construction, we have the following assumptions:
  // The rotation is a multiple of 90 degrees, ie no deskew yet.
  // The blobs have had their left and right rules set to also limit
  // the range of projection.
  void ConstructProjection(TO_BLOCK* input_block,
                           const FCOORD& rotation, Pix* nontext_map);

  // Display the blobs in the window colored according to textline quality.
  void PlotGradedBlobs(BLOBNBOX_LIST* blobs, ScrollView* win);

  // Moves blobs that look like they don't sit well on a textline from the
  // input blobs list to the output small_blobs list.
  // This gets them away from initial textline finding to stop diacritics
  // from forming incorrect textlines. (Introduced mainly to fix Thai.)
  void MoveNonTextlineBlobs(BLOBNBOX_LIST* blobs,
                            BLOBNBOX_LIST* small_blobs) const;

  // Create a window and display the projection in it.
  void DisplayProjection() const;

  // Compute the distance of the box from the partition using curved projection
  // space. As DistanceOfBoxFromBox, except that the direction is taken from
  // the ColPartition and the median bounds of the ColPartition are used as
  // the to_box.
  int DistanceOfBoxFromPartition(const TBOX& box, const ColPartition& part,
                                 const DENORM* denorm, bool debug) const;

  // Compute the distance from the from_box to the to_box using curved
  // projection space. Separation that involves a decrease in projection
  // density (moving from the from_box to the to_box) is weighted more heavily
  // than constant density, and an increase is weighted less.
  // If horizontal_textline is true, then curved space is used vertically,
  // as for a diacritic on the edge of a textline.
  // The projection uses original image coords, so denorm is used to get
  // back to the image coords from box/part space.
  int DistanceOfBoxFromBox(const TBOX& from_box, const TBOX& to_box,
                           bool horizontal_textline,
                           const DENORM* denorm, bool debug) const;

  // Compute the distance between (x, y1) and (x, y2) using the rule that
  // a decrease in textline density is weighted more heavily than an increase.
  // The coordinates are in source image space, ie processed by any denorm
  // already, but not yet scaled by scale_factor_.
  // Going from the outside of a textline to the inside should measure much
  // less distance than going from the inside of a textline to the outside.
  int VerticalDistance(bool debug, int x, int y1, int y2) const;

  // Compute the distance between (x1, y) and (x2, y) using the rule that
  // a decrease in textline density is weighted more heavily than an increase.
  int HorizontalDistance(bool debug, int x1, int x2, int y) const;

  // Returns true if the blob appears to be outside of a horizontal textline.
  // Such blobs are potentially diacritics (even if large in Thai) and should
  // be kept away from initial textline finding.
  bool BoxOutOfHTextline(const TBOX& box, const DENORM* denorm,
                        bool debug) const;

  // Evaluates the textlineiness of a ColPartition. Uses EvaluateBox below,
  // but uses the median top/bottom for horizontal and median left/right for
  // vertical instead of the bounding box edges.
  // Evaluates for both horizontal and vertical and returns the best result,
  // with a positive value for horizontal and a negative value for vertical.
  int EvaluateColPartition(const ColPartition& part, const DENORM* denorm,
                           bool debug) const;

  // Computes the mean projection gradients over the horizontal and vertical
  // edges of the box:
  //   -h-h-h-h-h-h
  //  |------------| mean=htop   -v|+v--------+v|-v
  //  |+h+h+h+h+h+h|             -v|+v        +v|-v
  //  |            |             -v|+v        +v|-v
  //  |    box     |             -v|+v  box   +v|-v
  //  |            |             -v|+v        +v|-v
  //  |+h+h+h+h+h+h|             -v|+v        +v|-v
  //  |------------| mean=hbot   -v|+v--------+v|-v
  //   -h-h-h-h-h-h
  //                           mean=vleft  mean=vright
  //
  // Returns MAX(htop,hbot) - MAX(vleft,vright), which is a positive number
  // for a horizontal textline, a negative number for a vertical textline,
  // and near zero for undecided. Undecided is most likely non-text.
  int EvaluateBox(const TBOX& box, const DENORM* denorm, bool debug) const;

 private:
  // Internal version of EvaluateBox returns the unclipped gradients as well
  // as the result of EvaluateBox.
  // hgrad1 and hgrad2 are the gradients for the horizontal textline.
  int EvaluateBoxInternal(const TBOX& box, const DENORM* denorm, bool debug,
                          int* hgrad1, int* hgrad2,
                          int* vgrad1, int* vgrad2) const;

  // Helper returns the mean gradient value for the horizontal row at the given
  // y, (in the external coordinates) by subtracting the mean of the transformed
  // row 2 pixels above from the mean of the transformed row 2 pixels below.
  // This gives a positive value for a good top edge and negative for bottom.
  // Returns the best result out of +2/-2, +3/-1, +1/-3 pixels from the edge.
  int BestMeanGradientInRow(const DENORM* denorm, int16_t min_x, int16_t max_x,
                            int16_t y, bool best_is_max) const;

  // Helper returns the mean gradient value for the vertical column at the
  // given x, (in the external coordinates) by subtracting the mean of the
  // transformed column 2 pixels left from the mean of the transformed column
  // 2 pixels to the right.
  // This gives a positive value for a good left edge and negative for right.
  // Returns the best result out of +2/-2, +3/-1, +1/-3 pixels from the edge.
  int BestMeanGradientInColumn(const DENORM* denorm, int16_t x, int16_t min_y,
                               int16_t max_y, bool best_is_max) const;

  // Helper returns the mean pixel value over the line between the start_pt and
  // end_pt (inclusive), but shifted perpendicular to the line in the projection
  // image by offset pixels. For simplicity, it is assumed that the vector is
  // either nearly horizontal or nearly vertical. It works on skewed textlines!
  // The end points are in external coordinates, and will be denormalized with
  // the denorm if not nullptr before further conversion to pix coordinates.
  // After all the conversions, the offset is added to the direction
  // perpendicular to the line direction. The offset is thus in projection image
  // coordinates, which allows the caller to get a guaranteed displacement
  // between pixels used to calculate gradients.
  int MeanPixelsInLineSegment(const DENORM* denorm, int offset,
                              TPOINT start_pt, TPOINT end_pt) const;

  // Helper function to add 1 to a rectangle in source image coords to the
  // internal projection pix_.
  void IncrementRectangle8Bit(const TBOX& box);
  // Inserts a list of blobs into the projection.
  // Rotation is a multiple of 90 degrees to get from blob coords to
  // nontext_map coords, image_box is the bounds of the nontext_map.
  // Blobs are spread horizontally or vertically according to their internal
  // flags, but the spreading is truncated by set pixels in the nontext_map
  // and also by the horizontal rule line limits on the blobs.
  void ProjectBlobs(BLOBNBOX_LIST* blobs, const FCOORD& rotation,
                    const TBOX& image_box, Pix* nontext_map);
  // Pads the bounding box of the given blob according to whether it is on
  // a horizontal or vertical text line, taking into account tab-stops near
  // the blob. Returns true if padding was in the horizontal direction.
  bool PadBlobBox(BLOBNBOX* blob, TBOX* bbox);

  // Helper denormalizes the TPOINT with the denorm if not nullptr, then
  // converts to pix_ coordinates.
  void TransformToPixCoords(const DENORM* denorm, TPOINT* pt) const;

  // Helper truncates the TPOINT to be within the pix_.
  void TruncateToImageBounds(TPOINT* pt) const;

  // Transform tesseract coordinates to coordinates used in the pix.
  int ImageXToProjectionX(int x) const;
  int ImageYToProjectionY(int y) const;

  // The down-sampling scale factor used in building the image.
  int scale_factor_;
  // The blob coordinates of the top-left (origin of the pix_) in tesseract
  // coordinates. Used to transform the bottom-up tesseract coordinates to
  // the top-down coordinates of the pix.
  int x_origin_;
  int y_origin_;
  // The image of horizontally smeared blob boxes summed to provide a
  // textline density map. As with a horizontal projection, the map has
  // dips in the gaps between textlines.
  Pix* pix_;
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_TEXTLINEPROJECTION_H_
