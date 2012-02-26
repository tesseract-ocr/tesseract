:version: $RCSfile: index.rst,v $ $Revision: 76e0bf38aaba $ $Date: 2011/03/22 00:48:41 $

.. default-role:: fs

=================================
 Programming with `libtesseract`
=================================

To use `libtesseract` in your own application you need to include
|Leptonica|\ ’s `allheaders.h`, and |Tesseractocr|\ ’s `baseapi.h` and
`strngs.h`. 

|Tesseractocr| uses `liblept` mainly for image I/O, but you can also use
any of |Leptonica|\ ’s *many* image processing functions on ``PIX``,
while at the same time calling ``TessBaseAPI`` methods. See the
`Leptonica documentation <http://tpgit.github.com/UnOfficialLeptDocs/>`_
for more details.

There doesn't seem to be any documentation on `api\\baseapi.h`, but it
has extensive comments. You can also look at the :ref:`APITest` and
:ref:`APIExamples` projects.

See the :ref:`APITest` project for an example of which compiler and
linker settings you need for various build configurations. The easiest
way to begin a new application is to just make a copy of the `APITest`
directory. See :ref:`this step <copying_a_project>` for detailed
instructions (skip the last step about adding :guilabel:`Project
Dependencies`).

If you want to manually set the required settings, then here's the list
of things to do:

1. Add the following :guilabel:`Preprocessor Definitions` when compiling
   any files that include `baseapi.h` and you are linking with the
   static library versions of `libtesseract`::

      USE_STD_NAMESPACE

   If you are linking with the DLL versions of `libtesseract` instead
   add::

      USE_STD_NAMESPACE;TESSDLL_IMPORTS;CCUTIL_IMPORTS;LIBLEPT_IMPORTS

#. Be sure to add the following to :guilabel:`Additional Include
   Directories`::

      C:\BuildFolder\include
      C:\BuildFolder\include\leptonica

      C:\BuildFolder\include\tesseract or

      <tesseract-3.0x dir> (all its sub-directories that contain header files)

#. Add `C:\\BuildFolder\\lib` to your :guilabel:`Additional Library
   Directories`.

#. In the `C:\\BuildFolder\\include` directory are two Visual Studio
   Property Sheet files::

      tesseract_versionnumbers.vsprops
      leptonica_versionnumbers.vsprops

   Using `tesseract_versionnumbers.vsprops` (which automatically inherits
   `leptonica_versionnumbers.vsprops`) can make it easier to specify the
   libraries you need to import. For example, when creating a staticly
   linked debug executable you can say::

      zlib$(ZLIB_VERSION)-static-mtdll-debug.lib
      libpng$(LIBPNG_VERSION)-static-mtdll-debug.lib
      libjpeg$(LIBJPEG_VERSION)-static-mtdll-debug.lib
      giflib$(GIFLIB_VERSION)-static-mtdll-debug.lib
      libtiff$(LIBTIFF_VERSION)-static-mtdll-debug.lib
      liblept$(LIBLEPT_VERSION)-static-mtdll-debug.lib
      libtesseract$(LIBTESS_VERSION)-static-debug.lib

   to make your application less dependent on library version numbers.

   To add the Property Sheet to a Project, open its :guilabel:`Properties
   Pages` Dialog, and set the :guilabel:`Configuration Properties |
   General | Inherited Project Property Sheets` item to::

      ..\..\..\include\tesseract_versionnumbers.vsprops

   Choosing :menuselection:`&View --> Oth&er Windows --> Property
   &Manager` from the menubar will let you see the Properties attached
   to each Project's configurations.

.. note::

   The DLL versions of |libtess| currently only export the
   ``TessBaseAPI`` C++ class from `baseapi.h`, there is no C function
   interface yet.

.. note::

   The DLL versions of `libtesseract` currently only export the
   ``TessBaseAPI`` and ``STRING`` classes. In theory, all you need is
   are those classes. However, if you find yourself having to manipulate
   other "internal" tesseract objects then you currently have to link
   with the **static library** versions of `libtesseract`.

.. warning::

   The Release versions of |liblept|, by design, *never* print out any
   possibly helpful messages to the console. Therefore, it is highly
   recommended that you do your initial development using the Debug
   versions of |liblept|. See `Compile-time control over stderr output
   <http://tpgit.github.com/UnOfficialLeptDocs/leptonica/README.html#compile-time-control-over-stderr-output>`_
   for details.

<<<Need to add the URL of the zip file that contains include & lib
directory contents for those people who don't want to build libtesseract
themselves>>>


Debugging Tips
==============

Before debugging programs written with `libtesseract`, you should first
download the latest Leptonica sources (currently
`leptonica-1.68.tar.gz`) and VS2008 source package (`vs2008-1.68.zip`)
from:

