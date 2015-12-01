#!/usr/bin/env python
from __future__ import print_function
from builtins import input
"""
tesshelper.py -- Utility operations to compare, report stats, and copy 
                 public headers for tesseract 3.0x VS2010 Project

$RCSfile: tesshelper.py,v $ $Revision: 7ca575b377aa $ $Date: 2012/03/07 17:26:31 $
"""

r"""
Requires:

 python 2.7 or greater: activestate.com
  http://www.activestate.com/activepython/downloads

because using the new argparse module and new literal set syntax (s={1, 2}) .

General Notes:
--------------

Format for a .vcproj file entry:

			<File
				RelativePath="..\src\allheaders.h"
				>
			</File>

"""

epilogStr = r"""
Examples:

Assume that tesshelper.py is in c:\buildfolder\tesseract-3.02\vs2010,
which is also the current directory. Then,

    python tesshelper .. compare
    
will compare c:\buildfolder\tesseract-3.02 "library" directories to the
libtesseract Project 
(c:\buildfolder\tesseract-3.02\vs2010\libtesseract\libtesseract.vcproj).

    python tesshelper ..  report
    
will display summary stats for c:\buildfolder\tesseract-3.02 "library" 
directories and the libtesseract Project.

    python tesshelper .. copy ..\..\include

will copy all "public" libtesseract header files to
c:\buildfolder\include.

    python tesshelper .. clean

will clean the vs2010 folder of all build directories, and .user, .suo,
.ncb, and other temp files.

"""

# imports of python standard library modules
# See Python Documentation | Library Reference for details
import collections
import glob
import argparse
import os
import re
import shutil
import sys

# ====================================================================

VERSION = "1.0 %s" % "$Date: 2012/03/07 17:26:31 $".split()[1]
PROJ_SUBDIR = r"vs2010\libtesseract"
PROJFILE = "libtesseract.vcproj"

NEWHEADERS_FILENAME = "newheaders.txt"
NEWSOURCES_FILENAME = "newsources.txt"

fileNodeTemplate = \
'''    <ClCompile Include="..\..\%s" />'''

# ====================================================================

def getProjectfiles(libTessDir, libProjectFile, nTrimChars):
    """Return sets of all, c, h, and resources files in libtesseract Project"""

    #extract filenames of header & source files from the .vcproj
    projectCFiles = set()
    projectHFiles = set()
    projectRFiles = set()
    projectFilesSet = set()
    f = open(libProjectFile, "r")
    data = f.read()
    f.close()

    projectFiles = re.findall(r'(?i)Include="(\.[^"]+)"', data)
    for projectFile in projectFiles:
        root, ext = os.path.splitext(projectFile.lower())
        if ext == ".c" or ext == ".cpp":
            projectCFiles.add(projectFile)
        elif ext == ".h":
            projectHFiles.add(projectFile)
        elif ext == ".rc":
            projectRFiles.add(projectFile)
        else:
            print("unknown file type: %s" % projectFile)
            
        relativePath = os.path.join(libTessDir, projectFile)
        relativePath = os.path.abspath(relativePath)
        relativePath = relativePath[nTrimChars:].lower()
        projectFilesSet.add(relativePath)
    
    return projectFilesSet, projectHFiles, projectCFiles, projectRFiles
    
def getTessLibFiles(tessDir, nTrimChars):
    """Return set of all libtesseract files in tessDir"""

    libDirs = [
        "api",
        "ccmain",
        "ccstruct",
        "ccutil",
        "classify",
        "cube",
        "cutil",
        "dict",
        r"neural_networks\runtime",
        "opencl",
        "textord",
        "viewer",
        "wordrec",
        #"training",
        r"vs2010\port",
        r"vs2010\libtesseract",
        ]

    #create list of all .h, .c, .cpp files in "library" directories
    tessFiles = set()
    for curDir in libDirs:
        baseDir = os.path.join(tessDir, curDir)
        for filetype in ["*.c", "*.cpp", "*.h"]:
            pattern = os.path.join(baseDir, filetype)
            fileList = glob.glob(pattern)
            for curFile in fileList:
                curFile = os.path.abspath(curFile)
                relativePath = curFile[nTrimChars:].lower()
                tessFiles.add(relativePath)

    return tessFiles

