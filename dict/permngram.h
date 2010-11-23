///////////////////////////////////////////////////////////////////////
// File:        permngram.h
// Description: Character n-gram permuter
// Author:      Thomas Kielbus
// Created:     Wed Sep 12 11:26:42 PDT 2007
//
// (C) Copyright 2007, Google Inc.
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

#ifndef PERMNGRAM_H
#define PERMNGRAM_H

#include "dict.h"
#include "clst.h"
#include "unichar.h"
#include "params.h"

class ViterbiEntry : public ELIST_LINK {
 public:
  ViterbiEntry() {}
  ~ViterbiEntry() {}
  // Comparator function for sorting ViterbiEntry_LISTs in
  // non-increasing order of costs.
  static int compare(const void *e1, const void *e2) {
    const ViterbiEntry *ve1 =
      *reinterpret_cast<const ViterbiEntry * const *>(e1);
    const ViterbiEntry *ve2 =
      *reinterpret_cast<const ViterbiEntry * const *>(e2);
    if (ve1->cost == ve2->cost) return 0;
    return (ve1->cost < ve2->cost) ? -1 : 1;
  }
  inline void CopyChars(const ViterbiEntry &src) {
    strcpy(string_so_far, src.string_so_far);
    num_unichars = src.num_unichars;
    if (src.num_unichars > 0) {
      memcpy(fragment_lengths, src.fragment_lengths,
             src.num_unichars * sizeof(char));
      memcpy(unichar_ids, src.unichar_ids,
             src.num_unichars * sizeof(UNICHAR_ID));
    }
  }
  inline void UpdateChars(const char *unichar, int unichar_length,
                          UNICHAR_ID unichar_id) {
    char *string_so_far_end = string_so_far + strlen(string_so_far);
    strcpy(string_so_far_end, unichar);
    fragment_lengths[num_unichars] = unichar_length;
    unichar_ids[num_unichars] = unichar_id;
    num_unichars++;
  }

  void Print() const {
    tprintf("ViterbiEntry: string_so_far=%s cost=%g ratings_sum=%g"
            " unichar_ids=[ ", string_so_far, cost, ratings_sum);
    int i;
    for (i = 0; i < num_unichars; ++i) tprintf("%d ", unichar_ids[i]);
    tprintf("] unichar lengths=[ ");
    for (i = 0; i < num_unichars; ++i) tprintf("%d ", fragment_lengths[i]);
    tprintf("]\n");
  }
  char string_so_far[MAX_WERD_LENGTH * UNICHAR_LEN + 1];
  int num_unichars;
  char fragment_lengths[MAX_WERD_LENGTH];
  UNICHAR_ID unichar_ids[MAX_WERD_LENGTH];
  float cost;
  float ratings_sum;
  const CHAR_FRAGMENT *frag;
};

// Make ViterbiEntry listable.
ELISTIZEH(ViterbiEntry)

#endif  // PERMNGRAM_H
