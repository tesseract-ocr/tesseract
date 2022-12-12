///////////////////////////////////////////////////////////////////////
// File:        object_cache.h
// Description: A string indexed object cache.
// Author:      David Eger
//
// (C) Copyright 2012, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_CCUTIL_OBJECT_CACHE_H_
#define TESSERACT_CCUTIL_OBJECT_CACHE_H_

#include <functional> // for std::function
#include <mutex>      // for std::mutex
#include <string>
#include <vector>     // for std::vector
#include "ccutil.h"
#include "errcode.h"

namespace tesseract {

// A simple object cache which maps a string to an object of type T.
// Usually, these are expensive objects that are loaded from disk.
// Reference counting is performed, so every Get() needs to be followed later
// by a Free().  Actual deletion is accomplished by DeleteUnusedObjects().
template <typename T>
class ObjectCache {
public:
  ObjectCache() = default;
  ~ObjectCache() {
    std::lock_guard<std::mutex> guard(mu_);
    for (auto &it : cache_) {
      if (it.count > 0) {
        tprintf(
            "ObjectCache(%p)::~ObjectCache(): WARNING! LEAK! object %p "
            "still has count %d (id %s)\n",
            static_cast<void *>(this), static_cast<void *>(it.object),
            it.count, it.id.c_str());
      } else {
        delete it.object;
        it.object = nullptr;
      }
    }
  }

  // Return a pointer to the object identified by id.
  // If we haven't yet loaded the object, use loader to load it.
  // If loader fails to load it, record a nullptr entry in the cache
  // and return nullptr -- further attempts to load will fail (even
  // with a different loader) until DeleteUnusedObjects() is called.
  // We delete the given loader.
  T *Get(const std::string &id, std::function<T *()> loader) {
    T *retval = nullptr;
    std::lock_guard<std::mutex> guard(mu_);
    for (auto &it : cache_) {
      if (id == it.id) {
        retval = it.object;
        if (it.object != nullptr) {
          it.count++;
        }
        return retval;
      }
    }
    cache_.push_back(ReferenceCount());
    ReferenceCount &rc = cache_.back();
    rc.id = id;
    retval = rc.object = loader();
    rc.count = (retval != nullptr) ? 1 : 0;
    return retval;
  }

  // Decrement the count for t.
  // Return whether we knew about the given pointer.
  bool Free(T *t) {
    if (t == nullptr) {
      return false;
    }
    std::lock_guard<std::mutex> guard(mu_);
    for (auto &it : cache_) {
      if (it.object == t) {
        --it.count;
        return true;
      }
    }
    return false;
  }

  void DeleteUnusedObjects() {
    std::lock_guard<std::mutex> guard(mu_);
    cache_.erase(std::remove_if(cache_.begin(), cache_.end(),
                                [](const ReferenceCount &it) {
                                  if (it.count <= 0) {
                                    delete it.object;
                                    return true;
                                  } else {
                                    return false;
                                  }
                                }),
                 cache_.end());
  }

private:
  struct ReferenceCount {
    std::string id; // A unique ID to identify the object (think path on disk)
    T *object;      // A copy of the object in memory.  Can be delete'd.
    int count;      // A count of the number of active users of this object.
  };

  std::mutex mu_;
  std::vector<ReferenceCount> cache_;
};

} // namespace tesseract

#endif // TESSERACT_CCUTIL_OBJECT_CACHE_H_
