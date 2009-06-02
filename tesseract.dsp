# Microsoft Developer Studio Project File - Name="tesseract" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=tesseract - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tesseract.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tesseract.mak" CFG="tesseract - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tesseract - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "tesseract - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tesseract - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "tess.Release6"
# PROP Intermediate_Dir "tess.Release6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "aspirin" /I "ccutil" /I "ccstruct" /I "classify" /I "cutil" /I "dict" /I "display" /I "image" /I "textord" /I "viewer" /I "wordrec" /I "." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSW32__" /D "_AFXDLL" /Yu"mfcpch.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"./tesseract.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "tesseract - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "tess.Debug6"
# PROP Intermediate_Dir "tess.Debug6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "aspirin" /I "ccutil" /I "ccstruct" /I "classify" /I "cutil" /I "dict" /I "display" /I "image" /I "textord" /I "viewer" /I "wordrec" /I "." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSW32__" /D "_AFXDLL" /Yu"mfcpch.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /out:"bin.dbg6/tesseract.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "tesseract - Win32 Release"
# Name "tesseract - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ccmain"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ccmain\adaptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\applybox.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\baseapi.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\ccmain\blobcmp.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\callnet.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\charcut.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\charsample.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\control.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\docqual.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\expandblob.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\fixspace.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\fixxht.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\imgscale.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\matmatch.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\output.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\pagewalk.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\pagewalk.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\paircmp.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\pgedit.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\ccmain\pgedit.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\reject.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\scaleimg.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\tessbox.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\tessedit.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\tesseractmain.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\tessvars.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\tfacepp.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\tstruct.cpp
# End Source File
# Begin Source File

SOURCE=.\ccmain\varabled.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\ccmain\varabled.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\werdit.cpp
# End Source File
# End Group
# Begin Group "ccstruct"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ccstruct\blobbox.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\blobs.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\blread.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\callcpp.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\coutln.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\genblob.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\labls.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\linlsq.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\lmedsq.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\mod128.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\normalis.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\ocrblock.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\ocrrow.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\pageblk.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\pageres.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\pdblock.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\points.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\polyaprx.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\polyblk.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\polyblob.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\polyvert.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\poutline.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\quadlsq.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\quadratc.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\quspline.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\ratngs.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\rect.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\rejctmap.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\rwpoly.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\statistc.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\stepblob.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\txtregn.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\vecfuncs.cpp
# End Source File
# Begin Source File

SOURCE=.\ccstruct\werd.cpp
# End Source File
# End Group
# Begin Group "ccutil"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ccutil\basedir.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\bits16.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\boxread.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\clst.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\debugwin.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\elst.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\elst2.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\errcode.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\globaloc.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\hashfn.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\mainblk.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\memblk.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\memry.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\mfcpch.cpp
# ADD CPP /Yc"mfcpch.h"
# End Source File
# Begin Source File

SOURCE=.\ccutil\ocrshell.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\serialis.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\strngs.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\tessopt.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\tprintf.cpp
# End Source File
# Begin Source File

SOURCE=.\ccutil\unichar.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\ccutil\unicharmap.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\ccutil\unicharset.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\ccutil\varable.cpp
# End Source File
# End Group
# Begin Group "classify"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\classify\adaptive.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\adaptmatch.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\baseline.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\blobclass.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\chartoname.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\cluster.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\clusttool.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\cutoffs.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\extract.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\featdefs.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\flexfx.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\float2int.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\fpoint.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\fxdefs.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\hideedge.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\intfx.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\intmatcher.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\intproto.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\kdtree.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\mf.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\mfdefs.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\mfoutline.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\mfx.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\normfeat.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\normmatch.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\ocrfeatures.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\outfeat.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\picofeat.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\protos.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\sigmenu.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\speckle.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\classify\xform2d.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "cutil"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cutil\bitvec.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\cutil.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\danerror.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\debug.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\efio.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\emalloc.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\freelist.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\globals.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\listio.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\oldheap.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\oldlist.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\structures.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\tessarray.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\tordvars.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\cutil\variables.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "dict"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dict\choices.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\dict\context.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\dict\dawg.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\dict\hyphen.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\dict\permdawg.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\dict\permngram.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\dict\permnum.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\dict\permute.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\dict\states.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\dict\stopper.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\dict\trie.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "image"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\image\bitstrm.cpp
# End Source File
# Begin Source File

SOURCE=.\image\imgbmp.cpp
# End Source File
# Begin Source File

