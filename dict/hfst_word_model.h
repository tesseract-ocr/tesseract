///////////////////////////////////////////////////////////////////////
// File:        hfst_word_model.h
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

#ifndef HEADER_hfst_word_model_h
#define HEADER_hfst_word_model_h

#include <unordered_map>

#include "dawg.h"

#include "hfst_ol_driver.h"

namespace tesseract
{
  bool is_hfst_peek(FILE * file);
  
  class hfst_word_model : public Dawg
  {
  public:
    hfst_word_model(FILE *file, 
		    DawgType type, 
		    const STRING &lang, 
		    PermuterType perm, 
		    int debug_level);
    
    hfst_word_model(std::istream &in, 
		    DawgType type, 
		    const STRING &lang, 
		    PermuterType perm, 
		    int debug_level);
    
    hfst_word_model(const char *filename, 
		DawgType type, 
		    const STRING &lang, 
		    PermuterType perm, 
		    int debug_level);
    
    hfst_word_model(EDGE_ARRAY edges, 
		    int num_edges,
		    DawgType type, 
		    const STRING &lang, 
		    PermuterType perm, 
		    int unicharset_size, 
		    int debug_level);
    
    ~hfst_word_model(void);

    void set_unichar_ids(const UNICHARSET &uc_set);

    int NumEdges (void);

    EDGE_REF edge_char_of(NODE_REF node, 
			  UNICHAR_ID unichar_id, 
			  bool word_end) const;
    
    void unichar_ids_of(NODE_REF node, NodeChildVector *vec,
			bool eow) const;
    
    NODE_REF next_node(EDGE_REF edge) const;
    
    bool end_of_word(EDGE_REF edge_ref) const;
    
    UNICHAR_ID edge_letter(EDGE_REF edge_ref) const;
    
    void print_node(NODE_REF node, int max_num_edges) const;
    
    void write_squished_dawg(FILE *file);
    
    void write_squished_dawg(const char *filename);

  private:
    typedef std::unordered_map<UNICHAR_ID, unsigned short> UnicharSymbolMap;
    typedef std::vector<UNICHAR_ID> SymbolUnicharMap;
    typedef std::unordered_map<unsigned int, unsigned int> IndexMap;
    
    FILE * file;
    hfst::OLFst<FILE> acceptor;
    DawgType type;
    STRING lang;
    PermuterType perm;
    int debug_level;
    size_t transition_count;
    
    IndexMap transition_start_indices;
    
    SymbolUnicharMap symbol_unichar_map;
    UnicharSymbolMap unichar_symbol_map;
    
    void set_tr_start_indices(void); 
    void set_symbols(void);
    
    UNICHAR_ID get_symbol_uc(unsigned short symbol) const;
    unsigned short get_uc_symbol(UNICHAR_ID uc) const;
  };

}

#endif // HEADER_hfst_word_model_h
