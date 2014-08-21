///////////////////////////////////////////////////////////////////////
// File:        hfst_word_model.cpp
// Description: Specialization of Dawg for HFST optimized lookup fsts
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

#include "hfst_word_model.h"

#ifndef TEST_hfst_word_model_cc

#include <iostream>
#include <queue>

#include "unichar.h"

using namespace::hfst;

namespace tesseract
{

bool is_hfst_peek(FILE * file)
{
  int file_first_char = getc(file);

  bool ret_val = 0;

  if (file_first_char == 'H')
    { ret_val = 1; }

  if (file_first_char != EOF)
    { ungetc(file_first_char, file); }

  return ret_val;
}
    
hfst_word_model::hfst_word_model(FILE *file, 
			     DawgType type, 
			     const STRING &lang, 
			     PermuterType perm, 
			     int debug_level):
  file(0),
  acceptor(file),
  type(type),
  lang(lang),
  perm(perm),
  debug_level(debug_level),
  transition_count(0)
{ 
  set_tr_start_indices();
  set_symbols();
}

hfst_word_model::hfst_word_model(const char *filename, 
			     DawgType type, 
			     const STRING &lang, 
			     PermuterType perm, 
			     int debug_level):
  file(fopen(filename, "r")),
  acceptor(file),
  type(type),
  lang(lang),
  perm(perm),
  debug_level(debug_level),
  transition_count(0)
{ 
  fclose(file); 

  set_tr_start_indices(); 
  set_symbols();
}

hfst_word_model::hfst_word_model(EDGE_ARRAY edges, 
			     int num_edges,
			     DawgType type, 
			     const STRING &lang, 
			     PermuterType perm, 
			     int unicharset_size, 
			     int debug_level)
{
  // FIXME?

  assert(0);
}

hfst_word_model::~hfst_word_model(void)
{ 

}

int hfst_word_model::NumEdges (void)
{ 
  return transition_count; 
}

void hfst_word_model::set_unichar_ids(const UNICHARSET &uc_set)
{
  // FIXME
  for (int i = 1; i < acceptor.alphabet_size(); ++i)
    {
      unichar_symbol_map
	[uc_set.unichar_to_id(acceptor.string_symbol(i).c_str())] = i;

      symbol_unichar_map[i] = uc_set.unichar_to_id(acceptor.string_symbol(i).c_str());
    }
}

EDGE_REF hfst_word_model::edge_char_of(NODE_REF node, 
				       UNICHAR_ID unichar_id, 
				       bool word_end) const
{
  assert(node != NOSTATE);

  unsigned short symbol_index = get_uc_symbol(unichar_id);

  if (symbol_index == FINAL_SYMBOL)
    { return NO_EDGE; }

  unsigned int tr_id = acceptor.get_transition_start_index(node, symbol_index);

  if (tr_id == NOINDEX)
    { return NO_EDGE; }

  if (word_end)
    {
      unsigned int target_state = acceptor.get_target_index(tr_id);

      if (not acceptor.is_final_state(target_state))
	{ return NO_EDGE; }
    }

  return tr_id;
}

void hfst_word_model::unichar_ids_of(NODE_REF node, NodeChildVector * vec,
				     bool eow) const
{
  unsigned int tr_id      = transition_start_indices.at(node);
  unsigned short i_symbol = acceptor.get_input_symbol(tr_id);

  while (i_symbol != FINAL_SYMBOL)
    {
      if (not eow or end_of_word(tr_id))
	{
	  vec->push_back(NodeChild(get_symbol_uc(i_symbol),
				   acceptor.get_target_index(tr_id)));
	}
      
      ++tr_id;
      i_symbol = acceptor.get_input_symbol(tr_id);
    }
}

NODE_REF hfst_word_model::next_node(EDGE_REF edge) const
{ 
  assert(edge != NO_EDGE);

  return acceptor.get_target_index(edge); 
}

bool hfst_word_model::end_of_word(EDGE_REF edge_ref) const
{  
  assert(edge_ref != NO_EDGE);

  unsigned int target_state = acceptor.get_target_index(edge_ref);

  return acceptor.is_final_state(target_state);
}

UNICHAR_ID hfst_word_model::edge_letter(EDGE_REF edge_ref) const
{ 
  assert(edge_ref != NO_EDGE);

  return acceptor.get_input_symbol(edge_ref);
}

void hfst_word_model::print_node(NODE_REF node, int max_num_edges) const
{ 

  std::cerr << "whatever" << std::endl; 
}

void hfst_word_model::write_squished_dawg(FILE *file)
{ 
  // FIXME
  assert(0);
}

void hfst_word_model::write_squished_dawg(const char *filename)
{
  // FIXME
  assert(0);
}

UNICHAR_ID hfst_word_model::get_symbol_uc(unsigned short symbol) const
{
  assert(symbol != FINAL_SYMBOL);

  if (symbol == FINAL_SYMBOL)
    { return NOINDEX; }

  return symbol_unichar_map.at(symbol);
}

unsigned short hfst_word_model::get_uc_symbol(UNICHAR_ID uc) const
{

  if (unichar_symbol_map.count(uc) == 0)
    { return FINAL_SYMBOL; }
  
  return unichar_symbol_map.find(uc)->second;
}

void hfst_word_model::set_symbols(void)
{ 
  for (int i = 0; i < acceptor.alphabet_size(); ++i)
    {
      UNICHAR u(acceptor.string_symbol(i).c_str(), -1);
	
      // Only map possible input symbols.
      if (u.utf8_len() == 1)
	{ 
	  unichar_symbol_map[u.first_uni()] = i; 
	  symbol_unichar_map.push_back(u.first_uni());
	}
      else
	{ symbol_unichar_map.push_back(NOINDEX); }
    }
}

typedef std::queue<unsigned int> StateQueue;

void hfst_word_model::set_tr_start_indices(void)
{
  StateQueue agenda;

  agenda.push(FST_START);

  size_t alphabet_size = acceptor.alphabet_size();

  while (not agenda.empty())
    {
      unsigned int state = agenda.front();
      agenda.pop();

      unsigned int tr_index = NOINDEX;
      
      for (int i = 0; i < alphabet_size; ++i)
	{
	  unsigned int start_index = acceptor.get_transition_start_index(state, i);

	  if (start_index != NOINDEX)
	    { 
	      tr_index = start_index;
	      break;
	    }
	}
      
      transition_start_indices[state] = tr_index;

      if (tr_index != NOINDEX)
	{            
	  while (acceptor.get_input_symbol(tr_index) != FINAL_SYMBOL)
	    {
	      ++transition_count;
	      
	      unsigned int target = acceptor.get_target_index(tr_index);
	
	      if (transition_start_indices.count(target) == 0)
		{ 
		  agenda.push(target); 
		  transition_start_indices[target] = NOINDEX;
		}

	      tr_index += 1; //TRSIZE;
	    }
	}
    }
}

}

#else // TEST_hfst_word_model_cc

#include <iostream>

using namespace tesseract;

int main(int argc, char * argv[])
{
  hfst_word_model model(argv[1], DAWG_TYPE_WORD, "FIN",  NO_PERM, 1);
}

#endif // TEST_hfst_word_model_cc
