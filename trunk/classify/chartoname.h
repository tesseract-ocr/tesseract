#ifndef CHARTONAME_H
#define CHARTONAME_H

                                 /*result */
void chartoname(register char *name,
                char c,            /*char to convert */
                const char *dir);  /*directory to use */

/*
#if defined(__STDC__) || defined(__cplusplus)
# define _P(s) s
#else
# define _P(s) ()
#endif*/

/* chartoname.c
int chartoname _P((char *name, int c, char *dir));

#undef _P
*/
#endif
