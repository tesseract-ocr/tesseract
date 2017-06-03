## run autogen.sh to create Makefile.in from this file

ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS = subdir-objects

if ENABLE_TRAINING
TRAINING_SUBDIR = training
training:
	$(MAKE)
	@cd "$(top_builddir)/training" && $(MAKE)
training-install:
	@cd "$(top_builddir)/training" && $(MAKE) install
training-uninstall:
	@cd "$(tpkgconfigdirop_builddir)/training" && $(MAKE) uninstall
clean-local:
	@cd "$(top_builddir)/training" && $(MAKE) clean
else
training:
	@echo "Need to reconfigure project, so there are no errors"
endif


lib_LTLIBRARIES = libtesseract.la

libtesseract_la_SOURCES =
include_HEADERS =

include api/Makefile.am
include arch/Makefile.am
include ccmain/Makefile.am
include ccstruct/Makefile.am
include ccutil/Makefile.am
include classify/Makefile.am
include cutil/Makefile.am
include dict/Makefile.am
include lstm/Makefile.am
include opencl/Makefile.am
include textord/Makefile.am
include viewer/Makefile.am
include wordrec/Makefile.am

SUBDIRS = . tessdata doc


AM_CPPFLAGS += \
    -DLOCALEDIR=\"$(localedir)\"\
    -DUSE_STD_NAMESPACE \
    -I$(top_srcdir)/api \
    -I$(top_srcdir)/arch \
    -I$(top_srcdir)/ccmain \
    -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/classify \
    -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/dict \
    -I$(top_srcdir)/lstm \
    -I$(top_srcdir)/opencl \
    -I$(top_srcdir)/textord \
    -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/wordrec

if T_WIN
AM_CPPFLAGS += \
    -I$(top_srcdir)/vs2010/port \
    -DWINDLLNAME=\"lib@GENERIC_LIBRARY_NAME@\"
endif

if !NO_TESSDATA_PREFIX
AM_CPPFLAGS += -DTESSDATA_PREFIX=@datadir@/
endif

AM_CPPFLAGS += $(OPENCL_CPPFLAGS)

AM_CXXFLAGS += $(OPENMP_CXXFLAGS)

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS
AM_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden 
endif


libtesseract_la_CPPFLAGS = $(AM_CPPFLAGS)
libtesseract_la_CXXFLAGS = $(AM_CXXFLAGS)

if AVX_OPT
libtesseract_la_CXXFLAGS += -mavx
endif

if SSE41_OPT
libtesseract_la_CXXFLAGS += -msse4.1
endif

libtesseract_la_LDFLAGS = $(AM_LDFLAGS)
libtesseract_la_LDFLAGS += $(LEPTONICA_LIBS)
libtesseract_la_LDFLAGS += $(OPENCL_LDFLAGS)

libtesseract_la_LDFLAGS += \
    -version-info $(GENERIC_LIBRARY_VERSION) -no-undefined

if T_WIN
libtesseract_la_LDFLAGS += -no-undefined -Wl,--as-needed -lws2_32
endif


bin_PROGRAMS = tesseract

tesseract_SOURCES = api/tesseractmain.cpp

tesseract_LDADD = libtesseract.la

tesseract_CPPFLAGS = $(AM_CPPFLAGS)
tesseract_CXXFLAGS = $(AM_CXXFLAGS)

tesseract_LDFLAGS = $(OPENCL_LDFLAGS)

tesseract_LDADD += $(LEPTONICA_LIBS)
tesseract_LDADD += $(OPENMP_CXXFLAGS)

if T_WIN
tesseract_LDADD += -ltiff
tesseract_LDADD += -lws2_32
endif

if ADD_RT
tesseract_LDADD += -lrt
endif


EXTRA_DIST = \
    README.md \
    aclocal.m4 \
    config \
    configure.ac \
    autogen.shpkgconfigdir \
    contrib \
    tesseract.pc.in \
    $(TRAINING_SUBDIR) \
    java \
    doc \
    testing

DIST_SUBDIRS  = $(SUBDIRS) $(TRAINING_SUBDIR)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tesseract.pc

.PHONY: install-langs ScrollView.jar install-jars training

uninstall-hook:
	rm -rf $(DESTDIR)$(includedir)

dist-hook:
# Need to remove .svn directories from directories
# added using EXTRA_DIST. $(distdir)/tessdata would in
# theory suffice.
	rm -rf $(find $(distdir) -name .svn)
	rm -rf $(find $(distdir) -name .git)
	rm -rf $(find $(distdir) -name .deps)
	rm -rf $(find $(distdir) -name .libs)
	rm -rf $(find $(distdir) -name *.o)
	rm -rf $(find $(distdir) -name *.lo)
	rm -rf $(find $(distdir) -name *.la)
	rm -rf $(find $(distdir)/training -executable -type f)
	rm -rf $(distdir)/doc/html/*

ScrollView.jar:
	@cd "$(top_builddir)/java" && $(MAKE) $@

install-jars:
	@cd "$(top_builddir)/java" && $(MAKE) $@

doc-dummy:

doc: doc-dummy
	-srcdir="$(top_srcdir)" builddir="$(top_builddir)" \
	version="@PACKAGE_VERSION@" name="@PACKAGE_NAME@" \
	doxygen $(top_srcdir)/doc/Doxyfile

doc-pack: doc
	-chmod a+r $(top_srcdir)/doc/html/*
	 @tar --create --directory=$(top_srcdir)/doc/html --verbose --file=- . | gzip -c -9 > $(top_srcdir)/@PACKAGE_NAME@-@PACKAGE_VERSION@-doc-html.tar.gz;

doc-clean:
	rm -rf $(top_srcdir)/doc/html/*

