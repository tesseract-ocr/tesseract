# autotools (LINUX/UNIX , msys...)

If you have cloned Tesseract from GitHub, you must generate
the configure script.

If you have tesseract 3.0x installation in your system, please remove it
before new build.

Known dependencies for training tools (excluding leptonica):
 * compiler with c++ support
 * pango-devel
 * cairo-devel
 * icu-devel

So, the steps for making Tesseract are:

    $ ./autogen.sh
    $ ./configure
    $ make
    $ sudo make install
    $ make training
    $ sudo make training-install

You need to install at least English language and OSD data files to TESSDATA_PREFIX
directory. You can retrieve single file with tools like [wget](https://www.gnu.org/software/wget/), [curl](https://curl.haxx.se/), [GithubDownloader](https://github.com/intezer/GithubDownloader) or browser.

All language data files can be retrieved from git repository (useful only for packagers!):

    $ git clone https://github.com/tesseract-ocr/tessdata.git tesseract-ocr.tessdata

(Repository is huge - more that 1.2 GB. You do not need to download
all languages). 

To compile ScrollView.jar you need to download piccolo2d-core-3.0.jar
and [piccolo2d-extras-3.0.jar](http://search.maven.org/#search|ga|1|g%3A%22org.piccolo2d%22) and place them to tesseract/java.

Then run:

    $ make ScrollView.jar

and follow instruction on [Viewer Debugging wiki](https://github.com/tesseract-ocr/tesseract/wiki/ViewerDebugging).


# CMAKE

There is alternative build system based on multiplatform [cmake](https://cmake.org/)

## LINUX

    $ mkdir build
    $ cd build && cmake .. && make
    $ sudo make install


## WINDOWS

You need to use leptonica with cmake patch:

    git clone https://github.com/DanBloomberg/leptonica.git
    cd leptonica
    mkdir build
    cd build
    cmake ..
    cmake --build .
    cd ..\..
    git clone https://github.com/tesseract-ocr/tesseract.git
    cd tesseract
    mkdir build
    cd build
    cmake .. -DLeptonica_BUILD_DIR=\abs\path\to\leptonica\build
    cmake --build .


# WINDOWS Visual Studio

Please read http://vorba.ch/2014/tesseract-3.03-vs2013.html