SOURCE=.\image\imgio.cpp
# End Source File
# Begin Source File

SOURCE=.\image\imgs.cpp
# End Source File
# Begin Source File

SOURCE=.\image\imgtiff.cpp
# End Source File
# Begin Source File

SOURCE=.\image\svshowim.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "textord"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\textord\blkocc.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\drawedg.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\drawtord.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\edgblob.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\edgloop.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\fpchop.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\gap_map.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\makerow.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\oldbasel.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\pithsync.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\pitsync1.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\scanedg.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\sortflts.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\topitch.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\tordmain.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\tospace.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\tovars.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\underlin.cpp
# ADD CPP /I "pageseg"
# End Source File
# Begin Source File

SOURCE=.\textord\wordseg.cpp
# ADD CPP /I "pageseg"
# End Source File
# End Group
# Begin Group "viewer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\viewer\scrollview.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\viewer\scrollview.h
# End Source File
# Begin Source File

SOURCE=.\viewer\svmnode.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\viewer\svmnode.h
# End Source File
# Begin Source File

SOURCE=.\viewer\svutil.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\viewer\svutil.h
# End Source File
# End Group
# Begin Group "wordrec"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wordrec\associate.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\badwords.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\bestfirst.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\chop.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\chopper.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\closed.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\djmenus.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\drawfx.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\findseam.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\gradechop.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\heuristic.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\makechop.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\matchtab.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\matrix.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\metrics.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\mfvars.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\msmenus.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\olutil.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\outlines.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\pieces.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\plotedges.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\plotseg.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\render.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\seam.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\split.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\tally.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\tessinit.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\tface.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\wordrec\wordclass.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "ccmain header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ccmain\adaptions.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\applybox.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\baseapi.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\blobcmp.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\callnet.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\charcut.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\charsample.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\control.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\docqual.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\expandblob.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\fixspace.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\fixxht.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\imgscale.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\matmatch.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\output.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\paircmp.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\reject.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\scaleimg.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\tessbox.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\tessedit.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\tessembedded.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\tesseractmain.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\tessvars.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\tfacep.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\tfacepp.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\tstruct.h
# End Source File
# Begin Source File

SOURCE=.\ccmain\werdit.h
# End Source File
# End Group
# Begin Group "ccstruct header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ccstruct\blckerr.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\blobbox.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\blobs.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\blread.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\coutln.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\crakedge.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\genblob.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\hpddef.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\hpdsizes.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\ipoints.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\labls.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\linlsq.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\lmedsq.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\mod128.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\normalis.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\ocrblock.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\ocrrow.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\pageblk.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\pageres.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\pdblock.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\pdclass.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\points.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\polyaprx.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\polyblk.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\polyblob.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\polyvert.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\poutline.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\quadlsq.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\quadratc.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\quspline.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\ratngs.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\rect.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\rejctmap.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\rwpoly.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\statistc.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\stepblob.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\txtregn.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\vecfuncs.h
# End Source File
# Begin Source File

SOURCE=.\ccstruct\werd.h
# End Source File
# End Group
# Begin Group "ccutil header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cutil\array.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\basedir.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\bits16.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\clst.h
# End Source File
# Begin Source File

SOURCE=.\cutil\cutil.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\debugwin.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\elst.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\elst2.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\errcode.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\fileerr.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\globaloc.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\hashfn.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\host.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\hosthplb.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\lsterr.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\mainblk.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\memblk.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\memry.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\memryerr.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\mfcpch.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\ndminx.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\notdll.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\nwmain.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\ocrclass.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\ocrshell.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\platform.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\scanutils.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\secname.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\serialis.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\stderr.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\strngs.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\tessopt.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\tprintf.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\unichar.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\unicharmap.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\unicharset.h
# End Source File
# Begin Source File

SOURCE=.\ccutil\varable.h
# End Source File
# End Group
# Begin Group "classify header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\classify\adaptive.h
# End Source File
# Begin Source File

SOURCE=.\classify\adaptmatch.h
# End Source File
# Begin Source File

SOURCE=.\classify\baseline.h
# End Source File
# Begin Source File

SOURCE=.\classify\blobclass.h
# End Source File
# Begin Source File

SOURCE=.\classify\chartoname.h
# End Source File
# Begin Source File

SOURCE=.\classify\cluster.h
# End Source File
# Begin Source File

SOURCE=.\classify\clusttool.h
# End Source File
# Begin Source File

SOURCE=.\classify\cutoffs.h
# End Source File
# Begin Source File

