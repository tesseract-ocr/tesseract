:version: $RCSfile: versions.rst,v $ $Revision: 6c29e8896e5c $ $Date: 2011/03/14 21:50:44 $

.. default-role:: fs

===============
 Version Notes
===============

3.02 -- February ??, 2012
=========================

+ Created a completely new Visual Studio 2008 solution from scratch.

+ Added 64 new source files and removed the following deleted files
  (relative to v3.01):

     + ccutil\memblk.cpp
     + ccutil\memblk.h
     + ccutil\memryerr.h
     + wordrec\pieces.h
     + wordrec\tally.cpp
     + wordrec\tally.h

+ Created :guilabel:`LIB_Release`, :guilabel:`LIB_Debug`,
  :guilabel:`DLL_Release`, and :guilabel:`DLL_Debug` build
  configurations.

+ Created a single `libtesseract` library and removed generation of the
  twelve sub-libraries.

+ Used references to `leptonica_versionnumbers.vsprops` and
  `tesseract_versionnumbers.vsprops` Property Sheets, which define
  version number "user macros", in all Visual Studio Projects. These are
  also copied to `C:\\BuildFolder\\include`, so you can refer to them in
  your own projects. By using the new ``*_VERSION`` macros, you'll be
  isolated from worrying about version number changes in the library
  filenames.

  See :ref:`APITest's <APITest>` :ref:`LIB_Release <apitest-lib-release>` Linker
  :guilabel:`Additional Dependencies` settings for an example of what
  this looks like in practice.

  See |Leptonica|\ ’s explanation `About version numbers in library
  filenames
  <http://tpgit.github.com/UnOfficialLeptDocs/vs2008/downloading-binaries.html#about-version-numbers>`_
  for more details.

+ Added a :ref:`Version Resource <version-resource>` to all DLLs and
  applications.

+ Removed inclusion of the |Leptonica| libraries. They now have to be
  :ref:`downloaded separately <download-leptonica>`.

+ Changed to a :ref:`Build directory structure <directory-setup>` that
  is compatible with |Leptonica| and allows the building of
  |Tesseractocr|\ -based applications using only the `include` and `lib`
  directories.

+ The `libtesseract` libraries are now named as follows:

  static libraries:

  + libtesseract302-static.lib

  + libtesseract302-static-debug.lib

  DLLs:

  + libtesseract302.lib  (import library)

  + libtesseract302.dll

  + libtesseract302d.lib (import library)

  + libtesseract302d.dll

+ Used compiler and linker settings based on the |Leptonica| `VS2008
  Developer package
  <http://tpgit.github.com/UnOfficialLeptDocs/vs2008/index.html>`_.

+ Removed all preprocessor defines of ``__MSW32__`` which is no longer
  needed.

+ Removed `vs2008\include\stdint.h` which is no longer required to build
  |liblept|.

+ Removed `vs2008\include\inttypes.h` which isn't needed to build
  |liblept|.

+ Turned off the following compiler warnings::

     /wd4005: 'snprintf' : macro redefinition

     /wd4018 'expression' : signed/unsigned mismatch

     /wd4099 type name first seen using 'class' now seen using 'struct'

     /wd4244 conversion from 'double' to 'float', possible loss of data

     /wd4267 conversion from 'size_t' to 'type', possible loss of data

     /wd4305 truncation from 'type1' to 'type2'

     /wd4355 'this' : used in base member initializer list

     /wd4566 character represented by universal-character-name x cannot
             be represented in the current code page

     /wd4800 forcing value to bool 'true' or 'false' (performance warning)

     /wd4996 'function': was declared deprecated


+ Used the "C7 Compatible" Debug Information (/Z7) compiler switch,
  which puts the debugging information in the .obj files. That way we
  don't have to worry about also supplying `.pdb` files. See `/Z7,
  /Zi, /ZI (Debug Information Format)
  <http://msdn.microsoft.com/en-us/library/958x11bc(VS.90).aspx>`_ for
  more information.

+ Added Projects for the following new :ref:`training applications
  <training-applications>`:

  + ambiguous_words

  + classifier_tester

  + dawg2wordlist

  + shapeclustering

+ Moved `mathfix.h` from `vs2008\\include` to the `vs2008\\port`
  directory.
  
+ Removed Visual Studio 2010 support. See :doc:`vs2010-notes` for the
  rationale.

+ Created a python script called :ref:`tesshelper.py <tesshelper>`, that
  eases some maintenance tasks related to releasing future VS2008
  Solutions.

+ The list of "public" header files that are required to build
  "external" applications is now limited to the following 13 files::

     api\apitypes.h
     api\baseapi.h
     ccmain\thresholder.h
     ccstruct\publictypes.h
     ccutil\errcode.h
     ccutil\fileerr.h
     ccutil\host.h
     ccutil\memry.h
     ccutil\platform.h
     ccutil\serialis.h
     ccutil\strngs.h
     ccutil\tesscallback.h
     ccutil\unichar.h

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
