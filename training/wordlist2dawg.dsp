# Microsoft Developer Studio Project File - Name="wordlist2dawg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=wordlist2dawg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wordlist2dawg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wordlist2dawg.mak" CFG="wordlist2dawg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wordlist2dawg - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "wordlist2dawg - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wordlist2dawg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "w2d.Release6"
# PROP Intermediate_Dir "w2d.Release6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../ccutil" /I "../cutil" /I "../dict" /I "../viewer" /I "../ccstruct" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSW32__" /D "_AFXDLL" /Yu"mfcpch.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"./wordlist2dawg.exe"

!ELSEIF  "$(CFG)" == "wordlist2dawg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "w2d.Debug6"
# PROP Intermediate_Dir "w2d.Debug6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../ccutil" /I "../cutil" /I "../dict" /I "../viewer" /I "../ccstruct" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSW32__" /D "_AFXDLL" /Yu"mfcpch.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /out:"../bin.dbg6/wordlist2dawg.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "wordlist2dawg - Win32 Release"
# Name "wordlist2dawg - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ccutil"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ccutil\clst.cpp
# ADD CPP /Yu"mfcpch.h"
# End Source File
# Begin Source File

SOURCE=..\ccutil\debugwin.cpp
# ADD CPP /Yu"mfcpch.h"
# End Source File
# Begin Source File

SOURCE=..\ccutil\errcode.cpp
# ADD CPP /Yu"mfcpch.h"
# End Source File
# Begin Source File

SOURCE=..\ccutil\globaloc.cpp
# ADD CPP /Yu"mfcpch.h"
# End Source File
# Begin Source File

SOURCE=..\ccutil\hashfn.cpp
# ADD CPP /Yu"mfcpch.h"
# End Source File
# Begin Source File

SOURCE=..\ccutil\memblk.cpp
# ADD CPP /Yu"mfcpch.h"
# End Source File
# Begin Source File

SOURCE=..\ccutil\memry.cpp
# ADD CPP /Yu"mfcpch.h"
# End Source File
# Begin Source File

SOURCE=..\ccutil\mfcpch.cpp
# ADD CPP /Yc"mfcpch.h"
# End Source File
# Begin Source File

SOURCE=..\ccutil\serialis.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\strngs.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\tessopt.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\tprintf.cpp
# ADD CPP /Yu"mfcpch.h"
# End Source File
# Begin Source File

SOURCE=..\ccutil\unichar.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\ccutil\unicharmap.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\ccutil\unicharset.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\ccutil\varable.cpp
# End Source File
# End Group
# Begin Group "cutil"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\cutil\cutil.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\cutil\danerror.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\cutil\debug.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\cutil\emalloc.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\cutil\freelist.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\cutil\globals.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\cutil\listio.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\cutil\oldlist.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\cutil\structures.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\cutil\tordvars.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\cutil\variables.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "dict"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\dict\context.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\dict\dawg.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\dict\lookdawg.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\dict\makedawg.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\dict\reduce.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\dict\trie.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "ccstruct"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ccstruct\callcpp.cpp
# End Source File
# End Group
# Begin Group "viewer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\viewer\scrollview.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\viewer\svmnode.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\viewer\svutil.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Source File

SOURCE=.\wordlist2dawg.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\viewer\grphics.h
# End Source File
# Begin Source File

SOURCE=..\dict\lookdawg.h
# End Source File
# Begin Source File

SOURCE=..\dict\makedawg.h
# End Source File
# Begin Source File

SOURCE=..\dict\reduce.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
