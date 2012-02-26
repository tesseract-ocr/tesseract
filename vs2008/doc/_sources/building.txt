:version: $RCSfile: index.rst,v $ $Revision: 76e0bf38aaba $ $Date: 2011/03/22 00:48:41 $

.. default-role:: fs

=========================
 Building |Tesseractocr|
=========================

The Visual Studio 2008 Solution for |Tesseractocr| builds:

+ `libtesseract`

+ `tesseract.exe`

+ 9 training applications (for v3.02)

Unlike earlier Solutions only a single `libtesseract` library is
generated --- the twelve projects matching the twelve source subfolders
have been abandoned. They were deemed too complicated since they were
never (rarely?) used by themselves, but only along with the entire
library.

In addition, `libtesseract` and `tesseract.exe` can be built using four
configurations: :guilabel:`LIB_Release`, :guilabel:`LIB_Debug`,
:guilabel:`DLL_Release`, and :guilabel:`DLL_Debug`.

Two Visual Studio Property Sheets, `leptonica_versionnumbers.vsprops`
and `tesseract_versionnumbers.vsprops`, are employed to isolate the
Solution from changes in dependency version numbers (and isolate
dependent Solutions). See :ref:`APITest's <APITest>` :ref:`LIB_Release
<apitest-lib-release>` Linker :guilabel:`Additional Dependencies`
settings for an example of what this looks like in practice. See
|Leptonica|\ ’s explanation `About version numbers in library filenames
<http://tpgit.github.com/UnOfficialLeptDocs/vs2008/downloading-binaries.html#about-version-numbers>`_
for the rationale behind using Property Sheets.


Building `libtesseract` and `tesseract.exe`
===========================================

1. Open `C:\\BuildFolder\\tesseract-3.0x\\vs2008\\tesseract.sln` in Visual
   Studio 2008.

   You'll see the following projects in the :guilabel:`Solution
   Explorer` (for v3.02)::

      ambiguous_words
      classifier_tester
      cntraining
      combine_tessdata
      dawg2wordlist
      libtesseract302
      mftraining
      shapeclustering
      tesseract
      unicharset_extractor
      wordlist2dawg

2. Select the build configuration you'd like to use from the
   :guilabel:`Solution Configurations` dropdown. It lists the following
   configurations::

      DLL_Debug
      DLL_Release
      LIB_Debug
      LIB_Release

   The `DLL_` configurations build the DLL version of `libtesseract-3.0x`
   (and link with the DLL version of Leptonica 1.68). The `LIB_`
   configurations build the static library version of `libtesseract-3.0x`
   (and link with the static version of Leptonica 1.68 and the required
   image libraries).

3. Build `libtesseract` by right-clicking the
   :guilabel:`libtesseract30x` project and choosing
   :menuselection:`B&uild` from the pop-up menu.

   The resultant library will be written to the
   `C:\\BuildFolder\\tesseract-3.0x\\vs2008\\<ConfigurationName>` directory
   where `<ConfigurationName>` is the same as the build configuration you
   selected earlier. It is also copied to the `C:\\BuildFolder\\lib` folder
   to make it easy to link your own applications to `libtesseract`.

   The library is named as follows (for v3.02):

   .. parsed-literal::

      static libraries:

         `libtesseract302-static.lib`
         `libtesseract302-static-debug.lib`

      DLLs:

         `libtesseract302.lib`  (import library)
         `libtesseract302.dll`
         `libtesseract302d.lib` (import library)
         `libtesseract302d.dll`

4. Build the main tesseract OCR application by right-clicking the
   :guilabel:`tesseract` project and choosing :menuselection:`B&uild`. 

   The resultant executable will be written to the
   `C:\\BuildFolder\\tesseract-3.0x\\vs2008\\<ConfigurationName>` directory
   where `<ConfigurationName>` is the same as the build configuration you
   selected earlier. It is named as follows:

   .. parsed-literal::

     LIB_Release: `tesseract.exe`
     LIB_Debug:   `tesseractd.exe`
     DLL_Release: `tesseract-dll.exe`
     DLL_Debug:   `tesseract-dlld.exe`
   

Testing `tesseract.exe`
=======================

It's usually better to make a separate directory to test
`tesseract.exe`. To run tesseract, you either need to make sure your
test directory contains the `tessdata` tesseract language data folder or
you set the ``TESSDATA_PREFIX`` environment variable to point to it. See
http://code.google.com/p/tesseract-ocr/wiki/ReadMe for important
details.

For example, you can use the following directory structure::

   C:\BuildFolder\
     include\
     lib\
     tesseract-3.02\
     testing\
        tessdata\

Copy your tesseract executable to `C:\\BuildFolder\\testing`. If you
built a DLL version then be sure to also copy the required DLLs to the
same directory (or add `C:\\BuildFolder\\lib` to your ``PATH`` --
However, this isn't really recommended).

For example, if you are trying to run `tesseractd.exe` then you'll need
to also copy the following to `C:\\BuildFolder\\testing`::

   liblept168d.dll
   libtesseract302d.dll

Copy a few test images to `C:\\BuildFolder\\testing` just to make it easy
to run test commands.

Test tesseract by doing something like the following::

   tesseractd.exe eurotext.tif eurotext

This will create a file called `eurotext.txt` that will contain the
result of OCRing `eurotext.tif`.


Building the training applications
==================================

The training related applications are built using the following
projects::

   ambiguous_words
   classifier_tester
   cntraining
   combine_tessdata
   dawg2wordlist
   mftraining
   shapeclustering
   unicharset_extractor
   wordlist2dawg

.. note::

   Currently these applications can **ONLY** be built with the LIB_Debug
   and LIB_Release configurations. If you try to use a DLL configuration
   you'll get "undefined external symbol" errors.

To build one of the above training applications, simply right-click one
of the projects in the Solution Explorer, and choose
:menuselection:`B&uild` from the pop-up menu.

Alternatively, you can build :bi:`everything` in the Solution by
choosing :menuselection:`&Build --> &Build Solution` (:kbd:`Ctrl+Shift+B`)
from the menu bar.

See http://code.google.com/p/tesseract-ocr/wiki/TrainingTesseract3 for
more information on using these applications.


.. _building-with-vc2008-express:

Building |Tesseractocr| with Visual C++ 2008 Express Edition
============================================================

The Solution file that comes with |Tesseractocr| was created with Visual
Studio 2008, and is compatible for the most part with the free `Visual
C++ 2008 Express Edition
<http://www.microsoft.com/visualstudio/en-us/products/2008-editions/express>`_. You
might, however, sometimes see the following error message::

   Fatal error RC1015: cannot open include file 'afxres.h'

.. _version-resource:

The Solution uses resource files to set application and DLL properties
that are visible on Windows 7 when you right-click them in Windows
Explorer, choose :menuselection:`Properties`, and look at the
:guilabel:`Details` tab (the :guilabel:`Version` tab on Windows XP).

      .. image:: images/dll_properties_details_tab.png
         :align: center
         :alt: Windows 7 Properties' Details Tab

Unfortunately, the Express Edition doesn't include the Resource
Editor. So in all resource files::

   #include "afxres.h"

has to be changed to::

   #include "windows.h"

If someone has used the VS2008 Resource Editor to change a `.rc` file
associated with an application or DLL and forgotten to make these
changes before checking the file in, you'll see the above "Fatal error"
message. Simply manually make the change to fix the error.


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
