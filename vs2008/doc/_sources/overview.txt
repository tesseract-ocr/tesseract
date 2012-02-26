:version: $RCSfile: index.rst,v $ $Revision: 76e0bf38aaba $ $Date: 2011/03/22 00:48:41 $

.. default-role:: fs

==========
 Overview
==========

The recommended audience for this document are developers who want to
use Microsoft Visual Studio 2008 with `Tesseract-OCR
<http://code.google.com/p/tesseract-ocr/>`_. If you simply want to *run*
`tesseract` or its various language training applications, then see the
`ReadMe <http://code.google.com/p/tesseract-ocr/wiki/ReadMe>`_. You'll
find instructions there on how to download tesseract's Windows
installer.

|Tesseractocr| consists of:

+ `libtesseract` -- the static (or dynamic) library that does all the
  actual work. As of February 2012 it consists of 260+ `C++` files
  along with 290+ header files.

+ `tesseract.exe` -- the command-line OCR engine. It's built from a
  single, small `C++` file that just calls functions in
  `libtesseract`. There currently isn't very much documentation on how
  to use `tesseract.exe`, but you can look at what's there in the
  repository's `doc
  <http://code.google.com/p/tesseract-ocr/source/browse/#svn%2Ftrunk%2Fdoc>`_
  subdirectory.

+ Language packs -- needed by `tesseract.exe` in order to recognize
  particular languages.

.. _training-applications:

+ Language training applications -- used to teach `tesseract.exe` new
  languages. Each has their own (very brief) man page in the `doc
  <http://code.google.com/p/tesseract-ocr/source/browse/#svn%2Ftrunk%2Fdoc>`_
  subdirectory and include:

  + `ambiguous_words.exe` -- generate sets of words Tesseract is likely
    to find ambiguous

  + `classifier_tester` -- tests a Tesseract character classifier on
    data as formatted for training

  + `cntraining.exe` -- character normalization training

  + `combine_tessdata.exe` -- combine/extract/overwrite Tesseract data

  + `dawg2wordlist.exe` -- convert a Tesseract DAWG to a wordlist

  + `mftraining.exe` -- feature training

  + `shapeclustering.exe` -- shape clustering training

  + `unicharset_extractor.exe` -- extract unicharset from Tesseract
    boxfiles

  + `wordlist2dawg.exe` -- convert a wordlist to a DAWG

  Their use is described in the `TrainingTesseract3
  <http://code.google.com/p/tesseract-ocr/wiki/TrainingTesseract3>`_
  Wiki page.

This document explains how to:

+ :doc:`Setup <setup>` the proper directory structure required to use
  the supplied Visual Studio 2008 Solution

* :doc:`Build <building>` `libtesseract`, `tesseract.exe`, and the
  training apps

* :doc:`Write <programming>` programs that link with `libtesseract`


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
