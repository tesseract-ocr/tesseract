## run autogen.sh to create Makefile.in from this file
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = ccutil viewer cutil image ccstruct dict classify wordrec neural_networks/runtime textord cube ccmain api . training java doc tessdata testing   

#if USING_GETTEXT
#SUBDIRS += po
#AM_CPPFLAGS += -DLOCALEDIR=\"$(localedir)\"
#endif

EXTRA_DIST = eurotext.tif phototest.tif ReleaseNotes \
	aclocal.m4 config configure.ac autogen.sh tesseract.spec contrib \
	tesseract.pc.in

#EXTRA_DIST = doc/html doc/@PACKAGE_NAME@_@PACKAGE_VERSION@.pdf doc/@PACKAGE_NAME@_@PACKAGE_VERSION@.ps.gz

uninstall-hook:
	rm -rf $(DESTDIR)$(includedir)

dist-hook:
# Need to remove .svn directories from directories
# added using EXTRA_DIST. $(distdir)/tessdata would in
# theory suffice.
	rm -rf `find $(distdir) -name .svn`
	rm -rf `find $(distdir) -name .deps`

# 'make install' will install only libraries and programs (no language 
#       data files)
# 'make install LANGS=' will install libraries, programs and  all 
#       language datafiles in tessdata/
# 'make install LANGS="eng ara deu"' will install only English, Arabic 
#       and German language datafiles if they are present in tessdata/
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

.PHONY: install-langs ScrollView.jar
install-langs:
	@cd "$(top_builddir)/tessdata" && $(MAKE) $@
	
ScrollView.jar:
	@cd "$(top_builddir)/java" && $(MAKE) $@

doc-dummy:

doc: doc-dummy
	-doxygen doc/Doxyfile

doc-pack: doc
	-chmod a+r $(top_srcdir)/doc/html/*
	 @tar --create --directory=$(top_srcdir)/doc/html --verbose --file=- . | gzip -c -9 > $(top_srcdir)/@PACKAGE_NAME@-@PACKAGE_VERSION@-doc-html.tar.gz;

doc-clean:
	rm -rf $(top_srcdir)/doc/html/*

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tesseract.pc
