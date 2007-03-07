/* Include this file in any module which needs to have conditional compilation
  of sensitive code for UNLV. In INTERNAL mode SECURE_NAMES is NOT defined.
  For UNLV mode it IS defined, allowing multiple modules to do conditional
  compilation on the same name.
*/

#ifndef SECURE_NAMES
/* #define SECURE_NAMES */
#endif
