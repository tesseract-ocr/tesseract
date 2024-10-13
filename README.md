# Tesseract OCR 

<div align="center">

[![Build status](https://ci.appveyor.com/api/projects/status/miah0ikfsf0j3819/branch/master?svg=true)](https://ci.appveyor.com/project/zdenop/tesseract/)
[![Build status](https://github.com/tesseract-ocr/tesseract/actions/workflows/sw.yml/badge.svg)](https://github.com/tesseract-ocr/tesseract/actions/workflows/sw.yml)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/tesseract-ocr/badge.svg)](https://scan.coverity.com/projects/tesseract-ocr)
[![CodeQL](https://github.com/tesseract-ocr/tesseract/workflows/CodeQL/badge.svg)](https://github.com/tesseract-ocr/tesseract/security/code-scanning)
[![OSS-Fuzz](https://img.shields.io/badge/oss--fuzz-fuzzing-brightgreen)](https://bugs.chromium.org/p/oss-fuzz/issues/list?sort=-opened&can=2&q=proj:tesseract-ocr)

[![GitHub license](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](https://raw.githubusercontent.com/tesseract-ocr/tesseract/main/LICENSE)
[![Downloads](https://img.shields.io/badge/download-all%20releases-brightgreen.svg)](https://github.com/tesseract-ocr/tesseract/releases/)

</div>

## Table of Contents 📚 


* [Tesseract OCR](#tesseract-ocr)
  * [📖About](#about)
  * [⏳Brief history](#brief-history)
  * [💻Installing Tesseract](#installing-tesseract)
  * [🚀 Running Tesseract](#running-tesseract)
  * [👩‍💻For developers](#for-developers)
  * [🤝Support](#support)
  * [📜License](#license)
  * [🔗Dependencies](#dependencies)
  * [📅Latest Version of README](#latest-version-of-readme)

## 📖About
    
   <p>
        This package contains an Optical Character Recognition (OCR)👁️‍🗨️engine—<strong>libtesseract</strong>—and a command-line program called <strong>tesseract</strong>.
    </p>


   
   <h2>Tesseract 4 Features</h2>
   <div align="center">
    <img src="https://www.stormthecastle.com/indeximages/the-completed-tesseract.jpg" height="200"> </div>
    
   <p>
        Tesseract 4 adds a new neural network 🧠 (LSTM) based OCR engine that focuses on line recognition, while still supporting the legacy Tesseract OCR engine from Tesseract 3, which recognizes character patterns. Compatibility with Tesseract 3 can be enabled by using the Legacy OCR Engine mode (<code>--oem 0</code>). This mode requires trained data files that support the legacy engine, available from the <a href="https://tesseract-ocr.github.io/tessdoc/Data-Files.html">tessdata repository</a>.
    </p>

   <h2>Development Team 👥</h2>
    <p>
        The current lead developer is <strong>Stefan Weil</strong>. <strong>Ray Smith</strong> served as the lead developer until 2018. The project maintainer is <strong>Zdenko Podobny</strong>. For a list of contributors, please refer to the <code>AUTHORS</code> file and the contributors’ log on GitHub.
    </p>

   <h2>Language and Format Support 📄 </h2>
    <p>
        Tesseract supports Unicode (UTF-8) and can recognize over 100 languages "out of the box." It supports various image formats including PNG, JPEG, and TIFF.
    </p>
    <p>
        Tesseract also supports various output formats:
    </p>
    <ul>
        <li>Plain text</li>
        <li>hOCR (HTML)</li>
        <li>PDF</li>
        <li>Invisible-text-only PDF</li>
        <li>TSV</li>
        <li>ALTO</li>
        <li>PAGE</li>
    </ul>

   <h2>Image Quality: 🖼️</h2>
    <p>
        To achieve better OCR results, it is important to enhance the quality of the images provided to Tesseract.
    </p>

   <h2>GUI Applications 🖥️ </h2>
    <p>
        This project does not include a graphical user interface (GUI). If you need one, please refer to the third-party documentation.
    </p>

   <h2>Training Tesseract📚</h2>
    <p>
        Tesseract can be trained to recognize additional languages. For more information, please see the <a href="https://tesseract-ocr.github.io/tessdoc/Data-Files.html">Tesseract Training documentation</a> and the <a href="https://github.com/tesseract-ocr/tessdata">tessdata repository</a>.
    </p>

   <h2>Additional Information ℹ️ </h2>
    <p>
        For more about Optical Character Recognition, visit <a href="https://en.wikipedia.org/wiki/Optical_character_recognition">Wikipedia on OCR</a>.
    </p>
</body>
</html>




## ⏳Brief history

<body>
   

   <p>
        Tesseract was originally developed at Hewlett-Packard Laboratories in Bristol, UK, and at Hewlett-Packard Co. in Greeley, Colorado, USA, between 1985 and 1994. Subsequent changes were made in 1996 to port it to Windows, with further enhancements in C++ occurring in 1998. In 2005, Tesseract was open-sourced by HP, and it was actively developed by Google from 2006 until November 2018.
    </p>

   <p>
        The current stable version, <strong>Major Version 5</strong>, began with the release of 5.0.0 on November 30, 2021. Newer minor and bug fix versions are continuously made available on <a href="https://github.com/tesseract-ocr/tesseract">GitHub</a>.
    </p>

   <p>
        The latest source code can be found in the main branch on <a href="https://github.com/tesseract-ocr/tesseract">GitHub</a>. You can also view open issues in the <a href="https://github.com/tesseract-ocr/tesseract/issues">issue tracker</a> and consult the planning documentation.
    </p>

   <p>
        For more details on the releases, see the <a href="https://github.com/tesseract-ocr/tesseract/releases">Release Notes</a> and the <a href="https://github.com/tesseract-ocr/tesseract/blob/master/CHANGELOG.md">Change Log</a>.
    </p>
</body>

## 💻Installing Tesseract

You can either [Install Tesseract via pre-built binary package](https://tesseract-ocr.github.io/tessdoc/Installation.html)
or [build it from source](https://tesseract-ocr.github.io/tessdoc/Compiling.html).

Before building Tesseract from source, please check that your system has a compiler which is one of the [supported compilers](https://tesseract-ocr.github.io/tessdoc/supported-compilers.html).

## 🚀Running Tesseract

Basic **[command line usage](https://tesseract-ocr.github.io/tessdoc/Command-Line-Usage.html)**:

    tesseract imagename outputbase [-l lang] [--oem ocrenginemode] [--psm pagesegmode] [configfiles...]

For more information about the various command line options use `tesseract --help` or `man tesseract`.

Examples can be found in the [documentation](https://tesseract-ocr.github.io/tessdoc/Command-Line-Usage.html#simplest-invocation-to-ocr-an-image).

## 👩‍💻For developers

Developers can use `libtesseract` [C](https://github.com/tesseract-ocr/tesseract/blob/main/include/tesseract/capi.h) or
[C++](https://github.com/tesseract-ocr/tesseract/blob/main/include/tesseract/baseapi.h) API to build their own application. If you need bindings to `libtesseract` for other programming languages, please see the
[wrapper](https://tesseract-ocr.github.io/tessdoc/AddOns.html#tesseract-wrappers) section in the AddOns documentation.

Documentation of Tesseract generated from source code by doxygen can be found on [tesseract-ocr.github.io](https://tesseract-ocr.github.io/).

## 🤝Support

Before you submit an issue, please review **[the guidelines for this repository](https://github.com/tesseract-ocr/tesseract/blob/main/CONTRIBUTING.md)**.

For support, first read the [documentation](https://tesseract-ocr.github.io/tessdoc/),
particularly the [FAQ](https://tesseract-ocr.github.io/tessdoc/FAQ.html) to see if your problem is addressed there.
If not, search the [Tesseract user forum](https://groups.google.com/g/tesseract-ocr), the [Tesseract developer forum](https://groups.google.com/g/tesseract-dev) and [past issues](https://github.com/tesseract-ocr/tesseract/issues), and if you still can't find what you need, ask for support in the mailing-lists.

<h4>Mailing-lists:</h4>

* [tesseract-ocr](https://groups.google.com/g/tesseract-ocr) - For tesseract users.
* [tesseract-dev](https://groups.google.com/g/tesseract-dev) - For tesseract developers.

Please report an issue only for a **bug**, not for asking questions.

## 📜License

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

Tesseract uses [Leptonica library](http://leptonica.com/) which essentially
uses a [BSD 2-clause license](http://leptonica.com/about-the-license.html).

## 🔗Dependencies

Tesseract uses [Leptonica library](https://github.com/DanBloomberg/leptonica)
for opening input images (e.g. not documents like pdf).
It is suggested to use leptonica with built-in support for [zlib](https://zlib.net),
[png](https://sourceforge.net/projects/libpng) and
[tiff](http://www.simplesystems.org/libtiff) (for multipage tiff).

## 📅Latest Version of README

For the latest online version of the README.md see:

<https://github.com/tesseract-ocr/tesseract/blob/main/README.md>
