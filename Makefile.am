## run autogen.sh to create Makefile.in from this file
ACLOCAL_AMFLAGS = -I m4

if ENABLE_TRAINING
TRAINING_SUBDIR = training
training:
	$(MAKE)
	@cd "$(top_builddir)/training" && $(MAKE)
training-install:
	@cd "$(top_builddir)/training" && $(MAKE) install
training-uninstall:
	@cd "$(top_builddir)/training" && $(MAKE) uninstall
clean-local:
	@cd "$(top_builddir)/training" && $(MAKE) clean
else
training:
	@echo "Need to reconfigure project, so there are no errors"
endif

.PHONY: install-langs ScrollView.jar install-jars training

SUBDIRS = ccutil viewer cutil opencl ccstruct dict classify wordrec textord
if !NO_CUBE_BUILD
    SUBDIRS += neural_networks/runtime cube
endif
SUBDIRS += ccmain api . tessdata doc

EXTRA_DIST = README.md\
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
	rm -rf `find $(distdir) -name .libs`
	rm -rf `find $(distdir) -name *.o`
	rm -rf `find $(distdir) -name *.lo`
	rm -rf `find $(distdir) -name *.la`
	rm -rf `find $(distdir)/training -executable -type f`
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

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tesseract.pc
