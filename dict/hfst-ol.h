/* -*- Mode: C++ -*- */
// Copyright 2010 University of Helsinki
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

/*
 * This file contains some classes, typedefs and constant common to all
 * hfst-optimized-lookup stuff. This is just to get them out of the way
 * of the actual ospell code.
 */

#ifndef HFST_OSPELL_HFST_OL_H_
#define HFST_OSPELL_HFST_OL_H_

#include <vector>
#include <map>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <set>
#include <utility>
#include "ol-exceptions.h"

namespace hfst_ol {

typedef unsigned short SymbolNumber;
typedef unsigned int TransitionTableIndex;
typedef std::vector<SymbolNumber> SymbolVector;
typedef std::vector<std::string> KeyTable;
typedef std::map<std::string, SymbolNumber> StringSymbolMap;
typedef short ValueNumber;
typedef float Weight;

// Forward declarations to typedef some more containers
class TransitionIndex;
class Transition;
class FlagDiacriticOperation;

typedef std::vector<TransitionIndex*> TransitionIndexVector;
typedef std::vector<Transition*> TransitionVector;

typedef std::map<SymbolNumber, FlagDiacriticOperation> OperationMap;

const SymbolNumber NO_SYMBOL = USHRT_MAX;
const TransitionTableIndex NO_TABLE_INDEX = UINT_MAX;
const Weight INFINITE_WEIGHT = static_cast<float>(NO_TABLE_INDEX);
const unsigned int MAX_SYMBOL_BYTES = 1000;

// This is 2^31, hopefully equal to UINT_MAX/2 rounded up.
// For some profound reason it can't be replaced with (UINT_MAX+1)/2.
const TransitionTableIndex TARGET_TABLE = 2147483648u;

// the flag diacritic operators as given in
// Beesley & Karttunen, Finite State Morphology (U of C Press 2003)
enum FlagDiacriticOperator {P, N, R, D, C, U};

enum HeaderFlag {Weighted, Deterministic, Input_deterministic, Minimized,
                 Cyclic, Has_epsilon_epsilon_transitions,
                 Has_input_epsilon_transitions, Has_input_epsilon_cycles,
                 Has_unweighted_input_epsilon_cycles};

// Utility function for dealing with raw memory
void skip_c_string(char ** raw);

//! Internal class for Transducer processing.

//! Contains low-level processing stuff.
class TransducerHeader
{
private:
    SymbolNumber number_of_symbols;
    SymbolNumber number_of_input_symbols;
    TransitionTableIndex size_of_transition_index_table;
    TransitionTableIndex size_of_transition_target_table;

    TransitionTableIndex number_of_states;
    TransitionTableIndex number_of_transitions;

    bool weighted;
    bool deterministic;
    bool input_deterministic;
    bool minimized;
    bool cyclic;
    bool has_epsilon_epsilon_transitions;
    bool has_input_epsilon_transitions;
    bool has_input_epsilon_cycles;
    bool has_unweighted_input_epsilon_cycles;
    void read_property(bool &property, FILE * f);
    void read_property(bool &property, char ** raw);
    void skip_hfst3_header(FILE * f);
    void skip_hfst3_header(char ** f);

public:
    //!
    //! @brief read header from file @a f
    TransducerHeader(FILE * f);

    //!
    //! read header from raw memory data @a raw
    TransducerHeader(char ** raw);
    //!
    //! count symbols
    SymbolNumber symbol_count(void);
    //!
    //! count input symbols
    SymbolNumber input_symbol_count(void);
    //!
    //! index table size
    TransitionTableIndex index_table_size(void);
    //!
    //! target table size
    TransitionTableIndex target_table_size(void);
    //!
    //! check for flag
    bool probe_flag(HeaderFlag flag);
};

//! Internal class for flag diacritic processing.

//! Contains low-level processing stuff.
class FlagDiacriticOperation
{
private:
    const FlagDiacriticOperator operation;
    const SymbolNumber feature;
    const ValueNumber value;
public:
    //! 
    //! Construct flag diacritic of from \@ @a op . @a feat . @a val \@.
    FlagDiacriticOperation(const FlagDiacriticOperator op,
                           const SymbolNumber feat,
                           const ValueNumber val):
        operation(op), feature(feat), value(val) {}

