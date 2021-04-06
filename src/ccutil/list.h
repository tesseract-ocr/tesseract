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

template <typename CONTAINER, typename ITERATOR_TYPE, typename CLASSNAME>
class X_LIST : public CONTAINER {
public:
  X_LIST() = default;
  X_LIST(const X_LIST &) = delete;
  X_LIST &operator=(const X_LIST &) = delete;
  ~X_LIST() {
    clear();
  }

  /* delete elements */
  void clear() {
    CONTAINER::internal_clear([](void *link) {delete reinterpret_cast<CLASSNAME *>(link);});
  }

  /* Become a deep copy of src_list */
  template <typename U>
  void deep_copy(const U *src_list, CLASSNAME *(*copier)(const CLASSNAME *)) {
    X_ITER<ITERATOR_TYPE, CLASSNAME> from_it(const_cast<U *>(src_list));
    X_ITER<ITERATOR_TYPE, CLASSNAME> to_it(this);

    for (from_it.mark_cycle_pt(); !from_it.cycled_list(); from_it.forward())
      to_it.add_after_then_move((*copier)(from_it.data()));
  }
};

} // namespace tesseract

#endif
