///////////////////////////////////////////////////////////////////////
// File:        hfst_size_defs.h
// Description: Sizes of fields used in HFST optimized lookup fsts
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

#ifndef HEADER_size_defs_h
#define HEADER_size_defs_h

namespace hfst
{
const int INTSIZE           =          4;
const int SHORTSIZE         =          2;
const int CHARSIZE          =          1;
const int TRINDEXSIZE       =          6;
const int TRSIZE            =         12;
const int TRISYMBOLOFFSET   =          0;
const int TRIINDEXOFFSET    =          2;
const int TRINSYMBOLOFFSET  =          0;
const int TROUTSYMBOLOFFSET =          2;
const int TRSTATEOFFSET     =          4;
const int TRWEIGHTOFFSET    =          8;
const int TRTABLEZERO       = 2147483648;
const int NOINDEX           =         -1;
const int NOSTATE           =         -1;
const int FINAL_SYMBOL      =      65535;
const int MAX_STRING_BUFFER =     100000;
}

#endif // HEADER_size_defs_h
