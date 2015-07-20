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
