[![Build Status](https://travis-ci.org/tesseract-ocr/tesseract.svg?branch=3.05)](https://travis-ci.org/tesseract-ocr/tesseract?branch=3.05)
[![Build status](https://ci.appveyor.com/api/projects/status/github/tesseract-ocr/tesseract?branch=3.05&svg=true)](https://ci.appveyor.com/project/zdenop/tesseract?branch=3.05)

For the latest online version of the README.md see:
    
  https://github.com/tesseract-ocr/tesseract/blob/master/README.md

# About

This package contains an OCR engine - `libtesseract` and a command line program - `tesseract`.

The lead developer is Ray Smith. The maintainer is Zdenko Podobny. 
For a list of contributors see [AUTHORS](https://github.com/tesseract-ocr/tesseract/blob/master/AUTHORS)
and GitHub's log of [contributors](https://github.com/tesseract-ocr/tesseract/graphs/contributors).

Tesseract has unicode (UTF-8) support, and can recognize more than 100
languages "out of the box". It can be trained to recognize other languages. See [Tesseract Training](https://github.com/tesseract-ocr/tesseract/wiki/TrainingTesseract) for more information. 

Tesseract supports various output formats: plain-text, hocr(html), pdf.

This project does not include a GUI application. If you need one, please see the [3rdParty](https://github.com/tesseract-ocr/tesseract/wiki/User-Projects-%E2%80%93-3rdParty) wiki page.

You should note that in many cases, in order to get better OCR results, you'll need to [improve the quality](https://github.com/tesseract-ocr/tesseract/wiki/ImproveQuality) of the image you are giving Tesseract.

The latest stable version is 3.05.02, released on 19th June 2018.

# Brief history

Tesseract was originally developed at Hewlett-Packard Laboratories Bristol and
at Hewlett-Packard Co, Greeley Colorado between 1985 and 1994, with some
more changes made in 1996 to port to Windows, and some C++izing in 1998.

In 2005 Tesseract was open sourced by HP. Since 2006 it is developed by Google.

[Release Notes](https://github.com/tesseract-ocr/tesseract/wiki/ReleaseNotes)

# For developers

Developers can use `libtesseract` [C](https://github.com/tesseract-ocr/tesseract/blob/master/api/capi.h) or [C++](https://github.com/tesseract-ocr/tesseract/blob/master/api/baseapi.h) API to build their own application. If you need bindings to `libtesseract` for other programming languages, please see the [wrapper](https://github.com/tesseract-ocr/tesseract/wiki/AddOns#tesseract-wrappers) section on AddOns wiki page.

Documentation of Tesseract generated from source code by doxygen can be found on [tesseract-ocr.github.io](http://tesseract-ocr.github.io/).

# License

    The code in this repository is licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

**NOTE**: This software depends on other packages that may be licensed under different open source licenses.

# Installing Tesseract

You can either [Install Tesseract via pre-built binary package](https://github.com/tesseract-ocr/tesseract/wiki) or [build it from source](https://github.com/tesseract-ocr/tesseract/wiki/Compiling).

## Supported Compilers

* GCC 4.8 and above
* Clang 3.4 and above
* MSVC 2015, 2017

Other compilers might work, but are not officially supported.

# Running Tesseract

Basic command line usage:

    tesseract imagename outputbase [-l lang] [--psm pagesegmode] [configfiles...]

For more information about the various command line options use `tesseract --help` or `man tesseract`. 

# Support

Mailing-lists:
* [tesseract-ocr](https://groups.google.com/d/forum/tesseract-ocr) - For tesseract users. 
* [tesseract-dev](https://groups.google.com/d/forum/tesseract-dev) - For tesseract developers. 

Please read the [FAQ](https://github.com/tesseract-ocr/tesseract/wiki/FAQ) before asking any question in the mailing-list or reporting an issue.
