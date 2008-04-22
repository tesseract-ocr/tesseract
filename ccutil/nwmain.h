/**********************************************************************
 * File:        nwmain.h
 * Description: Tool to declare main, making windows invisible.
 * Author:					Ray Smith
 * Created:					Fri Sep 07 13:27:50 MDT 1995
 *
 * (C) Copyright 1995, Hewlett-Packard Co.
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
 **********************************************************************/

#ifndef RUNMAIN_H
#define RUNMAIN_H

#include          "host.h"
#include          "varable.h"
#include          "notdll.h"     //must be last include

#define DECLARE_MAIN(ARGC,ARGV)\
STRING_VAR(init_config_file,"config","Config file to read on startup");\
REALLY_DECLARE_MAIN(ARGC,ARGV)

#define DECLARE_MAIN_CONFIG(ARGC,ARGV,NAME)\
STRING_VAR(init_config_file,NAME,"Config file to read on startup");\
REALLY_DECLARE_MAIN(ARGC,ARGV)

#ifndef __UNIX__

#define REALLY_DECLARE_MAIN(ARGC,ARGV)\
\
/**********************************************************************\
* parse_args\
*\
* Turn a list of args into a new list of args with each separate\
* whitespace spaced string being an arg.\
**********************************************************************/\
\
inT32						parse_args(					/*refine arg list*/\
inT32						argc,						/*no of input args*/\
char						*argv[],					/*input args*/\
char						*arglist[]					/*output args*/\
)\
{\
	inT32					argcount;					/*converted argc*/\
	char					*testchar;					/*char in option string*/\
	inT32					arg;						/*current argument*/\
\
	argcount=0;											/*no of options*/\
	for (arg=0;arg<argc;arg++)\
	{\
		testchar=argv[arg];								/*start of arg*/\
		do\
		{\
			while (*testchar\
			&& (*testchar==' ' || *testchar=='"' || *testchar=='\n' || *testchar=='\t'))\
				testchar++;								/*skip white space*/\
			if (*testchar)\
			{\
				arglist[argcount++]=testchar;			/*new arg*/\
				do\
				{\
					for (testchar++;*testchar\
					&& *testchar!=' ' && *testchar!='"' && *testchar!='\n' && *testchar!='\t';\
					testchar++);							/*skip to white space*/\
				}\
				while (*testchar=='"' && testchar[1]!=' ' && testchar[1]!='\0' && testchar[1]!='\n' && testchar[1]!='\t');\
				if (*testchar)\
					*testchar++='\0';					/*turn to separate args*/\
			}\
		}\
		while (*testchar);\
	}\
	return argcount;									/*new number of args*/\
}\
\
inT32						global_exit_code;\
inT32						real_main(inT32,const char**);\
\
inT32						run_main(					/*the main thread*/\
CWinApp*					theapp						/*arguments*/\
)\
{\
	char					**argv;\
	char					*argsin[2];\
	inT32					argc;\
	inT32					exit_code;\
	\
	argsin[0]=strdup(theapp->m_pszExeName);\
	argsin[1]=strdup(theapp->m_lpCmdLine);\
/*allocate memory for the args. There can never be more than half*/\
/*the total number of characters in the arguments.*/\
	argv=(char**)malloc(((strlen(argsin[0])+strlen(argsin[1]))/2+1)*sizeof(char*));\
\
/*now construct argv as it should be for C.*/\
	argc=parse_args(2,argsin,argv);\
\
/*call main(argc,argv) here*/\
	exit_code=real_main(argc,(const char **)argv);\
\
\
/*now get rid of the main app window*/\
	if (theapp!=NULL && theapp->m_pMainWnd!=NULL)\
		PostMessage(theapp->m_pMainWnd->m_hWnd,WM_QUIT,0,0);\
	free(argsin[0]);\
	free(argsin[1]);\
	free(argv);\
	global_exit_code=exit_code;\
	return exit_code;\
}\
\
inT32						real_main(inT32 ARGC,const char* ARGV[])\

#else

#define REALLY_DECLARE_MAIN(ARGC,ARGV)\
\
/**********************************************************************\
* parse_args\
*\
* Turn a list of args into a new list of args with each separate\
* whitespace spaced string being an arg.\
**********************************************************************/\
\
inT32						parse_args(					/*refine arg list*/\
inT32						argc,						/*no of input args*/\
char						*argv[],					/*input args*/\
char						*arglist[]					/*output args*/\
)\
{\
	inT32					argcount;					/*converted argc*/\
	char					*testchar;					/*char in option string*/\
	inT32					arg;						/*current argument*/\
\
	argcount=0;											/*no of options*/\
	for (arg=0;arg<argc;arg++)\
	{\
		testchar=argv[arg];								/*start of arg*/\
		do\
		{\
			while (*testchar\
			&& (*testchar==' ' || *testchar=='"' || *testchar=='\n' || *testchar=='\t'))\
				testchar++;								/*skip white space*/\
			if (*testchar)\
			{\
				arglist[argcount++]=testchar;			/*new arg*/\
				do\
				{\
					for (testchar++;*testchar\
					&& *testchar!=' ' && *testchar!='"' && *testchar!='\n' && *testchar!='\t';\
					testchar++);							/*skip to white space*/\
				}\
				while (*testchar=='"' && testchar[1]!=' ' && testchar[1]!='\0' && testchar[1]!='\n' && testchar[1]!='\t');\
				if (*testchar)\
					*testchar++='\0';					/*turn to separate args*/\
			}\
		}\
		while (*testchar);\
	}\
	return argcount;									/*new number of args*/\
}\
\
inT32						main(inT32 ARGC,const char* ARGV[])\

#endif

#else
#error "NOT allowed to include nwmain.h or runmain.h twice!!"
#endif
