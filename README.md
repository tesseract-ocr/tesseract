[![Build Status](https://travis-ci.org/tesseract-ocr/tesseract.svg?branch=master)](https://travis-ci.org/tesseract-ocr/tesseract)
[![Build status](https://ci.appveyor.com/api/projects/status/miah0ikfsf0j3819?svg=true)](https://ci.appveyor.com/project/zdenop/tesseract/)

Note that this is possibly out-of-date version of the wiki ReadMe,
which is located at:

  https://github.com/tesseract-ocr/tesseract/blob/master/README.md

Introduction
============

This package contains the Tesseract Open Source OCR Engine.
Originally developed at Hewlett-Packard Laboratories Bristol and
at Hewlett-Packard Co, Greeley Colorado, all the code
in this distribution is now licensed under the Apache License:

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


Dependencies and Licenses
=========================

[Leptonica](http://www.leptonica.com) is required. Tesseract no longer 
compiles without Leptonica.

Libtiff is no longer required as a direct dependency.


Installing and Running Tesseract
--------------------------------

All Users Do NOT Ignore!

The tarballs are split into pieces.

tesseract-x.xx.tar.gz contains all the source code.

tesseract-x.xx.`<lang>`.tar.gz contains the language data files for `<lang>`.
You need at least one of these or Tesseract will not work.

Note that tesseract-x.xx.tar.gz unpacks to the tesseract-ocr directory.
tesseract-x.xx.`<lang>`.tar.gz unpacks to the tessdata directory which 
belongs inside your tesseract-ocr directory. It is therefore best to 
download them into your tesseract-x.xx directory, so you can use unpack 
here or equivalent. You can unpack as many of the language packs as you 
care to, as they all contain different files. Note that if you are using
make install you should unpack your language data to your source tree 
before you run make install. If you unpack them as root to the 
destination directory of make install, then the user ids and access
permissions might be messed up.

boxtiff-2.xx.`<lang>`.tar.gz contains data that was used in training for 
those that want to do their own training. Most users should NOT download
these files.

Instructions for using the training tools are documented separately at 
[Tesseract Training wiki](https://github.com/tesseract-ocr/tesseract/wiki/TrainingTesseract)


Windows
-------

Please use the installer (for 3.00 and above). Tesseract is a library with a 
command line interface. If you need a GUI, please check the [3rdParty wiki page](https://github.com/tesseract-ocr/tesseract/wiki/3rdParty#gui).

If you are building from the sources, the recommended build platform is 
VC++ Express 2010.

The executables are built with static linking, so they stand more chance
of working out of the box on more Windows systems.

The executable must reside in the same directory as the tessdata 
directory or you need to set up environment variable TESSDATA_PREFIX.
Installer will set it up for you.

The command line is:

    tesseract imagename outputbase [-l lang] [-psm pagesegmode] [configfiles...]

If you need interface to other applications, please check wrapper section
on [AddOns wiki page](https://github.com/tesseract-ocr/tesseract/wiki/AddOns#for-tesseract-ocr-30x).


Non-Windows (or Cygwin)
-----------------------

You have to tell Tesseract through a standard unix mechanism where to 
find its data directory. You must either:

    ./autogen.sh
    ./configure
    make
    sudo make install
    sudo ldconfig

to move the data files to the standard place, or:

    export TESSDATA_PREFIX="directory in which your tessdata resides/"

In either case the command line is:

    tesseract imagename outputbase [-l lang] [-psm pagesegmode] [configfiles...]

New there is a tesseract.spec for making rpms. (Thanks to Andrew Ziem for
the help.) It might work with your OS if you know how to do that.

If you are linking to the libraries, as Ocropus does, please link to
libtesseract_api.


If you get `leptonica not found` and you've installed it with e.g. homebrew, you
can run `CPPFLAGS="-I/usr/local/include" LDFLAGS="-L/usr/local/lib" ./configure`
instead of `./configure` above.


History
=======
The engine was developed at Hewlett-Packard Laboratories Bristol and
at Hewlett-Packard Co, Greeley Colorado between 1985 and 1994, with some
more changes made in 1996 to port to Windows, and some C++izing in 1998.
A lot of the code was written in C, and then some more was written in C++.
Since then all the code has been converted to at least compile with a C++
compiler. Currently it builds under Linux with gcc 4.4.3 and under Windows
with VC++2010. The C++ code makes heavy use of a list system using macros.
This predates stl, was portable before stl, and is more efficient than stl
lists, but has the big negative that if you do get a segmentation violation,
it is hard to debug.

The most recent change is that Tesseract can now recognize 39 languages,
including Arabic, Hindi, Vietnamese, plus 3 Fraktur variants, 
is fully UTF8 capable, and is fully trainable. See TrainingTesseract for
more information on training.

Tesseract was included in UNLV's Fourth Annual Test of OCR Accuracy. 
Results were available on https://github.com/tesseract-ocr/docs/blob/master/AT-1995.pdf.
With Tesseract 2.00, scripts were included to allow anyone to reproduce 
some of these tests. See TestingTesseract for more details. 


About the Engine
================
This code is a raw OCR engine. It has limited PAGE LAYOUT ANALYSIS, simple
OUTPUT FORMATTING (txt, hocr/html), and NO UI. 
Having said that, in 1995, this engine was in the top 3 in terms of character
accuracy, and it compiles and runs on both Linux and Windows.
As of 3.01, Tesseract is fully unicode (UTF-8) enabled, and can recognize 39
languages "out of the box." Code and documentation is provided for the brave
to train in other languages. 
See [Tesseract Training wiki](https://github.com/tesseract-ocr/tesseract/wiki/TrainingTesseract) 
for more information on training. Additional [code and extracted documentation](http://tesseract-ocr.github.io/) was generated by Doxygen.