    // dummy constructor
    FlagDiacriticOperation():
        operation(P), feature(NO_SYMBOL), value(0) {}
  
    //!
    //! check if flag
    bool isFlag(void) const;
    //!
    //! Operation something I don't understand really.
    FlagDiacriticOperator Operation(void) const;
    //! 
    //! No clue
    SymbolNumber Feature(void) const;
    //! 
    //! Not a slightest idea
    ValueNumber Value(void) const;

};

//! Internal class for alphabet processing.

//! Contains low-level processing stuff.
class TransducerAlphabet
{
private:
    KeyTable kt;
    OperationMap operations;
    SymbolNumber other_symbol;
    SymbolNumber flag_state_size;
    StringSymbolMap string_to_symbol;
    void process_symbol(char * line);
    
    void read(FILE * f, SymbolNumber number_of_symbols);
    void read(char ** raw, SymbolNumber number_of_symbols);
    
public:
    //! 
    //! read alphabets from file @a f
    TransducerAlphabet(FILE *f, SymbolNumber number_of_symbols);
    //! 
    //! read alphabes from raw data @a raw
    TransducerAlphabet(char ** raw, SymbolNumber number_of_symbols);
    //!
    //! get alphabet's keytable mapping
    KeyTable * get_key_table(void);
    //! 
    //! get flag operation map stuff
    OperationMap * get_operation_map(void);
    //!
    //! get state's size
    SymbolNumber get_state_size(void);
    //! 
    //! get position of other symbol
    SymbolNumber get_other(void);
    //!
    //! get mapping from strings to symbols
    StringSymbolMap * get_string_to_symbol(void);
    //! 
    //! get if given symbol is a flag
    bool is_flag(SymbolNumber symbol);
};

class LetterTrie;
typedef std::vector<LetterTrie*> LetterTrieVector;

//! Internal class for alphabet processing.

//! Contains low-level processing stuff.
class LetterTrie
{
private:
    LetterTrieVector letters;
    SymbolVector symbols;

public:
    LetterTrie(void):
    letters(UCHAR_MAX, static_cast<LetterTrie*>(NULL)),
    symbols(UCHAR_MAX,NO_SYMBOL)
        {}
    //! 
    //! add a string to alphabets with a key
    void add_string(const char * p,SymbolNumber symbol_key);
    //!
    //! find a key for string or add it
    SymbolNumber find_key(char ** p);
    ~LetterTrie();
};

//! Internal class for alphabet processing.

//! Contains low-level processing stuff.
class Encoder {

private:
    LetterTrie letters;
    SymbolVector ascii_symbols;

    void read_input_symbols(KeyTable * kt, SymbolNumber number_of_input_symbols);

public:
    //!
    //! create encoder from keytable 
    Encoder(KeyTable * kt, SymbolNumber number_of_input_symbols);
    //!
    //! find corresponding key for alphabet or something
    SymbolNumber find_key(char ** p);
};

typedef std::vector<ValueNumber> FlagDiacriticState;

//! Internal class for transition data.

//! Contains low-level processing stuff.
class TransitionIndex
{
protected:
    SymbolNumber input_symbol; //!< transition's input symbol
    TransitionTableIndex first_transition_index; //!< first transition location
  
public:
  
    //!
    //! Each TransitionIndex has an input symbol and a target index.
    static const size_t SIZE = 
        sizeof(SymbolNumber) + sizeof(TransitionTableIndex);

