# Configuration for Tesseract OCR Engine

# Installation paths
prefix = @CMAKE_INSTALL_PREFIX@
exec_prefix = ${prefix}/bin
libdir = ${prefix}/@CMAKE_INSTALL_LIBDIR@
includedir = ${prefix}/include

# Package Information
Name: @tesseract_NAME@
Description: An OCR Engine developed at HP Labs between 1985 and 1995, now maintained by Google.
URL: https://github.com/tesseract-ocr/tesseract
Version: @tesseract_VERSION@
Requires.private: lept
Libs: -L${libdir} -l@tesseract_OUTPUT_NAME@ @libarchive_LIBS@ @libcurl_LIBS@ @TENSORFLOW_LIBS@
Libs.private:
Cflags: -I${includedir}
