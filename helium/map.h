// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file declares the Map class, and its base class MapBase. Map is used 
// in Helium for holding any type of two-dimensional data, such as bitmasks or
// images. Maps are reference counted, so they may be copied and used in Arrays.
//
#ifndef HELIUM_MAP_H__
#define HELIUM_MAP_H__

// Local includes
#include "debugging.h"
#include "point.h"
#include "refcount.h"
#include "types.h"

// C includes
#include <stddef.h>
#include <string.h>

namespace helium {

// The MapBase class provides access to the dimensions of the map. It is meant
// to provide a common interface to all methods that are not data dependant.
class MapBase : public ReferenceCounted {
  public:
    // Constructor. Initializes width and height to 0.
    MapBase() : width_(0), height_(0) {
    }
    
    // Constructor to initialize map with the specified dimenions.
    MapBase(unsigned width, unsigned height) : width_(width), height_(height) {
    }
    
    // Accessor to the width of the map.
    inline unsigned width() const {
      return width_;
    }
    
    // Accessor to the height of the map.
    inline unsigned height() const {
      return height_;
    }
    
    // Valid() returns true, if the width and height of the map are greater
    // than 0.
    inline bool Valid() const {
      return (width_ > 0) && (height_ > 0);
    }
    
  protected:
    unsigned width_, height_;
};

// The template class Map provides methods to create and modify two dimensional
// data of any size and element type. The idea is to provide simple pointer
// access to any portion of the data, so that algorithms can work with the
// data pointers directly.
// TODO: Provide methods for faster sequential access (on a word basis).
template<typename T>
class Map : public MapBase {
  public:
    // Constructor to create an empty map with no data.
    Map();
    
    // Constructor to allocate a map of the given width and height.
    Map(unsigned width, unsigned height);
    
    // Constructor to wrap existing data in a Map object.
    Map(unsigned width, unsigned height, T* data);
    
    // Deconstructor. Deletes when data is no longer referenced by any object.
    virtual ~Map();
    
    // Accessor to data.
    inline T* data() const {
      return data_;
    }
    
    // Accessor to set the data, and release the old data.
    inline void set_data(T* data) {
      Release();
      data_ = data;
    }

    // Reallocate the map size.
    void Reset(unsigned width, unsigned height);
    
    // Obtain a pointer to the specified coordinates. This method does bounds 
    // checking in only in debug mode!
    inline T* Access(const Point& p) const {
      ASSERT_IN_DEBUG_MODE(p.x >= 0 && p.x < width_ 
                           && p.y >= 0 && p.y < height_);
      return data_ + width_ * p.y + p.x;
    }
    
    // Obtain a pointer to the specified coordinates. This method does bounds 
    // checking in only in debug mode!
    inline T* Access(int x, int y) const {
      ASSERT_IN_DEBUG_MODE(p.x >= 0 && p.x < width_ 
                           && p.y >= 0 && p.y < height_);
      return data_ + width_ * y + x;
    }
    
    // Returns the value at the specified coordinates. This method does bounds 
    // checking in only in debug mode!
    inline T ValueAt(const Point& p) const {
      return *Access(p.x, p.y);
    }
    
    // Returns the value at the specified coordinates. This method does bounds 
    // checking in only in debug mode!
    inline T ValueAt(int x, int y) const {
      return *Access(x, y);
    }
    
    // Returns a pointer to the last value. This is useful as the upper limit,
    // when looping over the entire set of values.
    inline T* DataEnd() const {
      return Access(width_ - 1, height_ - 1);
    }
    
    // Returns true if the dimensions of the map are valid, and the data 
    // pointer is not NULL.
    inline bool Valid() const {
      return (data_ != NULL) && MapBase::Valid();
    }
    
    // Sets all the values in the map to 0.
    void Clear();
    
    // Releases the current data, and makes a copy of the specified Map. Use
    // this method sparingly, as it may require a large memcpy (Helium uses
    // this method only for debugging / reporting purposes).
    void Copy(const Map& other);
    
  protected:
    // Deletes the data. This is called when all objects referencing the data
    // have been released.
    void DeleteData();
    
    // The pointer to the data.
    T* data_;
};

// Template implementations ----------------------------------------------------
template<typename T>
Map<T>::Map() 
  : MapBase(), data_(NULL) {
}

template<typename T>
Map<T>::Map(unsigned width, unsigned height) 
  : MapBase(width, height), data_(NULL) {
  if (width * height > 0) data_ = new T[width * height];
}

template<typename T>
Map<T>::Map(unsigned width, unsigned height, T* data) 
  : MapBase(width, height), data_(data) {
}

template<typename T>
void Map<T>::Clear() {
  memset(data_, 0, width_ * height_ * sizeof(T));
}

template<typename T>
void Map<T>::Reset(unsigned width, unsigned height) {
  Release();
  width_ = width;
  height_ = height;
  data_ = new T[width_ * height_];
  Realloc();  // reset reference counter
}

template<typename T>
void Map<T>::Copy(const Map<T>& other) {
  Reset(other.width_, other.height_);
  memcpy(data_, other.data_, width_ * height_ * sizeof(T));
}

template<typename T>
Map<T>::~Map() {
  if (ShouldDelete()) DeleteData();
}

template<typename T>
void Map<T>::DeleteData() {
  delete[] data_;
  data_ = NULL;
  ReferenceCounted::DeleteData(); // Call Super
}

} // namespace

#endif  // HELIUM_MAP_H__
