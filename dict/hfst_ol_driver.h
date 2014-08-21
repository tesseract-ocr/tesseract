///////////////////////////////////////////////////////////////////////
// File:        hfst_ol_driver.h
// Description: Driver for HFST optimized lookup format.
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

#ifndef HEADER_hfst_ol_driver_h
#define HEADER_hfst_ol_driver_h

#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cassert>

#include "hfst_size_defs.h"
#include "hfst_io_utils.h"

#define FST_START 0

namespace hfst
{

typedef std::vector<std::string> StringVector;

size_t len(const std::string &str);

template <class IN> bool get_hfst_header(IN * in)
{
  std::string hfst_identifier = read_string(in);
  
  if (hfst_identifier != "HFST")
    { return false; }
  
  int header_size = read_short(in);

  if (read_char(in) != '\0')
    { return false; }

  char * header_rest = char_ptrize(malloc(header_size + 1));

  readsome(header_rest, header_size, in);
 
  // Replace EOS chars with zeros so that string conversion will
  // process the entire header.
  for (int i = 0; i < header_size; ++i)
    { header_rest[i] = (header_rest[i] == '\0' ? '0' : header_rest[i]); }
  
  header_rest[header_size] = '\0';

  std::string header_rest_str(header_rest);

  free(header_rest);

  // Valid olw headers need to contain a olw type specifier string
  // "type0HFST_OLW0"
  if (header_rest_str.find("type0HFST_OLW0") == std::string::npos)
    { return false; }

  return true;
}

template <class IN> struct OLHeader
{
  bool read_failed;

  int input_symbol_count;
  int total_symbol_count;
  int transition_index_count;
  int transition_count;
  int state_count;

  bool is_unweighted;
  bool is_deterministic;
  bool is_input_non_deterministic;
  bool is_minimized;
  bool is_acyclic;
  bool has_no_eps_eps_transitions;
  bool has_input_eps_transitions;
  bool has_input_eps_cycles;
  bool has_only_weighted_input_eps_cycles;

  OLHeader(IN * in):
  read_failed(false)
  {
    // If anything at all goes wrong, abort and set read_failed.
    try
      {
	if (not get_hfst_header(in))
	  { throw InvalidHeader(); }
	
	// Read numerical properties
	input_symbol_count     = read_short(in);
	total_symbol_count     = read_short(in);      
	transition_index_count = read_int(in);
	transition_count       = read_int(in);
	state_count            = read_int(in);
	
	// Read separator 0 between numerical and bool properties.
	int separator = read_int(in);
	
	if (separator != 0)
	  { throw InvalidHeader(); }
	
	// Read boolean properties
	is_unweighted                      = read_bool(in);
	is_deterministic                   = read_bool(in);
	is_input_non_deterministic         = read_bool(in);
	is_minimized                       = read_bool(in);
	is_acyclic                         = read_bool(in);
	has_no_eps_eps_transitions         = read_bool(in);
	has_input_eps_transitions          = read_bool(in);
	has_input_eps_cycles               = read_bool(in);
	has_only_weighted_input_eps_cycles = read_bool(in);
      }
    catch (...)
      { read_failed = true; } 
  }
};

template <class IN> struct OLAlphabet
{
  typedef std::unordered_map<std::string, size_t> SymbolIDMap;
  
  OLAlphabet(const OLHeader<IN> &header, 
	     IN * in):
  read_failed(false)
  {
    try
      {
	for (int i = 0; i < header.total_symbol_count; ++i)
	  { 
	    std::string symbol = read_string(in);
	    symbol_ids[symbol] = symbols.size();
	    
	    symbols.push_back(symbol);       
	  }
      }
    catch (...)
      { read_failed = true; }
  }

  bool         read_failed;  
  StringVector symbols;
  SymbolIDMap  symbol_ids;
};

template <class IN> class OLIArray
{
 public:
 bool read_failed;

 OLIArray(const OLHeader<IN> &header, 
	  IN * in):
 read_failed(false),
 entry_count(header.transition_index_count),
 entry_size(entry_count*TRINDEXSIZE)
 {
   size_t data_size = entry_count * TRINDEXSIZE;
   
   data = char_ptrize(malloc(data_size));
 
   try
     { readsome(data, data_size, in); }
   catch (...)
     { read_failed = true; }
 }
 
 ~OLIArray(void)
 { free(data); }
  
 size_t size(void) const
 { return entry_count; }

 unsigned short input_symbol(size_t index) const
 {
   unsigned short * s;
   
   s = reinterpret_cast<unsigned short *>
     (data + index + TRISYMBOLOFFSET);
   
   return *s;
 }

 unsigned int target_index(size_t index) const
 {
   int32_t * i;

   i = reinterpret_cast<int32_t *>
     (data + index + TRIINDEXOFFSET);
   
   return *i;
 }

 private:
 size_t entry_count;
 char * data;
 size_t entry_size;
};

template <class IN> class OLTArray
{
 public:
 bool read_failed;

 OLTArray(const OLHeader<IN> &header, 
	  IN * in):
 read_failed(false),
 entry_count(header.transition_count),
 entry_size(entry_count*TRSIZE)
 {
   size_t data_size = entry_count * TRSIZE;
   
   data = char_ptrize(malloc(data_size));
 
   try
     { readsome(data, data_size, in); }
   catch (...)
     { read_failed = true; }
 }

 ~OLTArray(void)
 { free(data); }

 size_t size(void) const
 { return entry_count; }
 
 unsigned short input_symbol(unsigned int index) const
 {
   if (index >= /*entry_count * TRSIZE*/entry_size)
     { return FINAL_SYMBOL; }

   unsigned short * s;
   
   s = reinterpret_cast<unsigned short *>
     (data + index + TRINSYMBOLOFFSET);
   
   return *s;
 }

 unsigned short output_symbol(unsigned int index) const
 {
   if (index >= /*entry_count * TRSIZE*/entry_size)
     { return FINAL_SYMBOL; }

   unsigned short * s;
   
   s = reinterpret_cast<unsigned short *>
     (data + index + TROUTSYMBOLOFFSET);
   
   return *s;
 }

 unsigned int target_index(unsigned int index) const
 {
   int32_t * s;
   
   s = reinterpret_cast<int32_t *>
     (data + index + TRSTATEOFFSET);
   
   return *s;
 }

 float weight(unsigned int index) const
 {
   float * w;
   
   w = reinterpret_cast<float *>
     (data + index + TRWEIGHTOFFSET);

   return *w;
 }

 private:
 size_t entry_count;
 char * data;
 size_t entry_size;
};

struct BrokenOLFst
{};

template <class IN> class OLFst
{
 public:

  bool broken;

  OLFst(IN * in):
  broken(0),
  header(0),
  alphabet(0),
  i_array(0),
  t_array(0)
  {
    header = new OLHeader<IN>(in);
    
    if (header->read_failed)
      { 
	broken = 1;
	return; 
      }
    
    alphabet = new OLAlphabet<IN>(*header, in);
    
    if (alphabet->read_failed)
      { 
	broken = 1;
	return; 
      }
    
    i_array = new OLIArray<IN>(*header, in);

    if (i_array->read_failed)
      { 
	broken = 1;
	return; 
      }

    t_array = new OLTArray<IN>(*header, in);

    if (t_array->read_failed)
      { 
	broken = 1;
	return; 
      }
  }

 OLFst(void):  
  broken(1),
  header(0),
  alphabet(0),
  i_array(0),
  t_array(0)
 {}

 ~OLFst(void)
 {
   delete header;
   delete alphabet;
   delete i_array;
   delete t_array;
 }

 unsigned int get_transition_start_index(unsigned int state_index, 
					 unsigned short input_symbol) const
 {
   if (broken)
     { throw BrokenOLFst(); }

   if (state_index < TRTABLEZERO)
     {
       unsigned int i_index = (state_index + input_symbol + 1) * TRINDEXSIZE;

       if (i_array->input_symbol(i_index) != input_symbol)
	 { return NOINDEX; }

       return i_array->target_index(i_index) - TRTABLEZERO;
     }
   else
     {
       unsigned int t_index = (state_index - TRTABLEZERO) * TRSIZE;

       if (t_array->input_symbol(t_index) == FINAL_SYMBOL)
	 { 
	   t_index     += TRSIZE; 
	   state_index += 1;
	 }

       if (t_array->input_symbol(t_index) != input_symbol)
	 { return NOINDEX; }

       return state_index;
     }
 }

 unsigned int get_target_index(unsigned int transition_index) const
 { 
   if (broken)
     { throw BrokenOLFst(); }

   return t_array->target_index(transition_index * TRSIZE); 
 }

 unsigned short get_input_symbol(unsigned int transition_index) const
 { 
   if (broken)
     { throw BrokenOLFst(); }

   return t_array->input_symbol(transition_index * TRSIZE); 
 }

 unsigned short get_output_symbol(unsigned int transition_index) const 
 { 
   if (broken)
     { throw BrokenOLFst(); }

   return t_array->output_symbol(transition_index * TRSIZE); 
 }

 float get_weight(unsigned int transition_index) const
 { 
   if (broken)
     { throw BrokenOLFst(); }

   return t_array->weight(transition_index * TRSIZE); 
 }

 unsigned int transition_array_size(void) const
 { return t_array->size(); }

 unsigned int is_final_state(unsigned int state_index) const 
 {
   if (broken)
     { throw BrokenOLFst(); }

   if (state_index < TRTABLEZERO)
     {
       unsigned int i_index = state_index * TRINDEXSIZE;

       return 
	 i_array->target_index(i_index) != static_cast<unsigned int>(NOINDEX);
     }
   else
     {
       unsigned int t_index = (state_index - TRTABLEZERO) * TRSIZE;

       if (t_array->input_symbol(t_index) == FINAL_SYMBOL)
	 { return t_array->target_index(t_index) != 
	     static_cast<unsigned int>(NOINDEX); }

       return false;
     }
 }

 float get_final_weight(unsigned int state_index) const 
 {
   if (broken)
     { throw BrokenOLFst(); }

   unsigned int t_index;

   if (state_index < TRTABLEZERO)
     {
       unsigned int i_index = state_index * TRINDEXSIZE;
       t_index = i_array->target_index(i_index);      
     }
   else
     { t_index = (state_index - TRTABLEZERO) * TRSIZE; }

   return t_array->weight(t_index);
 }
 
 size_t alphabet_size(void) const
 { 
   if (broken)
     { throw BrokenOLFst(); }

   return alphabet->symbols.size(); 
 }

 std::string string_symbol(size_t id) const
 { 
   if (broken)
     { throw BrokenOLFst(); }

   return alphabet->symbols[id]; 
 }
 
 unsigned short symbol_index(const std::string &symbol) const
 { 
   if (broken)
     { throw BrokenOLFst(); }

   return alphabet->symbol_ids[symbol]; 
 }

 private:
 OLHeader<IN>   * header;
 OLAlphabet<IN> * alphabet;
 OLIArray<IN>   * i_array;
 OLTArray<IN>   * t_array;
};

}

#endif // HEADER_hfst_ol_driver_h