+ http://code.google.com/p/leptonica/downloads/detail?name=leptonica-1.68.tar.gz
+ http://code.google.com/p/leptonica/downloads/detail?name=vs2008-1.68.zip

Unpack them to `C:\\BuildFolder` to get the following directory structure::

   C:\BuildFolder\
     include\
     lib\
     leptonica-1.68\
        vs2008\
     tesseract-3.02\
        vs2008\
     testing\
        tessdata\

(see `Building the liblept library
<http://tpgit.github.com/UnOfficialLeptDocs/vs2008/building-liblept.html>`_
for more information)

|Tesseractocr| uses |Leptonica| "under the hood" for all (most? some?)
of its image processing operations. Having the source available (and
compiling it in debug mode) will make it easier to see what's really
going on.

You might want to add
`C:\\BuildFolder\\leptonica-1.68\\vs2008\\leptonica.vcproj` and
`C:\\BuildFolder\\tesseract-3.02\\vs2008\\libtesseract\\libtesseract.vcproj`
to your solution by right-clicking it and choosing :menuselection:`A&dd -->
&Existing Project...`. This seems to make VS2008's Intellisense `work
better
<http://tpgit.github.com/UnOfficialLeptDocs/vs2008/building-other-programs.html#intellisense-and-liblept>`_
when finding "external" source files.

Definitely create a ``TESSDATA_PREFIX``x environment variable so that it
contains the absolute path of the directory that contains the
``tessdata`` directory. Otherwise you'll have to put a ``tessdata``
directory in every temporary build folder which quickly becomes painful
(especially since tessdata has gotten very big --- 600MB!).


.. _APITest:

APITest Sample
==============

The :guilabel:`APITest` Solution contains the minimal settings needed to
link with `libtesseract`. It demonstrates the typical situation, where
the "external" application's source files reside *outside* of the
`tesseract-3.0x` directory tree.

To build the `vs2008\\APITest` Solution, first copy it to your
`C:\\BuildFolder` directory. This should now look like::

   C:\BuildFolder\

     include\
        leptonica\
        tesseract\

        leptonica_versionnumbers.vsprops
        tesseract_versionnumbers.vsprops

     lib\
        giflib416-static-mtdll-debug.lib
        giflib416-static-mtdll.lib
        libjpeg8c-static-mtdll-debug.lib
        libjpeg8c-static-mtdll.lib
        liblept168-static-mtdll-debug.lib
        liblept168-static-mtdll.lib
        liblept168.dll
        liblept168.lib
        liblept168d.dll
        liblept168d.lib
        libpng143-static-mtdll-debug.lib
        libpng143-static-mtdll.lib
        libtesseract302.dll
        libtesseract302.lib
        libtesseract302d.dll
        libtesseract302d.lib
        libtesseract302-static.lib
        libtesseract302-static-debug.lib
        libtiff394-static-mtdll-debug.lib
        libtiff394-static-mtdll.lib
        zlib125-static-mtdll-debug.lib
        zlib125-static-mtdll.lib

     tesseract-3.02\

     APITest\
        baseapitester\
           baseapitester.cpp
           baseapitester.rc
           baseapitester.vcproj
           resource.h
           stdafx.cpp
           stdafx.h
           targetver.h
        APITest.sln

The :guilabel:`APITest` contains just the :guilabel:`baseapitester`
project. This was created using the VS2008 :guilabel:`Win32 Console
Application` Project Wizard and then just copying most of
`tesseractmain.cpp` and making minor edits. Its settings correctly refer
to the "public" `include` and `lib` directories using relative paths.

It assumes that the `C:\\BuildFolder\\include` directory has been
properly setup. See :ref:`this <copying-headers>` for more details. 

The `C:\\BuildFolder\\lib` directory will automatically get
`libtesseract` copied to it whenever it is built.

The `include\\tesseract_versionnumbers.vsprops` Property Sheet is used
to avoid explicit library version number dependencies. Precompiled
headers are used. :guilabel:`LIB_Release`, :guilabel:`LIB_Debug`,
:guilabel:`DLL_Release`, and :guilabel:`DLL_Debug` build configurations
are supported.

The following are the compiler command lines and linker options
used. See `Compiling a C/C++ Program | Compiler Options
<http://msdn.microsoft.com/en-us/library/9s7c9wdw(v=vs.90).aspx>`_ for a
detailed explanation of these options.

.. _apitest-lib-release:

