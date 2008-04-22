# Microsoft Developer Studio Project File - Name="unicharset_extractor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=unicharset_extractor - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "unicharset_extractor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "unicharset_extractor.mak" CFG="unicharset_extractor - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "unicharset_extractor - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "unicharset_extractor - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "unicharset_extractor - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "uce.Release6"
# PROP Intermediate_Dir "uce.Release6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../ccutil" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSW32__" /D "_AFXDLL" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 /nologo /subsystem:console /machine:I386 /out:"./unicharset_extractor.exe"

!ELSEIF  "$(CFG)" == "unicharset_extractor - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "uce.Debug6"
# PROP Intermediate_Dir "uce.Debug6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../ccutil" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSW32__" /D "_AFXDLL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386 /out:"../bin.dbg6/unicharset_extractor.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "unicharset_extractor - Win32 Release"
# Name "unicharset_extractor - Win32 Debug"
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
# ADD CPP /Yu"mfcpch.h"
# End Source File
# Begin Source File

SOURCE=..\ccutil\tessopt.cpp
# ADD CPP /Yu"mfcpch.h"
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
# ADD CPP /Yu"mfcpch.h"
# End Source File
# End Group
# Begin Source File

SOURCE=..\ccutil\boxread.cpp
# ADD CPP /Yu"mfcpch.h"
# End Source File
# Begin Source File

SOURCE=.\unicharset_extractor.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\ccutil\basedir.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\bits16.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\clst.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\debugwin.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\elst.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\elst2.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\errcode.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\fileerr.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\globaloc.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\hashfn.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\host.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\hosthplb.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\lsterr.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\mainblk.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\memblk.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\memry.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\memryerr.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\mfcpch.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\ndminx.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\notdll.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\nwmain.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\ocrclass.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\ocrshell.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\platform.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\scanutils.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\secname.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\serialis.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\stderr.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\strngs.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\tessclas.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\tessopt.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\tprintf.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\unichar.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\unicharmap.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\unicharset.h
# End Source File
# Begin Source File

SOURCE=..\ccutil\varable.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
