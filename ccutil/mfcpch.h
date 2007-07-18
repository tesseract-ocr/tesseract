// mfcpch.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
// For Unix and mac the file does nothing. It needs to be included in all cpp
// files for compatibility with the PC pre-compiled header mechanism.
#ifdef __MSW32__
#ifndef _AFXDLL
#define WIN32_LEAN_AND_MEAN
#define STRICT 1
//#include <windows.h>
#include <stdlib.h>
#else
#define VC_EXTRALEAN             // Exclude rarely-used stuff from Windows headers

#include <afxwin.h>              // mfc core and standard components
#include <afxext.h>              // mfc extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>              // mfc ole classes
#include <afxodlgs.h>            // mfc ole dialog classes
#include <afxdisp.h>             // mfc ole automation classes
#endif                           // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>               // mfc odbc database classes
#endif                           // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>              // mfc dao database classes
#endif                           // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>              // mfc support for windows 95 common controls
#endif                           // _AFX_NO_AFXCMN_SUPPORT
#endif
#endif