:guilabel:`LIB_Release` C/C++ :guilabel:`Command Line`::

   /O2
   /I "." /I "..\..\include" /I "..\..\include\leptonica"
   /I "..\..\include\tesseract"
   /D "WIN32" /D "_WINDOWS" /D "NDEBUG"
   /D "USE_STD_NAMESPACE" /D "_MBCS"
   /FD /EHsc /MD /Yc"stdafx.h"
   /Fp"LIB_Release\baseapitester.pch" /Fo"LIB_Release\\"
   /Fd"LIB_Release\vc90.pdb"
   /W3 /nologo /c 
   /wd4244 /wd4305 /wd4018 /wd4267 /wd4996 /wd4800 /wd4005 /wd4355 /wd4099 /wd4566
   /errorReport:prompt

:guilabel:`LIB_Release` Linker :guilabel:`Additional Dependencies`::

   ws2_32.lib
   user32.lib
   zlib$(ZLIB_VERSION)-static-mtdll.lib
   libpng$(LIBPNG_VERSION)-static-mtdll.lib
   libjpeg$(LIBJPEG_VERSION)-static-mtdll.lib
   giflib$(GIFLIB_VERSION)-static-mtdll.lib
   libtiff$(LIBTIFF_VERSION)-static-mtdll.lib
   liblept$(LIBLEPT_VERSION)-static-mtdll.lib
   libtesseract$(LIBTESS_VERSION)-static.lib

:guilabel:`LIB_Debug` C/C++ :guilabel:`Command Line`::

   /Od
   /I "." /I "..\..\include" /I "..\..\include\leptonica"
   /I "..\..\include\tesseract"
   /D "WIN32" /D "_WINDOWS" /D "_DEBUG"
   /D "USE_STD_NAMESPACE" /D "_MBCS"
   /FD /EHsc /RTC1 /MDd /Yc"stdafx.h"
   /Fp"LIB_Debug\baseapitesterd.pch" /Fo"LIB_Debug\\"
   /Fd"LIB_Debug\vc90.pdb"
   /W3 /nologo /c /Z7
   /wd4244 /wd4305 /wd4018 /wd4267 /wd4996 /wd4800 /wd4005 /wd4355 /wd4099 /wd4566
   /errorReport:prompt

:guilabel:`LIB_Debug` Linker :guilabel:`Additional Dependencies`::

   ws2_32.lib
   user32.lib
   zlib$(ZLIB_VERSION)-static-mtdll-debug.lib
   libpng$(LIBPNG_VERSION)-static-mtdll-debug.lib
   libjpeg$(LIBJPEG_VERSION)-static-mtdll-debug.lib
   giflib$(GIFLIB_VERSION)-static-mtdll-debug.lib
   libtiff$(LIBTIFF_VERSION)-static-mtdll-debug.lib
   liblept$(LIBLEPT_VERSION)-static-mtdll-debug.lib
   libtesseract$(LIBTESS_VERSION)-static-debug.lib

:guilabel:`DLL_Release` C/C++ :guilabel:`Command Line`::

   /O2
   /I "." /I "..\..\include" /I "..\..\include\leptonica"
   /I "..\..\include\tesseract"
   /D "WIN32" /D "_WINDOWS" /D "NDEBUG"
   /D "USE_STD_NAMESPACE" /D "_MBCS"
   /D "TESSDLL_IMPORTS" /D "CCUTIL_IMPORTS" /D "LIBLEPT_IMPORTS"
   /FD /EHsc /MD /Yc"stdafx.h"
   /Fp"DLL_Release\baseapitester-dll.pch" /Fo"DLL_Release\\"
   /Fd"DLL_Release\vc90.pdb"
   /W3 /nologo /c
   /wd4244 /wd4305 /wd4018 /wd4267 /wd4996 /wd4800 /wd4005 /wd4355 /wd4099 /wd4566
   /errorReport:prompt

:guilabel:`DLL_Release` Linker :guilabel:`Additional Dependencies`::

   ws2_32.lib
   user32.lib
   liblept$(LIBLEPT_VERSION).lib
   libtesseract$(LIBTESS_VERSION).lib

:guilabel:`DLL_Debug` C/C++ :guilabel:`Command Line`::

   /Od
   /I "." /I "..\..\include" /I "..\..\include\leptonica"
   /I "..\..\include\tesseract"
   /D "WIN32" /D "_WINDOWS" /D "_DEBUG"
   /D "USE_STD_NAMESPACE" /D "_MBCS" 
   /D "TESSDLL_IMPORTS" /D "CCUTIL_IMPORTS" /D "LIBLEPT_IMPORTS"
   /FD /EHsc /RTC1 /MDd /Yc"stdafx.h"
   /Fp"DLL_Debug\baseapitester-dlld.pch" /Fo"DLL_Debug\\"
   /Fd"DLL_Debug\vc90.pdb"
   /W3 /nologo /c /Z7
   /wd4244 /wd4305 /wd4018 /wd4267 /wd4996 /wd4800 /wd4005 /wd4355 /wd4099 /wd4566
   /errorReport:prompt

