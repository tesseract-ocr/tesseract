// Place holder
#define DLLSYM
#ifdef __MSW32__
#define SIGNED
#define snprintf _snprintf
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
