# Microsoft Developer Studio Project File - Name="cnTraining" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=cnTraining - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cnTraining.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cnTraining.mak" CFG="cnTraining - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cnTraining - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "cnTraining - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cnTraining - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "cntrain.Release6"
# PROP Intermediate_Dir "cntrain.Release6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../ccutil" /I "../ccstruct" /I "../classify" /I "../cutil" /I "../training" /I "../viewer" /I "../dict" /D "TRAINING" /D "WIN32" /D "_CONSOLE" /D "__NT__" /D "__MSW32__" /D "_AFXDLL" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib /nologo /subsystem:console /profile /debug /machine:I386 /out:"./cnTraining.exe"

!ELSEIF  "$(CFG)" == "cnTraining - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "cntrain.Debug6"
# PROP Intermediate_Dir "cntrain.Debug6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../ccutil" /I "../ccstruct" /I "../classify" /I "../cutil" /I "../training" /I "../viewer" /I "../dict" /D "_DEBUG" /D "TRAINING" /D "WIN32" /D "_CONSOLE" /D "__NT__" /D "__MSW32__" /D "_AFXDLL" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /out:"../bin.dbg6/cnTraining.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "cnTraining - Win32 Release"
# Name "cnTraining - Win32 Debug"
# Begin Source File

SOURCE=..\ccutil\clst.cpp
# End Source File
# Begin Source File

SOURCE=..\classify\cluster.cpp
# End Source File
# Begin Source File

SOURCE=..\classify\clusttool.cpp
# End Source File
# Begin Source File

SOURCE=.\cnTraining.cpp
# End Source File
# Begin Source File

SOURCE=..\cutil\cutil.cpp
# End Source File
# Begin Source File

SOURCE=..\cutil\danerror.cpp
# End Source File
# Begin Source File

SOURCE=..\cutil\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\debugwin.cpp
# End Source File
# Begin Source File

SOURCE=..\cutil\efio.cpp
# End Source File
# Begin Source File

SOURCE=..\cutil\emalloc.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\errcode.cpp
# End Source File
# Begin Source File

SOURCE=..\classify\featdefs.cpp
# End Source File
# Begin Source File

SOURCE=..\cutil\freelist.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\globaloc.cpp
# End Source File
# Begin Source File

SOURCE=..\cutil\globals.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\hashfn.cpp
# End Source File
# Begin Source File

SOURCE=..\classify\kdtree.cpp
# End Source File
# Begin Source File

SOURCE=..\cutil\listio.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\memblk.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\memry.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\mfcpch.cpp
# End Source File
# Begin Source File

SOURCE=..\training\name2char.cpp
# End Source File
# Begin Source File

SOURCE=..\classify\ocrfeatures.cpp
# End Source File
# Begin Source File

SOURCE=..\cutil\oldheap.cpp
# End Source File
# Begin Source File

SOURCE=..\cutil\oldlist.cpp
# End Source File
# Begin Source File

SOURCE=..\viewer\scrollview.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\serialis.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\strngs.cpp
# End Source File
# Begin Source File

SOURCE=..\cutil\structures.cpp
# End Source File
# Begin Source File

SOURCE=..\viewer\svmnode.cpp
# End Source File
# Begin Source File

SOURCE=..\viewer\svutil.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\tessopt.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\tprintf.cpp
# End Source File
# Begin Source File

SOURCE=..\training\training.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\unichar.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\unicharmap.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\unicharset.cpp
# End Source File
# Begin Source File

SOURCE=..\ccutil\varable.cpp
# End Source File
# Begin Source File

SOURCE=..\cutil\variables.cpp
# End Source File
# End Target
# End Project