:guilabel:`DLL_Debug` Linker :guilabel:`Additional Dependencies`::

   ws2_32.lib
   user32.lib
   liblept$(LIBLEPT_VERSION)d.lib
   libtesseract$(LIBTESS_VERSION)d.lib


.. _APIExamples:

APIExamples
===========

<<<NEEDS WORK>>>

Currently two Projects are in this solution:

+ preprocessing -- Demonstrates how to use |Leptonica|\ ’s image
  processing functions to clean up images *before* calling
  ``TessBaseAPI::SetImage()``.

+ getinfo -- Demonstrates calling various ``TessBaseAPI`` methods to get
  back information on the OCR process.



|Tesseractocr| preprocessor definitions
=======================================

``HAVE_CONFIG_H``
  Only defined when building under Linux. This causes the inclusion of
  `config_auto.h`, which is only auto-generated during the `./configure`
  process and thus *not* visible on Windows.

  This is what sets the ``VERSION`` macro (and lots of other
  configuration related macros).


``TESSDLL_EXPORTS``
  Only used when *building* DLL versions of |libtess|. 

``TESSDLL_IMPORTS``
  Should be defined when building apps that link to a DLL version of
  |libtess|. Used as follows in `baseapi.h`::
  
     #ifdef TESSDLL_EXPORTS
     #define TESSDLL_API __declspec(dllexport)
     #elif defined(TESSDLL_IMPORTS)
     #define TESSDLL_API __declspec(dllimport)
     #else
     #define TESSDLL_API
     #endif

  If you don't define this then you'll get "undefined external symbol"
  errors.

``TESSDLL_API``
  Used to mark classes for export (visibility) in DLL versions of
  |libtess|. Currently *only* used with the ``TestBaseAPI`` class.


``CCUTIL_EXPORTS``
  Only used when *building* DLL versions of |libtess|.

``CCUTIL_IMPORTS``
  Should be defined when building apps that link to a DLL version of
  |libtess|. Used as follows in `strngs.h`::

     #ifdef CCUTIL_EXPORTS
     #define CCUTIL_API __declspec(dllexport)
     #elif defined(CCUTIL_IMPORTS)
     #define CCUTIL_API __declspec(dllimport)
     #else
     #define CCUTIL_API
     #endif

  If you don't define this then you'll get "undefined external symbol STRING"
  errors.


``LIBLEPT_IMPORTS``
  Should be defined when building apps that link to a DLL version of
  |Leptonica|. Used as follows in environ.h::

     #if defined(LIBLEPT_EXPORTS) || defined(LEPTONLIB_EXPORTS)
     #define LEPT_DLL __declspec(dllexport)
     #elif defined(LIBLEPT_IMPORTS) || defined(LEPTONLIB_IMPORTS)
     #define LEPT_DLL __declspec(dllimport)
     #else
     #define LEPT_DLL
     #endif

  If you don't define this then you'll get "undefined external symbol"
  errors.

``USE_STD_NAMESPACE``
  Causes the following to be done::

     #ifdef USE_STD_NAMESPACE
     using std::string;
     using std::vector;
     #endif


``_WIN32``
  Used to indicate that the build target is Windows 32-bit or
  64-bit (``WIN32`` and ``WINDOWS`` are also added by the New Project
  Wizards).

  See `C/C+ Preprocessor Reference | The Preprocessor | Macros |
  Predefined Macros
  <http://msdn.microsoft.com/en-us/library/b0084kay(v=vs.90).aspx>`_ for
  the complete list for Visual Studio 2008.

``_MSC_VER``
  Used to check specifically for building with the VC++ compiler (as
  opposed to the MinGW gcc compiler).

``_USRDLL``
  Only defined when building the DLL versions of `libtesseract`.

``_MBCS``
  Automatically defined when :guilabel:`Configuration Properties |
  General | Character Set` is set to :guilabel:`Use Multi-Byte
  Character Set`.


``DLLSYM``
  `Obsolete
  <http://groups.google.com/group/tesseract-dev/msg/5e0f7f7fab27b463>`_
  and can be ignored. 

..         
   Local Variables:
   coding: utf-8
   mode: rst
   indent-tabs-mode: nil
   sentence-end-double-space: t
   fill-column: 72
   mode: auto-fill
   standard-indent: 3
   tab-stop-list: (3 6 9 12 15 18 21 24 27 30 33 36 39 42 45 48 51 54 57 60)
   End:
