/**********************************************************************
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

#ifndef LIST_ITERATOR_H
#define LIST_ITERATOR_H

#include <stdint.h>

namespace tesseract {

template <typename CONTAINER, typename CLASSNAME>
class X_ITER : public CONTAINER {
public:
  X_ITER() = default;
  template <typename U>
  X_ITER(U *list) : CONTAINER(list) {}

  CLASSNAME *data() {
    return static_cast<CLASSNAME *>(CONTAINER::data());
  }
  CLASSNAME *data_relative(int8_t offset) {
    return static_cast<CLASSNAME *>(CONTAINER::data_relative(offset));
  }
  CLASSNAME *forward() {
    return static_cast<CLASSNAME *>(CONTAINER::forward());
  }
  CLASSNAME *extract() {
    return static_cast<CLASSNAME *>(CONTAINER::extract());
  }
};

} // namespace tesseract

#endif
