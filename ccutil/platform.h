// Place holder
#define DLLSYM
#ifdef __MSW32__
#define SIGNED
#define snprintf _snprintf
#define read _read
#define write _write
#define close _close
#define lseek _lseek
#define open _open
#define ultoa _ultoa
#define ltoa _ltoa
#define strtok_r(s, d, p) strtok(s, d)
#if (_MSC_VER <= 1400)
#define vsnprintf _vsnprintf
#endif
#else
#define __UNIX__
#include <limits.h>
#ifndef PATH_MAX
#define MAX_PATH 4096
#else
#define MAX_PATH PATH_MAX
#endif
#define SIGNED signed
#endif