# ====================================================================

def tessCompare(tessDir):
    '''Compare libtesseract Project files and actual "sub-library" files.'''

    vs2010Dir = os.path.join(tessDir, "vs2010")
    libTessDir = os.path.join(vs2010Dir, "libtesseract")
    libProjectFile = os.path.join(libTessDir,"libtesseract.vcxproj")
    tessAbsDir = os.path.abspath(tessDir)
    nTrimChars = len(tessAbsDir)+1
    print('Comparing VS2010 Project "%s" with\n   "%s"' % (libProjectFile,
                                                           tessAbsDir))
    
    projectFilesSet, projectHFiles, projectCFiles, projectRFiles = \
        getProjectfiles(libTessDir, libProjectFile, nTrimChars)
    tessFiles = getTessLibFiles(tessDir, nTrimChars)
 
    extraFiles = tessFiles - projectFilesSet
    print("%2d Extra files (in %s but not in Project)" % (len(extraFiles),
                                                          tessAbsDir))
    headerFiles = []
    sourceFiles = []
    sortedList = list(extraFiles)
    sortedList.sort()
    for filename in sortedList:
        root, ext = os.path.splitext(filename.lower())
        if ext == ".h":
            headerFiles.append(filename)
        else:
            sourceFiles.append(filename)
        print("   %s " % filename)

    print()
    print("%2d new header file items written to %s" % (len(headerFiles),
                                                    NEWHEADERS_FILENAME))
    headerFiles.sort()
    with open(NEWHEADERS_FILENAME, "w") as f:
        for filename in headerFiles:
            f.write(fileNodeTemplate % filename)

    print("%2d new source file items written to %s" % (len(sourceFiles),
                                                    NEWSOURCES_FILENAME))
    sourceFiles.sort()
    with open(NEWSOURCES_FILENAME, "w") as f:
        for filename in sourceFiles:
            f.write(fileNodeTemplate % filename)
    print()

    deadFiles = projectFilesSet - tessFiles
    print("%2d Dead files (in Project but not in %s" % (len(deadFiles),
                                                        tessAbsDir))
    sortedList = list(deadFiles)
    sortedList.sort()
    for filename in sortedList:
        print("   %s " % filename)
        
# ====================================================================

def tessReport(tessDir):
    """Report summary stats on "sub-library" files and libtesseract Project file."""

    vs2010Dir = os.path.join(tessDir, "vs2010")
    libTessDir = os.path.join(vs2010Dir, "libtesseract")
    libProjectFile = os.path.join(libTessDir,"libtesseract.vcproj")
    tessAbsDir = os.path.abspath(tessDir)
    nTrimChars = len(tessAbsDir)+1
    
    projectFilesSet, projectHFiles, projectCFiles, projectRFiles = \
        getProjectfiles(libTessDir, libProjectFile, nTrimChars)
    tessFiles = getTessLibFiles(tessDir, nTrimChars)

    print('Summary stats for "%s" library directories' % tessAbsDir)
    folderCounters = {}
    for tessFile in tessFiles:
        tessFile = tessFile.lower()
        folder, head = os.path.split(tessFile)
        file, ext = os.path.splitext(head)
        typeCounter = folderCounters.setdefault(folder, collections.Counter())
        typeCounter[ext[1:]] += 1

    folders = list(folderCounters.keys())
    folders.sort()
    totalFiles = 0
    totalH = 0
    totalCPP = 0
    totalOther = 0

    print()
    print(" total  h  cpp")
    print(" ----- --- ---")
    for folder in folders:
        counters = folderCounters[folder]
        nHFiles = counters['h']
        nCPPFiles = counters['cpp']
        
        total = nHFiles + nCPPFiles
        totalFiles += total
        totalH += nHFiles
        totalCPP += nCPPFiles
        
        print(" %5d %3d %3d %s" % (total, nHFiles, nCPPFiles, folder))
    print(" ----- --- ---")
    print(" %5d %3d %3d" % (totalFiles, totalH, totalCPP))
    
    print()
    print('Summary stats for VS2010 Project "%s"' % libProjectFile)
    print(" %5d %s" %(len(projectHFiles), "Header files"))
    print(" %5d %s" % (len(projectCFiles), "Source files"))
    print(" %5d %s" % (len(projectRFiles), "Resource files"))
    print(" -----")
    print(" %5d" % (len(projectHFiles) + len(projectCFiles) + len(projectRFiles), ))
        
