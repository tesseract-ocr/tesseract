# Unit Testing for Tesseract

## Requirements

### Files and structure

```

├── langdata_lstm
│   ├── common.punc
│   ├── common.unicharambigs
│   ├── desired_bigrams.txt
│   ├── eng
│   │   ├── desired_characters
│   │   ├── eng.config
│   │   ├── eng.numbers
│   │   ├── eng.punc
│   │   ├── eng.singles_text
│   │   ├── eng.training_text
│   │   ├── eng.unicharambigs
│   │   ├── eng.wordlist
│   │   └── okfonts.txt
│   ├── extended
│   │   └── extended.config
│   ├── extendedhin
│   │   └── extendedhin.config
│   ├── font_properties
│   ├── forbidden_characters_default
│   ├── hin
│   │   ├── hin.config
│   │   ├── hin.numbers
│   │   ├── hin.punc
│   │   └── hin.wordlist
│   ├── kan
│   │   └── kan.config
│   ├── kor
│   │   └── kor.config
│   ├── osd
│   │   └── osd.unicharset
│   └── radical-stroke.txt
├── tessdata
│   ├── ara.traineddata
│   ├── chi_tra.traineddata
│   ├── eng.traineddata
│   ├── heb.traineddata
│   ├── hin.traineddata
│   ├── jpn.traineddata
│   ├── kmr.traineddata
│   ├── osd.traineddata
│   └── vie.traineddata
├── tessdata_best
│   ├── eng.traineddata
│   ├── fra.traineddata
│   ├── kmr.traineddata
│   └── osd.traineddata
├── tessdata_fast
│   ├── eng.traineddata
│   ├── kmr.traineddata
│   ├── osd.traineddata
│   └── script
│       └── Latin.traineddata
└── tesseract
    ...
    ├── test
    ├── unittest
    │   └── third_party/googletest
    └── VERSION
```

### Fonts

* Microsoft fonts: arialbi.ttf, times.ttf, verdana.ttf - [installation guide](https://www.makeuseof.com/tag/how-to-install-microsoft-core-fonts-in-ubuntu-linux/)
* [ae_Arab.ttf](https://www.wfonts.com/download/data/2014/12/03/ae-arab/ae-arab.zip)
* dejavu-fonts: [DejaVuSans-ExtraLight.ttf](https://dejavu-fonts.github.io/Download.html)
* [Lohit-Hindi.ttf](https://raw.githubusercontent.com/pratul/packageofpractices/master/assets/fonts/Lohit-Hindi.ttf)
* [UnBatang.ttf](https://raw.githubusercontent.com/byrongibson/fonts/master/backup/truetype.original/unfonts-core/UnBatang.ttf)

## Run tests

### CMake

By default, CMake uses the `unittest/third_party/googletest` submodule:

```sh
git submodule update --init unittest/third_party/googletest
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

To use an installed GoogleTest package instead, install both GoogleTest and
GoogleMock (for example, `libgtest-dev` and `libgmock-dev` on Debian/Ubuntu) and
configure with `-DUSE_SYSTEM_GTEST=ON`:

```sh
cmake -S . -B build -DBUILD_TESTS=ON -DUSE_SYSTEM_GTEST=ON
```

If the package is installed outside the standard search paths, add its prefix to
`CMAKE_PREFIX_PATH`, or set `GTest_DIR` to the directory containing
`GTestConfig.cmake`. The installed package must provide the `GTest::gtest_main`
and `GTest::gmock` targets. With CMake 3.22, use a package that provides
`GTestConfig.cmake`; CMake's module-based GoogleMock discovery requires 3.23 or
newer.

This replaces only the testing framework, not the test data, language models,
or fonts listed above. Those files are still required by the relevant tests.
When tests are requested but the selected framework is unavailable, configuration
fails rather than silently omitting the tests.

### Autotools

To run the tests, do the following in tesseract folder

```
autoreconf -fiv
git submodule update --init
git clone https://github.com/egorpugin/tessdata tessdata_unittest --depth 1
cp tessdata_unittest/fonts/* test/testing/
mv tessdata_unittest/* ../
export TESSDATA_PREFIX=/prefix/to/path/to/tessdata
make check
```
