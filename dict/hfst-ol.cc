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

#include "hfst-ol.h"
#include <string>

namespace hfst_ol {

void skip_c_string(char ** raw)
{
    while (**raw != 0) {
        ++(*raw);
    }
    ++(*raw);
}

void
TransducerHeader::read_property(bool& property, FILE* f)
{
    unsigned int prop;
    if (fread(&prop,sizeof(unsigned int),1,f) != 1) {
        HFST_THROW_MESSAGE(HeaderParsingException,
                           "Header ended unexpectedly\n");
    }
    if (prop == 0)
    {
        property = false;
        return;
    }
    else
    {
        property = true;
        return;
    }
}

void
TransducerHeader::read_property(bool& property, char** raw)
{
    unsigned int prop = *((unsigned int *) *raw);
    (*raw) += sizeof(unsigned int);
    if (prop == 0)
    {
        property = false;
        return;
    }
    else
    {
        property = true;
        return;
    }
}

void TransducerHeader::skip_hfst3_header(FILE * f)
{
    const char* header1 = "HFST";
    unsigned int header_loc = 0; // how much of the header has been found
    int c;
    for(header_loc = 0; header_loc < strlen(header1) + 1; header_loc++)
    {
        c = getc(f);
        if(c != header1[header_loc]) {
            break;
        }
    }
    if(header_loc == strlen(header1) + 1) // we found it
    {
        unsigned short remaining_header_len;
        if (fread(&remaining_header_len,
                  sizeof(remaining_header_len), 1, f) != 1 ||
            getc(f) != '\0') {
            HFST_THROW_MESSAGE(HeaderParsingException,
                               "Found broken HFST3 header\n");
        }
        char * headervalue = new char[remaining_header_len];
        if (fread(headervalue, remaining_header_len, 1, f) != 1)
        {
            HFST_THROW_MESSAGE(HeaderParsingException,
                               "HFST3 header ended unexpectedly\n");
        }
        if (headervalue[remaining_header_len - 1] != '\0') {
            HFST_THROW_MESSAGE(HeaderParsingException,
                               "Found broken HFST3 header\n");
        }
        std::string header_tail(headervalue, remaining_header_len);
        size_t type_field = header_tail.find("type");
        if (type_field != std::string::npos) {
            if (header_tail.find("HFST_OL") != type_field + 5 &&
                header_tail.find("HFST_OLW") != type_field + 5) {
                delete headervalue;
                HFST_THROW_MESSAGE(
                    TransducerTypeException,
                    "Transducer has incorrect type, should be "
                    "hfst-optimized-lookup\n");
            }
        }
    } else // nope. put back what we've taken
    {
        ungetc(c, f); // first the non-matching character
        for(int i = header_loc - 1; i>=0; i--) {
            // then the characters that did match (if any)
            ungetc(header1[i], f);
        }
    }
}

void TransducerHeader::skip_hfst3_header(char ** raw)
{
    const char* header1 = "HFST";
    unsigned int header_loc = 0; // how much of the header has been found

    for(header_loc = 0; header_loc < strlen(header1) + 1; header_loc++)
    {
        if(**raw != header1[header_loc]) {
            break;
        }
        ++(*raw);
    }
    if(header_loc == strlen(header1) + 1) // we found it
    {
        unsigned short remaining_header_len = *((unsigned short *) *raw);
        (*raw) += sizeof(unsigned short) + 1 + remaining_header_len;
    } else // nope. put back what we've taken
    {
        --(*raw); // first the non-matching character
        for(int i = header_loc - 1; i>=0; i--) {
            // then the characters that did match (if any)
            --(*raw);
        }
    }
}

TransducerHeader::TransducerHeader(FILE* f)
{
    skip_hfst3_header(f); // skip header iff it is present
    /* The following conditional clause does all the numerical reads
       and throws an exception if any fails to return 1 */
    if (fread(&number_of_input_symbols,
              sizeof(SymbolNumber),1,f) != 1||
        fread(&number_of_symbols,
              sizeof(SymbolNumber),1,f) != 1||
        fread(&size_of_transition_index_table,
              sizeof(TransitionTableIndex),1,f) != 1||
        fread(&size_of_transition_target_table,
              sizeof(TransitionTableIndex),1,f) != 1||
        fread(&number_of_states,
              sizeof(TransitionTableIndex),1,f) != 1||
        fread(&number_of_transitions,
              sizeof(TransitionTableIndex),1,f) != 1) {
        HFST_THROW_MESSAGE(HeaderParsingException,
                           "Header ended unexpectedly\n");
    }
    read_property(weighted,f);
    read_property(deterministic,f);
    read_property(input_deterministic,f);
    read_property(minimized,f);
    read_property(cyclic,f);
    read_property(has_epsilon_epsilon_transitions,f);
    read_property(has_input_epsilon_transitions,f);
    read_property(has_input_epsilon_cycles,f);
    read_property(has_unweighted_input_epsilon_cycles,f);
}

TransducerHeader::TransducerHeader(char** raw)
{
    skip_hfst3_header(raw); // skip header iff it is present
    number_of_input_symbols = *(SymbolNumber*) *raw;
    (*raw) += sizeof(SymbolNumber);
    number_of_symbols = *(SymbolNumber*) *raw;
    (*raw) += sizeof(SymbolNumber);
    size_of_transition_index_table = *(TransitionTableIndex*) *raw;
    (*raw) += sizeof(TransitionTableIndex);
    size_of_transition_target_table = *(TransitionTableIndex*) *raw;
    (*raw) += sizeof(TransitionTableIndex);
    number_of_states = *(TransitionTableIndex*) *raw;
    (*raw) += sizeof(TransitionTableIndex);
    number_of_transitions = *(TransitionTableIndex*) *raw;
    (*raw) += sizeof(TransitionTableIndex);
    read_property(weighted,raw);
    read_property(deterministic,raw);
    read_property(input_deterministic,raw);
    read_property(minimized,raw);
    read_property(cyclic,raw);
    read_property(has_epsilon_epsilon_transitions,raw);
    read_property(has_input_epsilon_transitions,raw);
    read_property(has_input_epsilon_cycles,raw);
    read_property(has_unweighted_input_epsilon_cycles,raw);
}

SymbolNumber
TransducerHeader::symbol_count()
  {
    return number_of_symbols; 
  }

SymbolNumber
TransducerHeader::input_symbol_count()
  {
    return number_of_input_symbols; 
  }
TransitionTableIndex
TransducerHeader::index_table_size(void)
  {
    return size_of_transition_index_table; 
  }

TransitionTableIndex 
TransducerHeader::target_table_size()
  {
    return size_of_transition_target_table; 
  }

bool
TransducerHeader::probe_flag(HeaderFlag flag)
  {
    switch (flag) {
    case Weighted:
        return weighted;
    case Deterministic:
        return deterministic;
    case Input_deterministic:
        return input_deterministic;
    case Minimized:
        return minimized;
    case Cyclic:
        return cyclic;
    case Has_epsilon_epsilon_transitions:
        return has_epsilon_epsilon_transitions;
    case Has_input_epsilon_transitions:
        return has_input_epsilon_transitions;
    case Has_input_epsilon_cycles:
        return has_input_epsilon_cycles;
    case Has_unweighted_input_epsilon_cycles:
        return has_unweighted_input_epsilon_cycles;
    }
    return false;
  }

bool
FlagDiacriticOperation::isFlag() const
  {
    return feature != NO_SYMBOL; 
  }

FlagDiacriticOperator
FlagDiacriticOperation::Operation() const
  {
    return operation;
  }

SymbolNumber
FlagDiacriticOperation::Feature() const
  {
    return feature;
  }


ValueNumber
FlagDiacriticOperation::Value() const
  {
    return value;
  }


void TransducerAlphabet::read(FILE * f, SymbolNumber number_of_symbols)
{
    char * line = (char *) malloc(MAX_SYMBOL_BYTES);
    std::map<std::string, SymbolNumber> feature_bucket;
    std::map<std::string, ValueNumber> value_bucket;
    value_bucket[std::string()] = 0; // empty value = neutral
    ValueNumber val_num = 1;
    SymbolNumber feat_num = 0;

    kt.push_back(std::string("")); // zeroth symbol is epsilon
    int byte;
    while ( (byte = fgetc(f)) != 0 ) {
        /* pass over epsilon */
        if (byte == EOF) {
            HFST_THROW(AlphabetParsingException);
        }
    }

    for (SymbolNumber k = 1; k < number_of_symbols; ++k) {
        char * sym = line;
        while ( (byte = fgetc(f)) != 0 ) {
            if (byte == EOF) {
                HFST_THROW(AlphabetParsingException);
            }
            *sym = byte;
            ++sym;
        }
        *sym = 0;
        // Detect and handle special symbols, which begin and end with @
        if (line[0] == '@' && line[strlen(line) - 1] == '@') {
            if (strlen(line) >= 5 && line[2] == '.') { // flag diacritic
                std::string feat;
                std::string val;
                FlagDiacriticOperator op = P; // for the compiler
                switch (line[1]) {
                case 'P': op = P; break;
                case 'N': op = N; break;
                case 'R': op = R; break;
                case 'D': op = D; break;
                case 'C': op = C; break;
                case 'U': op = U; break;
                }
                char * c = line;
                for (c +=3; *c != '.' && *c != '@'; c++) { feat.append(c,1); }
                if (*c == '.')
                {
                    for (++c; *c != '@'; c++) { val.append(c,1); }
                }
                if (feature_bucket.count(feat) == 0)
                {
                    feature_bucket[feat] = feat_num;
                    ++feat_num;
                }
                if (value_bucket.count(val) == 0)
                {
                    value_bucket[val] = val_num;
                    ++val_num;
                }

                operations.insert(
                    std::pair<SymbolNumber, FlagDiacriticOperation>(
                        k,
                        FlagDiacriticOperation(
                            op, feature_bucket[feat], value_bucket[val])));

                kt.push_back(std::string(""));
                continue;

            } else if (strcmp(line, "@_UNKNOWN_SYMBOL_@") == 0) { // other symbol
                other_symbol = k;
                kt.push_back(std::string(""));
                continue;
            } else { // we don't know what this is, ignore and suppress
                kt.push_back(std::string(""));
                continue;
            }
        }
        kt.push_back(std::string(line));
        string_to_symbol[std::string(line)] = k;
    }
    free(line);
    flag_state_size = feature_bucket.size();
}

void TransducerAlphabet::read(char ** raw, SymbolNumber number_of_symbols)
{
    std::map<std::string, SymbolNumber> feature_bucket;
    std::map<std::string, ValueNumber> value_bucket;
    value_bucket[std::string()] = 0; // empty value = neutral
    ValueNumber val_num = 1;
    SymbolNumber feat_num = 0;

    kt.push_back(std::string("")); // zeroth symbol is epsilon
    skip_c_string(raw);

    for (SymbolNumber k = 1; k < number_of_symbols; ++k) {

        // Detect and handle special symbols, which begin and end with @
        if ((*raw)[0] == '@' && (*raw)[strlen(*raw) - 1] == '@') {
            if (strlen(*raw) >= 5 && (*raw)[2] == '.') { // flag diacritic
                std::string feat;
                std::string val;
                FlagDiacriticOperator op = P; // for the compiler
                switch ((*raw)[1]) {
                case 'P': op = P; break;
                case 'N': op = N; break;
                case 'R': op = R; break;
                case 'D': op = D; break;
                case 'C': op = C; break;
                case 'U': op = U; break;
                }
                char * c = *raw;
                for (c +=3; *c != '.' && *c != '@'; c++) { feat.append(c,1); }
                if (*c == '.')
                {
                    for (++c; *c != '@'; c++) { val.append(c,1); }
                }
                if (feature_bucket.count(feat) == 0)
                {
                    feature_bucket[feat] = feat_num;
                    ++feat_num;
                }
                if (value_bucket.count(val) == 0)
                {
                    value_bucket[val] = val_num;
                    ++val_num;
                }

                operations.insert(
                    std::pair<SymbolNumber, FlagDiacriticOperation>(
                        k,
                        FlagDiacriticOperation(
                            op, feature_bucket[feat], value_bucket[val])));

                kt.push_back(std::string(""));
                skip_c_string(raw);
                continue;

            } else if (strcmp(*raw, "@_UNKNOWN_SYMBOL_@") == 0) { // other symbol
                other_symbol = k;
                kt.push_back(std::string(""));
                skip_c_string(raw);
                continue;
            } else { // we don't know what this is, ignore and suppress
                kt.push_back(std::string(""));
                skip_c_string(raw);
                continue;
            }
        }
        kt.push_back(std::string(*raw));
        string_to_symbol[std::string(*raw)] = k;
        skip_c_string(raw);
    }
    flag_state_size = feature_bucket.size();
}

TransducerAlphabet::TransducerAlphabet(FILE* f, SymbolNumber number_of_symbols):
        other_symbol(NO_SYMBOL)
        {
            read(f, number_of_symbols);
        }

TransducerAlphabet::TransducerAlphabet(char** raw,
                                       SymbolNumber number_of_symbols):
        other_symbol(NO_SYMBOL)
        {
            read(raw, number_of_symbols);
        }

KeyTable*
TransducerAlphabet::get_key_table()
  {
    return &kt; 
  }

OperationMap*
TransducerAlphabet::get_operation_map()
  {
    return &operations;
  }

SymbolNumber
TransducerAlphabet::get_state_size()
  {
    return flag_state_size;
  }

SymbolNumber
TransducerAlphabet::get_other()
  {
    return other_symbol;
  }

StringSymbolMap*
TransducerAlphabet::get_string_to_symbol()
  {
    return &string_to_symbol;
  }

bool
TransducerAlphabet::is_flag(SymbolNumber symbol)
  {
    return operations.count(symbol) == 1;
  }

void IndexTable::read(FILE * f,
                      TransitionTableIndex number_of_table_entries)
{
    size_t table_size = number_of_table_entries*TransitionIndex::SIZE;
    indices = (char*)(malloc(table_size));
    if (fread(indices,table_size, 1, f) != 1) {
        HFST_THROW(IndexTableReadingException);
    }
}

void IndexTable::read(char ** raw,
                      TransitionTableIndex number_of_table_entries)
{
    size_t table_size = number_of_table_entries*TransitionIndex::SIZE;
    indices = (char*)(malloc(table_size));
    memcpy((void *) indices, (const void *) *raw, table_size);
    (*raw) += table_size;
}

void TransitionTable::read(FILE * f,
                           TransitionTableIndex number_of_table_entries)
{
    size_t table_size = number_of_table_entries*Transition::SIZE;
    transitions = (char*)(malloc(table_size));
    if (fread(transitions, table_size, 1, f) != 1) {
        HFST_THROW(TransitionTableReadingException);
    }
}

void TransitionTable::read(char ** raw,
                           TransitionTableIndex number_of_table_entries)
{
    size_t table_size = number_of_table_entries*Transition::SIZE;
    transitions = (char*)(malloc(table_size));
    memcpy((void *) transitions, (const void *) *raw, table_size);
    (*raw) += table_size;
}

void LetterTrie::add_string(const char * p, SymbolNumber symbol_key)
{
    if (*(p+1) == 0)
    {
        symbols[(unsigned char)(*p)] = symbol_key;
        return;
    }
    if (letters[(unsigned char)(*p)] == NULL)
    {
        letters[(unsigned char)(*p)] = new LetterTrie();
    }
    letters[(unsigned char)(*p)]->add_string(p+1,symbol_key);
}

SymbolNumber LetterTrie::find_key(char ** p)
{
    const char * old_p = *p;
    ++(*p);
    if (letters[(unsigned char)(*old_p)] == NULL)
    {
        return symbols[(unsigned char)(*old_p)];
    }
    SymbolNumber s = letters[(unsigned char)(*old_p)]->find_key(p);
    if (s == NO_SYMBOL)
    {
        --(*p);
        return symbols[(unsigned char)(*old_p)];
    }
    return s;
}

LetterTrie::~LetterTrie()
  {
    for (LetterTrieVector::iterator i = letters.begin();
         i != letters.end(); ++i)
      {
        if (*i)
          { 
            delete *i;
          }
      }
   }

Encoder::Encoder(KeyTable * kt, SymbolNumber number_of_input_symbols):
        ascii_symbols(UCHAR_MAX,NO_SYMBOL)
        {
            read_input_symbols(kt, number_of_input_symbols);
        }

void Encoder::read_input_symbols(KeyTable * kt,
                                 SymbolNumber number_of_input_symbols)
{
    for (SymbolNumber k = 0; k < number_of_input_symbols; ++k)
    {
        const char * p = kt->at(k).c_str();
        if (strlen(p) == 0) { // ignore empty strings
            continue;
        }
        if ((strlen(p) == 1) && (unsigned char)(*p) <= 127)
        {
            ascii_symbols[(unsigned char)(*p)] = k;
        }
        letters.add_string(p,k);
    }
}

TransitionTableIndex
TransitionIndex::target() const
  {
    return first_transition_index;
  }

bool 
TransitionIndex::final(void) const
{
    return input_symbol == NO_SYMBOL &&
        first_transition_index != NO_TABLE_INDEX;
}

Weight
TransitionIndex::final_weight(void) const
{
    union to_weight
    {
        TransitionTableIndex i;
        Weight w;
    } weight;
    weight.i = first_transition_index;
    return weight.w;
}

SymbolNumber
TransitionIndex::get_input(void) const
{
    return input_symbol;
}

TransitionTableIndex 
Transition::target(void) const
{
    return target_index;
}

SymbolNumber
Transition::get_output(void) const
{
    return output_symbol;
}

SymbolNumber
Transition::get_input(void) const
{
    return input_symbol;
}

Weight
Transition::get_weight(void) const
{
    return transition_weight;
}

bool
Transition::final(void) const
{
    return input_symbol == NO_SYMBOL &&
        output_symbol == NO_SYMBOL &&
        target_index == 1;
}

IndexTable::IndexTable(FILE* f,
                       TransitionTableIndex number_of_table_entries):
        indices(NULL)
        {
            read(f, number_of_table_entries);
        }
    
IndexTable::IndexTable(char ** raw,
                       TransitionTableIndex number_of_table_entries):
        indices(NULL)
        {
            read(raw, number_of_table_entries);
        }

IndexTable::~IndexTable(void)
        {
            if (indices) {
                free(indices);
            }
        }

SymbolNumber
IndexTable::input_symbol(TransitionTableIndex i) const
        {
          return *((SymbolNumber *)
                   (indices + TransitionIndex::SIZE * i)); 
        }

TransitionTableIndex
IndexTable::target(TransitionTableIndex i) const
  {
     return *((TransitionTableIndex *) 
              (indices + TransitionIndex::SIZE * i + sizeof(SymbolNumber)));
  }

bool
IndexTable::final(TransitionTableIndex i) const
        {
            return input_symbol(i) == NO_SYMBOL && target(i) != NO_TABLE_INDEX;
        }

Weight
IndexTable::final_weight(TransitionTableIndex i) const
        {
            return *((Weight *)
                     (indices + TransitionIndex::SIZE * i + sizeof(SymbolNumber)));
        }

TransitionTable::TransitionTable(FILE * f,
                                 TransitionTableIndex transition_count):
        transitions(NULL)
        {
            read(f, transition_count);
        }
  
TransitionTable::TransitionTable(char ** raw,
                    TransitionTableIndex transition_count):
        transitions(NULL)
        {
            read(raw, transition_count);
        }

TransitionTable::~TransitionTable(void)
        {
            if (transitions) {
                free(transitions);
            }
        }

SymbolNumber
TransitionTable::input_symbol(TransitionTableIndex i) const
        {
            return *((SymbolNumber *)
                     (transitions + Transition::SIZE * i));
        }

SymbolNumber
TransitionTable::output_symbol(TransitionTableIndex i) const
        {
            return *((SymbolNumber *)
                     (transitions + Transition::SIZE * i +
                      sizeof(SymbolNumber)));
        }

TransitionTableIndex
TransitionTable::target(TransitionTableIndex i) const
        {
            return *((TransitionTableIndex *)
                     (transitions + Transition::SIZE * i +
                      2*sizeof(SymbolNumber)));
        }

Weight
TransitionTable::weight(TransitionTableIndex i) const
        {
            return *((Weight *)
                     (transitions + Transition::SIZE * i +
                      2*sizeof(SymbolNumber) + sizeof(TransitionTableIndex)));
        }

bool
TransitionTable::final(TransitionTableIndex i) const
        {
            return input_symbol(i) == NO_SYMBOL &&
                output_symbol(i) == NO_SYMBOL &&
                target(i) == 1;
        }

SymbolNumber Encoder::find_key(char ** p)
{
    if (ascii_symbols[(unsigned char)(**p)] == NO_SYMBOL)
    {
        return letters.find_key(p);
    }
    SymbolNumber s = ascii_symbols[(unsigned char)(**p)];
    ++(*p);
    return s;
}

} // namespace hfst_ol