SOURCE=.\classify\extern.h
# End Source File
# Begin Source File

SOURCE=.\classify\extract.h
# End Source File
# Begin Source File

SOURCE=.\classify\featdefs.h
# End Source File
# Begin Source File

SOURCE=.\classify\flexfx.h
# End Source File
# Begin Source File

SOURCE=.\classify\float2int.h
# End Source File
# Begin Source File

SOURCE=.\classify\fpoint.h
# End Source File
# Begin Source File

SOURCE=.\classify\fxdefs.h
# End Source File
# Begin Source File

SOURCE=.\classify\fxid.h
# End Source File
# Begin Source File

SOURCE=.\classify\hideedge.h
# End Source File
# Begin Source File

SOURCE=.\classify\intfx.h
# End Source File
# Begin Source File

SOURCE=.\classify\intmatcher.h
# End Source File
# Begin Source File

SOURCE=.\classify\intproto.h
# End Source File
# Begin Source File

SOURCE=.\classify\kdtree.h
# End Source File
# Begin Source File

SOURCE=.\classify\matchdefs.h
# End Source File
# Begin Source File

SOURCE=.\classify\mf.h
# End Source File
# Begin Source File

SOURCE=.\classify\mfdefs.h
# End Source File
# Begin Source File

SOURCE=.\classify\mfoutline.h
# End Source File
# Begin Source File

SOURCE=.\classify\mfx.h
# End Source File
# Begin Source File

SOURCE=.\classify\normfeat.h
# End Source File
# Begin Source File

SOURCE=.\classify\normmatch.h
# End Source File
# Begin Source File

SOURCE=.\classify\ocrfeatures.h
# End Source File
# Begin Source File

SOURCE=.\classify\outfeat.h
# End Source File
# Begin Source File

SOURCE=.\classify\picofeat.h
# End Source File
# Begin Source File

SOURCE=.\classify\protos.h
# End Source File
# Begin Source File

SOURCE=.\classify\sigmenu.h
# End Source File
# Begin Source File

SOURCE=.\classify\speckle.h
# End Source File
# Begin Source File

SOURCE=.\classify\xform2d.h
# End Source File
# End Group
# Begin Group "cutil header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cutil\bitvec.h
# End Source File
# Begin Source File

SOURCE=.\cutil\callcpp.h
# End Source File
# Begin Source File

SOURCE=.\cutil\const.h
# End Source File
# Begin Source File

SOURCE=.\cutil\danerror.h
# End Source File
# Begin Source File

SOURCE=.\cutil\debug.h
# End Source File
# Begin Source File

SOURCE=.\cutil\efio.h
# End Source File
# Begin Source File

SOURCE=.\cutil\emalloc.h
# End Source File
# Begin Source File

SOURCE=.\cutil\freelist.h
# End Source File
# Begin Source File

SOURCE=.\cutil\funcdefs.h
# End Source File
# Begin Source File

SOURCE=.\cutil\general.h
# End Source File
# Begin Source File

SOURCE=.\cutil\globals.h
# End Source File
# Begin Source File

SOURCE=.\cutil\listio.h
# End Source File
# Begin Source File

SOURCE=.\cutil\oldheap.h
# End Source File
# Begin Source File

SOURCE=.\cutil\oldlist.h
# End Source File
# Begin Source File

SOURCE=.\cutil\structures.h
# End Source File
# Begin Source File

SOURCE=.\cutil\tessarray.h
# End Source File
# Begin Source File

SOURCE=.\cutil\tordvars.h
# End Source File
# Begin Source File

SOURCE=.\cutil\util.h
# End Source File
# Begin Source File

SOURCE=.\cutil\variables.h
# End Source File
# End Group
# Begin Group "dict header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dict\choicearr.h
# End Source File
# Begin Source File

SOURCE=.\dict\choices.h
# End Source File
# Begin Source File

SOURCE=.\dict\context.h
# End Source File
# Begin Source File

SOURCE=.\dict\dawg.h
# End Source File
# Begin Source File

SOURCE=.\dict\hyphen.h
# End Source File
# Begin Source File

SOURCE=.\dict\matchdefs.h
# End Source File
# Begin Source File

SOURCE=.\dict\permdawg.h
# End Source File
# Begin Source File

SOURCE=.\dict\permnum.h
# End Source File
# Begin Source File

SOURCE=.\dict\permute.h
# End Source File
# Begin Source File

