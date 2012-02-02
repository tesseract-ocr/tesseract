/* -*-C-*-
 ********************************************************************************
 *
 * File:        matchtab.h  (Formerly matchtab.h)
 * Description:  Match table to retain blobs that were matched.
 * Author:       Mark Seaman, OCR Technology
 * Created:      Mon Jan 29 09:00:56 1990
 * Modified:     Tue Mar 19 15:38:19 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 *********************************************************************************/
#ifndef MATCHTAB_H
#define MATCHTAB_H

#include "ratngs.h"
#include "blobs.h"

namespace tesseract {

struct MATCH {
  MATCH() : rating(NULL) {}
  TBOX box;
  BLOB_CHOICE_LIST *rating;
};

// A class for mapping rectangular bounding boxes to choice lists.
// Only meant to be used at the word level, as we have a limit of
// 500 recognition lists for all subsequences of blobs.
class BlobMatchTable {
 public:
  BlobMatchTable();
  ~BlobMatchTable();

  void init_match_table();
  void end_match_table();
  void put_match(TBLOB *blob, BLOB_CHOICE_LIST *ratings);
  BLOB_CHOICE_LIST *get_match(TBLOB *blob);
  BLOB_CHOICE_LIST *get_match_by_box(const TBOX &box);
  void add_to_match(TBLOB *blob, BLOB_CHOICE_LIST *ratings);

 private:
  int Hash(const TBOX &box) const;
  // Returns whether the idx entry in the array is still empty.
  bool IsEmpty(int idx) const;

  bool been_initialized_;
  MATCH* match_table_;
};

}
#endif
