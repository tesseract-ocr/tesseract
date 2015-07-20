#include "hfst_ol_driver.h"

#ifndef TEST_hfst_ol_driver_cc

#include <cstdlib>
#include <cassert>

size_t len(const std::string &str)
{ return str.size(); }

#else // TEST_hfst_ol_driver_cc

#include <fstream>
#include <cassert>
#include <string>
#include <sstream>
#include <cmath>

int main(int argc, char * argv[])
{
  // A simple fst:
  // 0	1	a	a	0.000000
  // 0	1	b	b	0.500000
  // 1	0.500000

  
  // The fst in binary olw format:
  char olw_fst_data[265] = 
    { 
       72,   70,   83,   84,    0,   87,    0,    0,  118,  101,  114,  115, 
      105,  111,  110,    0,   51,   46,   51,    0,  116,  121,  112,  101, 
        0,   72,   70,   83,   84,   95,   79,   76,   87,    0,  102,  111, 
      114,  109,  117,  108,   97,  105,   99,   45,  100,  101,  102,  105, 
      110,  105,  116,  105,  111,  110,    0,   73,  100,   32,   84,   32, 
      116,    0,  110,   97,  109,  101,    0,   99,  111,  110,  118,  101, 
      114,  116,   40,  116,  101,  120,  116,   40,  116,  101,  115,  116, 
       95,   97,   95,   97,   46,  116,  120,  116,   41,   41,    0,    3, 
        0,    3,    0,    7,    0,    0,    0,    4,    0,    0,    0,    0, 
        0,    0,    0,    0,    0,    0,    0,    1,    0,    0,    0,    1, 
        0,    0,    0,    1,    0,    0,    0,    1,    0,    0,    0,    0, 
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 
        0,    0,    0,    0,    0,    0,    0,   64,   95,   69,   80,   83, 
       73,   76,   79,   78,   95,   83,   89,   77,   66,   79,   76,   95, 
       64,    0,   97,    0,   98,    0,   -1,   -1,   -1,   -1,   -1,   -1, 
       -1,   -1,   -1,   -1,   -1,   -1,    1,    0,    0,    0,    0, -128, 
        2,    0,    1,    0,    0, -128,   -1,   -1,   -1,   -1,   -1,   -1, 
       -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 
        1,    0,    1,    0,    2,    0,    0, -128,    0,    0,    0,    0, 
        2,    0,    2,    0,    2,    0,    0, -128,    0,    0,    0,   63, 
       -1,   -1,   -1,   -1,    1,    0,    0,    0,    0,    0,    0,   63, 
       -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    0,    0, -128,   79, 
       79
    }; 

  std::string olw_fst_data_string(olw_fst_data, 265);

  if (argc == 2)
    {
      std::ifstream in(argv[1]); 
      FILE * file = fopen(argv[1], "r");

      OLFst<std::ifstream> stream_olfst(&in);
      OLFst<FILE>          file_olfst(file);      
      
      assert(not stream_olfst.broken);
      assert(not file_olfst.broken);

      const char * koira = "koira";
      const char * xerox = "Xerox";

      unsigned int state = FST_START;

      while (*koira != '\0')
	{
	  unsigned short symbol_index = 
	    file_olfst.symbol_index(std::string(1, *koira++));

	  unsigned int transition = 
	    file_olfst.get_transition_start_index(state, symbol_index);

	  assert(file_olfst.get_input_symbol(transition) == symbol_index);

	  state = file_olfst.get_target_index(transition);
	}

      assert(file_olfst.is_final_state(state));

      state = FST_START;

      while (*xerox != '\0')
	{
	  unsigned short symbol_index = 
	    file_olfst.symbol_index(std::string(1, *xerox++));
	  
	  unsigned int transition = 
	    file_olfst.get_transition_start_index(state, symbol_index);

	  assert(file_olfst.get_input_symbol(transition) == symbol_index);

	  state = file_olfst.get_target_index(transition);
	}

      assert(file_olfst.is_final_state(state));

      state = FST_START;

      koira = "koira";
      xerox = "Xerox";

      while (*koira != '\0')
	{
	  unsigned short symbol_index = 
	    stream_olfst.symbol_index(std::string(1, *koira++));

	  unsigned int transition = 
	    stream_olfst.get_transition_start_index(state, symbol_index);

	  assert(stream_olfst.get_input_symbol(transition) == symbol_index);

	  state = stream_olfst.get_target_index(transition);
	  
	}

      assert(stream_olfst.is_final_state(state));

      for (int i = 0; i < 10000000; ++i)
	{
	  koira = "koirakoira";

	  state = FST_START;
	  
	  while (*koira != '\0')
	    {
	      unsigned short symbol_index = 
		stream_olfst.symbol_index(std::string(1, *koira++));
	      
	      unsigned int transition = 
		stream_olfst.get_transition_start_index(state, symbol_index);
	      
	      assert(stream_olfst.get_input_symbol(transition) == 
		     symbol_index);
	      
	      state = stream_olfst.get_target_index(transition);
	      
	    }
	  
	  assert(stream_olfst.is_final_state(state));
	}

      fclose(file);
    }
  else 
    {
      std::istringstream in(olw_fst_data_string);
      OLFst<std::istream> olfst(&in);

      assert(not olfst.broken);
      
      unsigned short a = olfst.symbol_index("a");
      unsigned short b = olfst.symbol_index("b");

      unsigned int start = FST_START;
      assert(not olfst.is_final_state(start));

      unsigned int tr_index = olfst.get_transition_start_index(start, a);

      assert(tr_index != static_cast<unsigned int>(NOINDEX));
      assert(olfst.get_input_symbol(tr_index) == a);
      assert(olfst.get_output_symbol(tr_index) == a);
      assert(olfst.get_weight(tr_index) < 0.001);

      unsigned int state = olfst.get_target_index(tr_index);

      assert(olfst.is_final_state(state));
      assert(fabs(olfst.get_final_weight(state) - 0.5) < 0.001);

      tr_index = olfst.get_transition_start_index(start, b);

      assert(tr_index != static_cast<unsigned int>(NOINDEX));
      assert(olfst.get_input_symbol(tr_index) == b);
      assert(olfst.get_output_symbol(tr_index) == b);
      assert(fabs(olfst.get_weight(tr_index) - 0.5) < 0.001);

      state = olfst.get_target_index(tr_index);

      assert(olfst.is_final_state(state));
      assert(fabs(olfst.get_final_weight(state) - 0.5) < 0.001);
    }
}

#endif // TEST_hfst_ol_driver_cc
