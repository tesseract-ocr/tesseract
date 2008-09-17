// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

#include <stdio.h>
#include "stringutils.h"

using namespace helium;

String::String() : ReferenceCounted(), string_(NULL) {
  string_ = new Array<char>(255);
  string_->Add('\0');
}

String::String(unsigned capacity) : ReferenceCounted(), string_(NULL) {
  string_ = new Array<char>(capacity);
  string_->Add('\0');
}

String::String(const char* string) : ReferenceCounted(), string_(NULL) {
  string_ = new Array<char>(strlen(string));
  string_->Add('\0');
  Append(string);
}

String::~String() {
  if( ShouldDelete() ) DeleteData();
}

void String::Append(const char* string) {
  string_->RemoveLast(1);  // Remove trailing 0-char
  for (unsigned i = 0; i < strlen(string); i++) string_->Add(string[i]);
  string_->Add('\0');
}

void String::Append(const String& string) {
  Append(string.CString());
}

String String::Concat(const char* string) const {
  String out(strlen(string) + string_->size());
  out.Append(CString());
  out.Append(string);
  return out;
}

String String::Concat(const String& string) const {
  return Concat(string.CString());
}

bool String::Contains(const char* other) const {
  const char* my_string = string_->values();
  unsigned j = 0;
  for (unsigned i = 0; my_string[i]; ++i) {
    if (!other[j]) return true;
    if (my_string[i] == other[j])
      j++;
    else
      j = 0;
  }
  if (!other[j]) return true;
  return false;
}

bool String::Contains(const String& other) const {
  return Contains(other.CString());
}

void String::GetWords(Array<String>& words) const {
  String cur_word;
  cur_word.string_->RemoveLast(1);

  for (unsigned i = 0; i < string_->size(); i++) {
    if (string_->ValueAt(i) != ' ' && string_->ValueAt(i) != '\0') {
      cur_word.string_->Add(string_->ValueAt(i));
    } else if (cur_word.string_->size() > 0) {
      cur_word.string_->Add('\0');
      words.Add(cur_word);
      cur_word = String("");
      cur_word.string_->RemoveLast(1);
    }
  }
  if (cur_word.string_->size() > 0) {
    cur_word.string_->Add('\0');
    words.Add(cur_word);
  }
}

char* String::ToCString() const {
  // Copy string
  char* out = new char[string_->size()];
  memcpy(out, string_->values(), string_->size());

  return out;
}

void String::DeleteData() {
  delete string_;
  ReferenceCounted::DeleteData();  // Call Super
}

String String::Filename(const char* path) {
  String out;
  int last_slash = -1;

  for (int i = 0; path[i]; ++i) if (path[i] == '/') last_slash = i;

  out.string_->RemoveLast(1);  // Remove trailing 0-char
  for (unsigned j = last_slash + 1; path[j]; ++j)
    out.string_->Add(path[j]);
  out.string_->Add('\0');

  return out;
}

void String::RemoveExtension() {
  unsigned remove = 0;
  for (int i = string_->size() - 1; i >= 0; --i) {
    remove++;
    if (string_->ValueAt(i) == '.') break;
  }
  if (remove < string_->size()) {
    string_->RemoveLast(remove);
    string_->Add('\0');
  }
}
