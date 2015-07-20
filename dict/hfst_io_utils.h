#ifndef HEADER_io_utils_h
#define HEADER_io_utils_h

#include <string>
#include <iostream>

#include "hfst_size_defs.h"

namespace hfst
{

struct InvalidRead
{};

struct InvalidBool
{};

struct InvalidHeader
{};

struct InvalidStream
{};

template <class T> char * char_ptrize(T * t)
{ return reinterpret_cast<char *>(t); }

void readsome(char * target, size_t len, std::istream * in);

void readsome(char * target, size_t len, FILE * file);

template <class T> T read_stuff(std::istream * in)
{
  if (in->eof() or in->fail() or in->bad())
    { throw InvalidStream(); }

  T t;

  readsome(char_ptrize(&t), sizeof(t), in);

  return t;
}

template <class T> T read_stuff(FILE * file)
{
  if (feof(file) or ferror(file))
    { throw InvalidStream(); }

  T t;

  readsome(char_ptrize(&t), sizeof(t), file);

  return t;
}

std::string read_string(std::istream * in);

std::string read_string(FILE * file);

template <class IN> char read_char(IN * in)
{ return read_stuff<int8_t>(in); }

template <class IN> int read_int(IN * in)
{ return read_stuff<int32_t>(in); }

template <class IN> short read_short(IN * in)
{ return read_stuff<int16_t>(in); }

template <class IN> bool read_bool(IN * in)
{
  int i = read_int(in);

  if (i == 1)
    { return true; }
  else if (i == 0)
    { return false; }
  else
    { throw InvalidBool(); }
}

}
#endif // HEADER_io_utils_h
