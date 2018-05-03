Unit Testing for Tesseract
----------

To run the tests, do the following in tesseract folder

```
autoreconf -fiv
git submodule update --init
export TESSDATA_PREFIX=/prefix/to/path/to/tessdata
make check
```
