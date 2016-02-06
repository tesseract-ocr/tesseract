[![Build Status](https://travis-ci.org/tesseract-ocr/tesseract.svg?branch=master)](https://travis-ci.org/tesseract-ocr/tesseract)
[![Build status](https://ci.appveyor.com/api/projects/status/miah0ikfsf0j3819?svg=true)](https://ci.appveyor.com/project/zdenop/tesseract/)


#About

This package contains an OCR engine - `libtesseract` and a command line program - `tesseract`.

The lead developer is Ray Smith. The maintainer is Zdenko Podobny. 
For a list of contributors see [AUTHORS](https://github.com/tesseract-ocr/tesseract/blob/master/AUTHORS) and github's log of [contributors](https://github.com/tesseract-ocr/tesseract/graphs/contributors).

Tesseract has unicode (UTF-8) support, and can recognize more than 100
languages "out of the box". It can be trained to recognize other languages. See [Tesseract Training](https://github.com/tesseract-ocr/tesseract/wiki/TrainingTesseract) for more information. 

Tesseract supports various output formats: plain-text, hocr(html), pdf.

The latest stable version is 3.04, released in July 2015.

#Brief history

Tesseract was originally developed at Hewlett-Packard Laboratories Bristol and
at Hewlett-Packard Co, Greeley Colorado between 1985 and 1994, with some
more changes made in 1996 to port to Windows, and some C++izing in 1998.

In 2005 Tesseract was open sourced by HP. Since 2006 it is developed by Google.

[Release Notes](https://github.com/tesseract-ocr/tesseract/wiki/ReleaseNotes)

#License

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

#Installing Tesseract

You can either [Install Tesseract via pre-built binary package](https://github.com/tesseract-ocr/tesseract/wiki) or [build it from source](https://github.com/tesseract-ocr/tesseract/wiki/Compiling).

#Running Tesseract

Basic command line usage:

    tesseract imagename outputbase [-l lang] [-psm pagesegmode] [configfiles...]

To see the full usage use `tesseract --help`

#Support

Mailing-lists:
* [tesseract-ocr](https://groups.google.com/d/forum/tesseract-ocr) - For tesseract users. 
* [tesseract-dev](https://groups.google.com/d/forum/tesseract-dev) - For tesseract developers. 

Please read the [FAQ](https://github.com/tesseract-ocr/tesseract/wiki/FAQ) before asking any question in the mailing-list or reporting an issue.