# ====================================================================

def copyIncludes(fileSet, description, tessDir, includeDir):
    """Copy set of files to specified include dir."""
    
    print()
    print('Copying libtesseract "%s" headers to %s' % (description, includeDir))
    print()

    sortedList = list(fileSet)
    sortedList.sort()
    
    count = 0
    errList = []
    for includeFile in sortedList:
        filepath = os.path.join(tessDir, includeFile)
        if os.path.isfile(filepath):
            shutil.copy2(filepath, includeDir)
            print("Copied: %s" % includeFile)
            count += 1
        else:
            print('***Error: "%s" doesn\'t exist"' % filepath)
            errList.append(filepath)
            
    print('%d header files successfully copied to "%s"' % (count, includeDir))
    if len(errList):
        print("The following %d files were not copied:")
        for filepath in errList:
            print("   %s" % filepath)
    
def tessCopy(tessDir, includeDir):
    '''Copy all "public" libtesseract Project header files to include directory.
    
    Preserves directory hierarchy.'''

    baseIncludeSet = {
        r"api\baseapi.h",
        r"api\capi.h",
        r"api\apitypes.h",
        r"ccstruct\publictypes.h",
        r"ccmain\thresholder.h",
        r"ccutil\host.h",
        r"ccutil\basedir.h",
        r"ccutil\tesscallback.h",
        r"ccutil\unichar.h",
        r"ccutil\platform.h",
        }

    strngIncludeSet = {
        r"ccutil\strngs.h",
        r"ccutil\memry.h",
        r"ccutil\host.h",
        r"ccutil\serialis.h",
        r"ccutil\errcode.h",
        r"ccutil\fileerr.h",
        #r"ccutil\genericvector.h",
        }

    resultIteratorIncludeSet = {
        r"ccmain\ltrresultiterator.h",
        r"ccmain\pageiterator.h",
        r"ccmain\resultiterator.h",
        r"ccutil\genericvector.h",
        r"ccutil\tesscallback.h",
        r"ccutil\errcode.h",
        r"ccutil\host.h",
        r"ccutil\helpers.h",
        r"ccutil\ndminx.h",
        r"ccutil\params.h",
        r"ccutil\unicharmap.h",
        r"ccutil\unicharset.h",
        }

    genericVectorIncludeSet = {
        r"ccutil\genericvector.h",
        r"ccutil\tesscallback.h",
        r"ccutil\errcode.h",
        r"ccutil\host.h",
        r"ccutil\helpers.h",
        r"ccutil\ndminx.h",
        }
    
    blobsIncludeSet = {
        r"ccstruct\blobs.h",
        r"ccstruct\rect.h",
        r"ccstruct\points.h",
        r"ccstruct\ipoints.h",
        r"ccutil\elst.h",
        r"ccutil\host.h",
        r"ccutil\serialis.h",
        r"ccutil\lsterr.h",
        r"ccutil\ndminx.h",
        r"ccutil\tprintf.h",
        r"ccutil\params.h",
        r"viewer\scrollview.h",
        r"ccstruct\vecfuncs.h",
        }

    extraFilesSet = {
        #r"vs2010\include\stdint.h",
        r"vs2010\include\leptonica_versionnumbers.vsprops",
        r"vs2010\include\tesseract_versionnumbers.vsprops",
        }
        
    tessIncludeDir = os.path.join(includeDir, "tesseract")
    if os.path.isfile(tessIncludeDir):
        print('Aborting: "%s" is a file not a directory.' % tessIncludeDir)
        return
    if not os.path.exists(tessIncludeDir):
        os.mkdir(tessIncludeDir)
    
    #fileSet = baseIncludeSet | strngIncludeSet | genericVectorIncludeSet | blobsIncludeSet
    fileSet = baseIncludeSet | strngIncludeSet | resultIteratorIncludeSet

    copyIncludes(fileSet, "public", tessDir, tessIncludeDir)
    copyIncludes(extraFilesSet, "extra", tessDir, includeDir)

# ====================================================================