    //!
    //! Create transition index for symbol
    TransitionIndex(const SymbolNumber input,
                    const TransitionTableIndex first_transition):
        input_symbol(input),
        first_transition_index(first_transition)
        {}
    //!
    //! return target of transition
    TransitionTableIndex target(void) const;
    //!
    //! whether it's final state
    bool final(void) const;
    //!
    //! retrieve final weight
    Weight final_weight(void) const;
    //!
    //! symbol number for transitions input
    SymbolNumber get_input(void) const;
};

//! Internal class for transition processing.

//! Contains low-level processing stuff.
class Transition
{
protected:
    SymbolNumber input_symbol; //!< input symbol
    SymbolNumber output_symbol; //!< output symbol
    TransitionTableIndex target_index; //!< location of target of transition
    Weight transition_weight; //!< tranisition's weight

public:

    //! Each transition has an input symbol, an output symbol and 
    //! a target index.
    static const size_t SIZE = 
        2 * sizeof(SymbolNumber) + sizeof(TransitionTableIndex) + sizeof(Weight);

    //!
    //! Create transition with input, output, target and weight.
    Transition(const SymbolNumber input,
               const SymbolNumber output,
               const TransitionTableIndex target,
               const Weight w):
        input_symbol(input),
        output_symbol(output),
        target_index(target),
        transition_weight(w)
        {}

    Transition():
        input_symbol(NO_SYMBOL),
        output_symbol(NO_SYMBOL),
        target_index(NO_TABLE_INDEX),
        transition_weight(INFINITE_WEIGHT)
        {}

    //! 
    //! get transitions target
    TransitionTableIndex target(void) const;
    //!
    //! get output symbol
    SymbolNumber get_output(void) const;
    //!
    //! get input symbol
    SymbolNumber get_input(void) const;
    //!
    //! get transition weight
    Weight get_weight(void) const;
    //!
    //! whether transition is final
    bool final(void) const;
};

//! Internal class for Transducer processing.

//! Contains low-level processing stuff.
class IndexTable
{
private:
    char * indices;
  
    void read(FILE * f,
              TransitionTableIndex number_of_table_entries);
    void read(char ** raw,
              TransitionTableIndex number_of_table_entries);


public:
    //!
    //! read index table from file @a f.
    IndexTable(FILE * f,
               TransitionTableIndex number_of_table_entries);
    //!
    //! read index table from raw data @a raw.
    IndexTable(char ** raw,
               TransitionTableIndex number_of_table_entries);
    ~IndexTable(void);
    //!
    //! input symbol for the index
    SymbolNumber input_symbol(TransitionTableIndex i) const;
    //!
    //! target state location for the index
    TransitionTableIndex target(TransitionTableIndex i) const;
    //! 
    //! whether it's final transition
    bool final(TransitionTableIndex i) const;
    //! 
    //! transition's weight
    Weight final_weight(TransitionTableIndex i) const;
};

//! Internal class for transition processing.

//! Contains low-level processing stuff.
class TransitionTable
{
protected:
    //!
    //! raw transition data
    char * transitions;
 
    //!
    //! read known amount of transitions from file @a f
    void read(FILE * f,
              TransitionTableIndex number_of_table_entries);
    //! read known amount of transitions from raw dara @a data
    void read(char ** raw,
              TransitionTableIndex number_of_table_entries);

public:
    //! 
    //! read transition table from file @a f
    TransitionTable(FILE * f,
                    TransitionTableIndex transition_count);
    //!
    //! read transition table from raw data @a raw
    TransitionTable(char ** raw,
                    TransitionTableIndex transition_count);

    ~TransitionTable(void);
    //! 
    //! transition's input symbol
    SymbolNumber input_symbol(TransitionTableIndex i) const;
    //! 
    //! transition's output symbol
    SymbolNumber output_symbol(TransitionTableIndex i) const;
    //!
    //! target node location
    TransitionTableIndex target(TransitionTableIndex i) const;
    //!
    //! weight of transiton
    Weight weight(TransitionTableIndex i) const;
    //!
    //! whether it's final
    bool final(TransitionTableIndex i) const;


};

template <class printable>
void debug_print(printable p)
{
    if (0) {
        std::cerr << p;
    }
}

} // namespace hfst_ol
    
#endif // HFST_OSPELL_HFST_OL_H_
