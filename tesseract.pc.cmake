prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@

Name: @tesseract_NAME@
Description: An OCR Engine that was developed at HP Labs (1985-1995) and Google (2006-2018).
URL: https://github.com/tesseract-ocr/tesseract
Version: @tesseract_VERSION@
Requires.private: lept
Libs: -L${libdir} -l@tesseract_OUTPUT_NAME@ @libarchive_LIBS@ @libcurl_LIBS@
Libs.private:
Cflags: -I${includedir}
