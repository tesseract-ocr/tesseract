#include "hfst_io_utils.h"

#ifndef TEST_io_utils_cc

#include <cassert>

namespace hfst
{

void readsome(char * target, size_t len, std::istream * in)
{ 
  in->read(target, len); 
  
  if (in->fail())
    { throw InvalidRead(); }
}

void readsome(char * target, size_t len, _IO_FILE * file)
{ 
  size_t block_count = fread(target, len, 1, file); 

  if (block_count != 1)
    { throw InvalidRead(); }
}

std::string read_string(std::istream * in)
{ 
  if (in->eof() or in->fail() or in->bad())
    { throw InvalidStream(); }

  std::string str;

  std::getline(*in, str, '\0');

  return str;
}

std::string read_string(_IO_FILE * file)
{ 
  if (feof(file) or ferror(file))
    { throw InvalidStream(); }

  char target[MAX_STRING_BUFFER + 1];

  for (size_t i = 0; i < MAX_STRING_BUFFER; ++i)
    {
      target[i] = static_cast<char>(getc(file));

      if (target[i] == '\0')
	{ break; }
    }

  target[MAX_STRING_BUFFER] = '\0';

  std::string str(target);

  return str;
}

}

#else // TEST_io_utils_cc

int main(void)
{

}

#endif // TEST_io_utils_cc
