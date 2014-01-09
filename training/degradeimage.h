/**********************************************************************
 * File:        degradeimage.h
 * Description: Function to degrade an image (usually of text) as if it
 *              has been printed and then scanned.
 * Authors:     Ray Smith
 * Created:     Tue Nov 19 2013
 *
 * (C) Copyright 2013, Google Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **********************************************************************/
#ifndef TESSERACT_TRAINING_DEGRADEIMAGE_H_
#define TESSERACT_TRAINING_DEGRADEIMAGE_H_

struct Pix;

namespace tesseract {

// Degrade the pix as if by a print/copy/scan cycle with exposure > 0
// corresponding to darkening on the copier and <0 lighter and 0 not copied.
// If rotation is not NULL, the clockwise rotation in radians is saved there.
// The input pix must be 8 bit grey. (Binary with values 0 and 255 is OK.)
// The input image is destroyed and a different image returned.
struct Pix* DegradeImage(struct Pix* input, int exposure, float* rotation);

}  // namespace tesseract

#endif  // TESSERACT_TRAINING_DEGRADEIMAGE_H_
