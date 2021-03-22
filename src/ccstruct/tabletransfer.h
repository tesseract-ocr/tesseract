/******************************************************************************
 * File:        tabletransfer.h
 * Description: Infrastructure for the transfer of table detection results
 * Author:      Stefan Brechtken
 *
 * (C) Copyright 2021, Stefan Brechtken
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
 ****************************************************************************/

#ifndef TESSERACT_CCSTRUCT_TABLETRANSFER_H_
#define TESSERACT_CCSTRUCT_TABLETRANSFER_H_
#include <memory>
#include <vector>
#include "rect.h"

namespace tesseract {

/// Structure for data transfer from table detector
struct TessTable {
  tesseract::TBOX box;
  std::vector<tesseract::TBOX> rows;
  std::vector<tesseract::TBOX> cols;
};

/** \brief You can use this small template function to ensure that one and
 *   only one object of type T exists. It implements the Singleton Pattern.
 *
 * T must be default-constructable.
 * Usage examples:
 *   A& a = uniqueInstance<A>();
 *   a.xyz();
 *   uniqueInstance<A>(make_unique<A>(42)); // replace instance
 *   a.foo();
 * or
 *   uniqueInstance<A>().xyz();
 */
template<typename T>
T& uniqueInstance(std::unique_ptr<T> new_instance = nullptr)
{
  static std::unique_ptr<T> _instance = std::make_unique<T>();

  if (new_instance) {
    _instance = std::move(new_instance);
  }

  return *_instance.get();
}

/// return const version of \see uniqueInstance
template<typename T>
const T& constUniqueInstance(std::unique_ptr<T> new_instance = nullptr)
{
  return uniqueInstance<T>(std::move(new_instance));
}

} // namespace tesseract

#endif  // TESSERACT_CCSTRUCT_TABLETRANSFER_H_
