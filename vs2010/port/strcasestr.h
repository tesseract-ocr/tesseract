/*
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its Copyright notices. In addition publicly
documented acknowledgment must be given that this software has been used if no
source code of this software is made available publicly. Making the source
available publicly means including the source for this software with the
distribution, or a method to get this software via some reasonable mechanism
(electronic transfer via a network or media) as well as making an offer to
supply the source on request. This Copyright notice serves as an offer to
supply the source on on request as well. Instead of this, supplying
acknowledgments of use of this software in either Copyright notices, Manuals,
Publicity and Marketing documents or any documentation provided with any
product containing this software. This License does not apply to any software
that links to the libraries provided by this software (statically or
dynamically), but only to the software provided.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Source:
Evil 1.7.4
The Evil library tried to port some convenient Unix functions
to the Windows (XP or CE) platform. They are planned to be used

http://git.enlightenment.org/legacy/evil.git/tree/src/lib/evil_string.h?id=eeaddf80d0d547d4c216974038c0599b34359695
*/

#ifndef VS2010_PORT_STRCASESTR_H_
#define VS2010_PORT_STRCASESTR_H_
/**
 * @brief Locatea substring into a string, ignoring case.
 *
 * @param haystack The string to search in.
 * @param needle The substring to find.
 * @return
 *
 * This function locates the string @p needle into the string @p haystack,
 * ignoring the case of the characters. It returns apointer to the
 * beginning of the substring, or NULL if the substring is not found.
 * If @p haystack or @p needle are @c NULL, this function returns @c NULL.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP, Windows CE
 */
char *strcasestr(const char *haystack, const char *needle);

#endif  /* VS2010_PORT_STRCASESTR_H_ */
