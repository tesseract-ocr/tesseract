# autotools (LINUX/UNIX , msys...)

If you have cloned Tesseract from GitHub, you must generate
the configure script.

If you have tesseract 4.0x installation in your system, please remove it
before new build.

You need Leptonica 1.74.2 (minimum) for Tesseract 4.0x.

Known dependencies for training tools (excluding leptonica):
 * compiler with c++11 support
 * automake
 * pkg-config
 * pango-devel
 * cairo-devel
 * icu-devel

So, the steps for making Tesseract are:

    $ ./autogen.sh
    $ ./configure
    $ make
    $ sudo make install
    $ sudo ldconfig
    $ make training
    $ sudo make training-install

You need to install at least English language and OSD traineddata files to
`TESSDATA_PREFIX` directory.

You can retrieve single file with tools like [wget](https://www.gnu.org/software/wget/), [curl](https://curl.haxx.se/), [GithubDownloader](https://github.com/intezer/GithubDownloader) or browser.

All language data files can be retrieved from git repository (useful only for packagers!).
(Repository is huge - more that 1.2 GB. You do NOT need to download traineddata files for
all languages).

    $ git clone https://github.com/tesseract-ocr/tessdata.git tesseract-ocr.tessdata


You need an Internet connection and [curl](https://curl.haxx.se/) to compile `ScrollView.jar`
because the build will automatically download
[piccolo2d-core-3.0.jar](http://search.maven.org/remotecontent?filepath=org/piccolo2d/piccolo2d-core/3.0/piccolo2d-core-3.0.jar > piccolo2d-core-3.0.jar) and
[piccolo2d-extras-3.0.jar](http://search.maven.org/remotecontent?filepath=org/piccolo2d/piccolo2d-extras/3.0/piccolo2d-extras-3.0.jar) and
[jaxb-api-2.3.1.jar](http://search.maven.org/remotecontent?filepath=javax/xml/bind/jaxb-api/2.3.1/jaxb-api-2.3.1.jar) and place them to `tesseract/java`.

Just run:

    $ make ScrollView.jar

and follow the instruction on [Viewer Debugging wiki](https://github.com/tesseract-ocr/tesseract/wiki/ViewerDebugging).


# CMAKE

There is alternative build system based on multiplatform [cmake](https://cmake.org/)

## LINUX

    $ mkdir build
    $ cd build && cmake .. && make
    $ sudo make install


## WINDOWS

See [Wiki](https://github.com/tesseract-ocr/tesseract/wiki) for more information on this.
