// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the String class, which provides simple String 
// functionality to Helium. String is mainly used for reporting or debugging
// purposes, and is not meant to be a full-fledged string class, like it's
// C++ counterpart.
//
#ifndef HELIUM_STRINGUTILS_H__
#define HELIUM_STRINGUTILS_H__

#include "array.h"
#include "refcount.h"

namespace helium {

  // The String class uses an internal Array to store the actual string. This
  // Array is resized dynamically, making the String mutable even in size.
  // Note that this is a rather limited and simple implementation, and should
  // only be used for debugging / reporting or simple path parsing.
  class String : public ReferenceCounted {
    public:
      // Constructor to create the empty string.
      String();
      
      // Constructor to create an empty string with the given capacity.
      String(unsigned capacity);
      
      // Constructor to convert a C-String to a String.
      String(const char* string);
      
      // Deconstructor deallocates data, if it is no longer referenced.
      ~String();

      // Append the given string to the receiver.
      void Append(const char* string);
      
      // Append the given string to the receiver.
      void Append(const String& string);
      
      // Return the string that concatenates the given string to the receiver.
      // The receiver is not affected.
      String Concat(const char* string) const;
      
      // Return the string that concatenates the given string to the receiver.
      // The receiver is not affected.
      String Concat(const String& string) const;
      
      // Returns true, if the receiver contains the given substring.
      bool Contains(const char* substring) const;
      
      // Returns true, if the receiver contains the given substring.
      bool Contains(const String& substring) const;
      
      // Splits the string into words and adds them to the specified Array.
      // It is assumed that words are split by spaces.
      void GetWords(Array<String>& words) const;
      
      // Converts the receiver to a C-string. The output is not owned, so you
      // must deallocate it with delete[]!
      char* ToCString() const;
      
      // Obtain access to the internal C-string pointer. This data is owned
      // by the String!
      inline const char* CString() const {
        return string_->values();
      }
      
      // Deletes the string data.
      void DeleteData();
      
      // Working with file paths -----------------------------------------------
      // Returns a String that contains just the file name of the given path.
      static String Filename(const char* path);
      
      // Removes any trailing file extension of the form ".*".
      void RemoveExtension();
      
    protected:
      Array<char>* string_;
  };

} // namespace

#endif  // HELIUM_STRINGUTILS_H__