SOURCE=.\dict\states.h
# End Source File
# Begin Source File

SOURCE=.\dict\stopper.h
# End Source File
# Begin Source File

SOURCE=.\dict\trie.h
# End Source File
# End Group
# Begin Group "image header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\image\bitstrm.h
# End Source File
# Begin Source File

SOURCE=.\image\img.h
# End Source File
# Begin Source File

SOURCE=.\image\imgbmp.h
# End Source File
# Begin Source File

SOURCE=.\image\imgerrs.h
# End Source File
# Begin Source File

SOURCE=.\image\imgio.h
# End Source File
# Begin Source File

SOURCE=.\image\imgs.h
# End Source File
# Begin Source File

SOURCE=.\image\imgtiff.h
# End Source File
# Begin Source File

SOURCE=.\image\imgunpk.h
# End Source File
# Begin Source File

SOURCE=.\image\svshowim.h
# End Source File
# End Group
# Begin Group "textord header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\textord\blkocc.h
# End Source File
# Begin Source File

SOURCE=.\textord\blobcmpl.h
# End Source File
# Begin Source File

SOURCE=.\textord\drawedg.h
# End Source File
# Begin Source File

SOURCE=.\textord\drawtord.h
# End Source File
# Begin Source File

SOURCE=.\textord\edgblob.h
# End Source File
# Begin Source File

SOURCE=.\textord\edgloop.h
# End Source File
# Begin Source File

SOURCE=.\textord\fpchop.h
# End Source File
# Begin Source File

SOURCE=.\textord\gap_map.h
# End Source File
# Begin Source File

SOURCE=.\textord\makerow.h
# End Source File
# Begin Source File

SOURCE=.\textord\oldbasel.h
# End Source File
# Begin Source File

SOURCE=.\textord\pithsync.h
# End Source File
# Begin Source File

SOURCE=.\textord\pitsync1.h
# End Source File
# Begin Source File

SOURCE=.\textord\scanedg.h
# End Source File
# Begin Source File

SOURCE=.\textord\sortflts.h
# End Source File
# Begin Source File

SOURCE=.\textord\tessout.h
# End Source File
# Begin Source File

SOURCE=.\textord\topitch.h
# End Source File
# Begin Source File

SOURCE=.\textord\tordmain.h
# End Source File
# Begin Source File

SOURCE=.\textord\tospace.h
# End Source File
# Begin Source File

SOURCE=.\textord\tovars.h
# End Source File
# Begin Source File

SOURCE=.\textord\underlin.h
# End Source File
# Begin Source File

SOURCE=.\textord\wordseg.h
# End Source File
# End Group
# Begin Group "viewer header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\viewer\evntlst.h
# End Source File
# Begin Source File

SOURCE=.\viewer\evnts.h
# End Source File
# Begin Source File

SOURCE=.\viewer\grphics.h
# End Source File
# Begin Source File

SOURCE=.\viewer\grphshm.h
# End Source File
# Begin Source File

SOURCE=.\viewer\sbgconst.h
# End Source File
# Begin Source File

SOURCE=.\viewer\sbgdefs.h
# End Source File
# Begin Source File

SOURCE=.\viewer\sbgtypes.h
# End Source File
# Begin Source File

SOURCE=.\viewer\showim.h
# End Source File
# End Group
# Begin Group "wordrec header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wordrec\associate.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\badwords.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\bestfirst.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\bitvec.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\charsample.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\chop.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\chopper.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\closed.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\control.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\djmenus.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\drawfx.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\findseam.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\gradechop.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\heuristic.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\makechop.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\matchtab.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\matrix.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\measure.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\metrics.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\mfvars.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\msmenus.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\olutil.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\outlines.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\pieces.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\plotedges.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\plotseg.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\render.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\seam.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\split.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\tally.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\tessinit.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\tface.h
# End Source File
# Begin Source File

SOURCE=.\wordrec\wordclass.h
# End Source File
# End Group
# Begin Group "pageseg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pageseg\leptonica_pageseg.cpp
# ADD CPP /I "ccmain"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pageseg\leptonica_pageseg.h
# End Source File
# Begin Source File

SOURCE=.\pageseg\leptonica_pageseg_interface.cpp
# ADD CPP /I "ccmain"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pageseg\leptonica_pageseg_interface.h
# End Source File
# Begin Source File

SOURCE=.\pageseg\pageseg.cpp
# ADD CPP /I "ccmain"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pageseg\pageseg.h
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ccutil\boxread.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
