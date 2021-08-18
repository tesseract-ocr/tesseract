///////////////////////////////////////////////////////////////////////
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "image.h"

#include <allheaders.h>

namespace tesseract {

Image Image::clone() const {
  return pix_ ? pixClone(pix_) : nullptr;
}

Image Image::copy() const {
  return pixCopy(nullptr, pix_);
}

void Image::destroy() {
  pixDestroy(&pix_);
}

bool Image::isZero() const {
  l_int32 r = 0;
  pixZero(pix_, &r);
  return r == 1;
}

Image Image::operator|(Image i) const {
  return pixOr(nullptr, pix_, i);
}

Image &Image::operator|=(Image i) {
  pixOr(pix_, pix_, i);
  return *this;
}

Image Image::operator&(Image i) const {
  return pixAnd(nullptr, pix_, i);
}

Image &Image::operator&=(Image i) {
  pixAnd(pix_, pix_, i);
  return *this;
}

}
