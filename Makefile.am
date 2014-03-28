## run autogen.sh to create Makefile.in from this file
ACLOCAL_AMFLAGS = -I m4

if ENABLE_TRAINING
TRAINING_SUBDIR = training
training:
	@cd "$(top_builddir)/training" && $(MAKE)
training-install:
	@cd "$(top_builddir)/training" && $(MAKE) install
clean-local:
	@cd "$(top_builddir)/training" && $(MAKE) clean
else
training:
	@echo "Need to reconfigure project, so there are no errors"
endif

.PHONY: install-langs ScrollView.jar install-jars $(TRAINING_SUBDIR)

SUBDIRS = ccutil viewer cutil opencl ccstruct dict classify wordrec neural_networks/runtime textord cube ccmain api . tessdata doc

EXTRA_DIST = eurotext.tif phototest.tif ReleaseNotes \
	aclocal.m4 config configure.ac autogen.sh contrib \
	tesseract.pc.in $(TRAINING_SUBDIR) java doc testing

DIST_SUBDIRS  = $(SUBDIRS) $(TRAINING_SUBDIR)

uninstall-hook:
	rm -rf $(DESTDIR)$(includedir)

dist-hook:
# Need to remove .svn directories from directories
# added using EXTRA_DIST. $(distdir)/tessdata would in
# theory suffice.
	rm -rf `find $(distdir) -name .svn`
	rm -rf `find $(distdir) -name .git`
	rm -rf `find $(distdir) -name .deps`

# 'make install' will install only libraries and programs (no language 
#       data files)
# 'make install LANGS=' will install libraries, programs and  all 
#       language datafiles in tessdata/
# 'make install LANGS="eng ara deu"' will install libraries, programs
#       English, Arabic and German language datafiles if they are present
#       in tessdata/
# 'make install-langs LANGS="eng ara deu"' will install only English,
#	Arabic and German language datafiles if they are present
#       in tessdata/
install-data-hook:
	@if test $${LANGS+defined}; then \
		if test "$${LANGS}" == ""; then \
			echo ____All language files will be installed; \
		else \
			echo ___Folowing language files will be installed: "$$LANGS"; \
		fi; \
		cd "$(top_builddir)/tessdata" && $(MAKE) install-langs LANG="${LANGS}"; \
	else \
		echo No language file is installed.; \
	fi;

install-langs:
	@cd "$(top_builddir)/tessdata" && $(MAKE) $@

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

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tesseract.pc