def tessClean(tessDir):
    '''Clean vs2010 folder of all build directories and certain temp files.'''

    vs2010Dir = os.path.join(tessDir, "vs2010")
    vs2010AbsDir = os.path.abspath(vs2010Dir)

    answer = eval(input(
        'Are you sure you want to clean the\n "%s" folder (Yes/No) [No]? ' % 
        vs2010AbsDir))
    if answer.lower() not in ("yes",):
        return
    answer = eval(input('Only list the items to be deleted (Yes/No) [Yes]? '))
    answer = answer.strip()
    listOnly = answer.lower() not in ("no",)

    for rootDir, dirs, files in os.walk(vs2010AbsDir):
        for buildDir in ("LIB_Release", "LIB_Debug", "DLL_Release", "DLL_Debug"):
            if buildDir in dirs:
                dirs.remove(buildDir)
                absBuildDir = os.path.join(rootDir, buildDir)
                if listOnly:
                    print("Would remove: %s" % absBuildDir)
                else:
                    print("Removing: %s" % absBuildDir)
                    shutil.rmtree(absBuildDir)

        if rootDir == vs2010AbsDir:
            for file in files:
                if file.lower() not in ("tesseract.sln",
                                        "tesshelper.py",
                                        "readme.txt"):
                    absPath = os.path.join(rootDir, file)
                    if listOnly:
                        print("Would remove: %s" % absPath)
                    else:
                        print("Removing: %s" % absPath)
                        os.remove(absPath)
        else:
            for file in files:
                root, ext = os.path.splitext(file)
                if ext.lower() in (".suo",
                                   ".ncb",
                                   ".user",
                                   ) or (
                    len(ext)>0 and ext[-1] == "~"):
                    absPath = os.path.join(rootDir, file)
                    if listOnly:
                        print("Would remove: %s" % absPath)
                    else:
                        print("Removing: %s" % absPath)
                        os.remove(absPath)

# ====================================================================

def validateTessDir(tessDir):
    """Check that tessDir is a valid tesseract directory."""
    
    if not os.path.isdir(tessDir):
        raise argparse.ArgumentTypeError('Directory "%s" doesn\'t exist.' % tessDir)
    projFile = os.path.join(tessDir, PROJ_SUBDIR, PROJFILE)
    if not os.path.isfile(projFile):
        raise argparse.ArgumentTypeError('Project file "%s" doesn\'t exist.' % projFile)
    return tessDir

def validateDir(dir):
    """Check that dir is a valid directory named include."""
    
    if not os.path.isdir(dir):
        raise argparse.ArgumentTypeError('Directory "%s" doesn\'t exist.' % dir)
    
    dirpath = os.path.abspath(dir)
    head, tail = os.path.split(dirpath)
    if tail.lower() != "include":
        raise argparse.ArgumentTypeError('Include directory "%s" must be named "include".' % tail)
        
    return dir

def main ():
    parser = argparse.ArgumentParser(
        epilog=epilogStr,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    
    parser.add_argument("--version", action="version",
                        version="%(prog)s " + VERSION)
    parser.add_argument('tessDir', type=validateTessDir,
                        help="tesseract installation directory")

    subparsers = parser.add_subparsers(
        dest="subparser_name",
        title="Commands")
    parser_changes = subparsers.add_parser('compare',
                                           help="compare libtesseract Project with tessDir")
    parser_changes.set_defaults(func=tessCompare)
    
    parser_report = subparsers.add_parser('report',
                                          help="report libtesseract summary stats")
    parser_report.set_defaults(func=tessReport)

    parser_copy = subparsers.add_parser('copy',
                                        help="copy public libtesseract header files to includeDir")
    parser_copy.add_argument('includeDir', type=validateDir,
                             help="Directory to copy header files to.")
    parser_copy.set_defaults(func=tessCopy)

    parser_clean = subparsers.add_parser('clean',
        help="clean vs2010 folder of build folders and .user files")
    parser_clean.set_defaults(func=tessClean)

    #kludge because argparse has no ability to set default subparser
    if (len(sys.argv) == 2):
        sys.argv.append("compare")
    args = parser.parse_args()

    #handle commands
    if args.func == tessCopy:
        args.func(args.tessDir, args.includeDir)
    else:
        args.func(args.tessDir)

if __name__ == '__main__' :
    main()
