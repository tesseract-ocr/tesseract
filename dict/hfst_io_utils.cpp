///////////////////////////////////////////////////////////////////////
// File:        hfst_io_utils.cpp
// Description: Useful functions for reading HFST optimized lookup fsts
// Author:      Miikka Silfverberg
//
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

  void readsome(char * target, size_t len, /*_IO_*/FILE * file)
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

std::string read_string(/*_IO_*/FILE * file)
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
