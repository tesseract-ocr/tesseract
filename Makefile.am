## run autogen.sh to create Makefile.in from this file

# Default location for tessdata directory.
# It can be overriden with configure option --datarootdir=DIR.
datadir = @datarootdir@/tessdata

ACLOCAL_AMFLAGS = -I m4

if ENABLE_TRAINING
TRAINING_SUBDIR = src/training
training: all
	@$(MAKE) -C src/training
training-install: training
	@$(MAKE) -C src/training install
training-uninstall:
	@$(MAKE) -C src/training uninstall
clean-local:
	@$(MAKE) -C src/training clean
# Some unit tests use code from training.
check: training
else
training:
	@echo "Need to reconfigure project, so there are no errors"
endif

.PHONY: doc install-langs ScrollView.jar install-jars install-tessdata training

SUBDIRS = src/arch src/ccutil src/viewer src/cutil src/opencl src/ccstruct
SUBDIRS += src/dict src/classify src/wordrec src/textord src/lstm
SUBDIRS += src/ccmain src/api . doc unittest

EXTRA_DIST = README.md LICENSE
EXTRA_DIST += aclocal.m4 config configure.ac autogen.sh
EXTRA_DIST += tesseract.pc.in $(TRAINING_SUBDIR) java doc
EXTRA_DIST += CMakeLists.txt tesseract.pc.cmake cmake VERSION src/vs2010 cppan.yml

# Files for tessdata.
TESSDATA_FILES = $(top_srcdir)/tessdata/pdf.ttf

# Files for tessdata/configs.
CONFIG_FILES = tessdata/pdf.ttf
CONFIG_FILES = inter makebox box.train unlv ambigs.train lstm.train lstmdebug
CONFIG_FILES += api_config kannada box.train.stderr quiet logfile digits get.images
CONFIG_FILES += lstmbox wordstrbox
# Configurations for OCR output.
CONFIG_FILES += alto hocr pdf tsv txt
CONFIG_FILES += linebox rebox strokewidth bigram

# Files for tessdata/tessconfigs.
TESSDATA_CONFIG_FILES = ${CONFIG_FILES:%=$(top_srcdir)/tessdata/configs/%}
TESSCONFIG_FILES = batch batch.nochop nobatch matdemo segdemo msdemo
TESSDATA_TESSCONFIG_FILES = ${TESSCONFIG_FILES:%=$(top_srcdir)/tessdata/tessconfigs/%}

DIST_SUBDIRS = $(SUBDIRS) $(TRAINING_SUBDIR)

uninstall-hook:
	rm -rf $(DESTDIR)$(pkgincludedir)

dist-hook:
# Need to remove .svn directories from directories
# added using EXTRA_DIST. $(distdir)/tessdata would in
# theory suffice.
	rm -rf `find $(distdir) -name .deps -type d`
	-rm -f $(distdir)/*/Makefile $(distdir)/*/*/Makefile
	rm -f `find $(distdir) -name '*~'`
	rm -rf $(find $(distdir)/src/training -executable -type f)
	rm -rf $(distdir)/doc/html/* $(distdir)/doc/*.log

ScrollView.jar:
	@cd "$(top_builddir)/java" && $(MAKE) $@

install-jars:
	@cd "$(top_builddir)/java" && $(MAKE) $@

install-tessdata:
	mkdir -p $(DESTDIR)$(datadir)
	$(INSTALL) -m 644 $(TESSDATA_FILES) $(DESTDIR)$(datadir)
	mkdir -p $(DESTDIR)$(datadir)/configs
	$(INSTALL) -m 644 $(TESSDATA_CONFIG_FILES) $(DESTDIR)$(datadir)/configs
	mkdir -p $(DESTDIR)$(datadir)/tessconfigs
	$(INSTALL) -m 644 $(TESSDATA_TESSCONFIG_FILES) $(DESTDIR)$(datadir)/tessconfigs

doc:
	-srcdir="$(top_srcdir)" builddir="$(top_builddir)" \
	version="@PACKAGE_VERSION@" name="@PACKAGE_NAME@" \
	doxygen $(top_srcdir)/doc/Doxyfile

doc-pack: doc
	-chmod a+r $(top_builddir)/doc/html/*
	 @tar --create --directory=$(top_builddir)/doc/html --verbose --file=- . | gzip -c -9 > $(top_builddir)/@PACKAGE_NAME@-@PACKAGE_VERSION@-doc-html.tar.gz;

doc-clean:
	rm -rf $(top_builddir)/doc/html/*

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tesseract.pc

# fuzzer-api is used for fuzzing tests.
# They are run by OSS-Fuzz https://oss-fuzz.com/, but can also be run locally.
# Note: -fsanitize=fuzzer currently requires the clang++ compiler.

# LIB_FUZZING_ENGINE can be overridden by the caller.
# This is used by OSS-Fuzz.
LIB_FUZZING_ENGINE ?= -fsanitize=fuzzer

fuzzer-api: all
fuzzer-api: $(top_srcdir)/unittest/fuzzers/fuzzer-api.cpp
	$(CXX) $(CXXFLAGS) -g $(LIB_FUZZING_ENGINE) \
          -I $(top_srcdir)/src/api \
          -I $(top_srcdir)/src/ccmain \
          -I $(top_srcdir)/src/ccstruct \
          -I $(top_srcdir)/src/ccutil \
          -I src/api \
          $< \
          src/api/.libs/libtesseract.a \
          $(LEPTONICA_LIBS) \
          $(TENSORFLOW_LIBS) \
          $(libarchive_LIBS) \
          -o $@
