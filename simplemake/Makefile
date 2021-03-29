include config.mk

VERSION = 5.0.0-alpha-20201231
VERSION_MAJ = 5
VERSION_MIN = 0
VERSION_MIC = 0
LIBVERSION = 5

CPPFLAGS = -DUSE_STD_NAMESPACE -DVERSION=\"$(VERSION)\" -DPACKAGE_VERSION=\"$(VERSION)\" \
           -DTESSDATA_PREFIX=$(TESSDATAPREFIX) \
           -DTESS_COMMON_TRAINING_API= -DTESS_PANGO_TRAINING_API= -DTESS_COMMON_TRAINING_API= -DPANGO_ENABLE_ENGINE -DTESS_UNICHARSET_TRAINING_API= \
           $(OPTIMIZATION_CPPFLAGS) \
           -Wall \
           -fPIC \
           -O2 \
           -std=c++17 -std=c++20 \
           -I../include -I../src \
           -I../src/ccmain -I../src/api -I../src/ccutil -I../src/ccstruct -I../src/viewer -I../src/textord -I../src/dict -I../src/classify -I../src/display -I../src/wordrec -I../src/cutil -I../src/lstm -I../src/arch -I../src/opencl \
           -I../src/training -I../src/training/common -I../src/training/pango -I../src/training/unicharset \
           $(LEPT_CPPFLAGS) \
           $(PANGO_CPPFLAGS) \
           $(OPENCL_CPPFLAGS) $(NOGRAPHICS_CPPFLAGS) $(EMBEDDED_CPPFLAGS) $(VISIBILITY_CPPFLAGS) $(ARCH_CPPFLAGS) \
           $(EXTRA_CPPFLAGS)
LDFLAGS = -lpthread $(EXTRA_LDFLAGS)

BIN = ../tesseract
MAN = ../doc/tesseract.1
LIB = ../libtesseract.so
LIBNAME = libtesseract.so
STATIC_LIB = ../libtesseract.a
TRAINING_LIB = ../libtesseract_training.so
TRAINING_LIBNAME = libtesseract_training.so
TRAINING_STATIC_LIB = ../libtesseract_training.a
SCROLLVIEW = ../java/ScrollView.jar

# Have this rule first so it's the default target
all: autoconfig ../src/config_auto.h $(BIN) $(LIB) $(STATIC_LIB)


# Source and header files

include api.mk
include arch.mk
include ccmain.mk
include ccstruct.mk
include ccutil.mk
include classify.mk
include cutil.mk
include dict.mk
include include.mk
include lstm.mk
include opencl.mk
include textord.mk
include training.mk
include viewer.mk
include wordrec.mk

include tessdata.mk
include scrollview.mk

OBJ = $(API_OBJ) $(ARCH_OBJ) $(CCMAIN_OBJ) $(CCSTRUCT_OBJ) $(CCUTIL_OBJ) $(CLASSIFY_OBJ) $(DICT_OBJ) $(LSTM_OBJ) $(OPENCL_OBJ) $(TEXTORD_OBJ) $(VIEWER_OBJ) $(WORDREC_OBJ) $(EXTRA_OBJ)
$(OBJ): $(INSTHDR)


# Inference rules

.SUFFIXES: .cpp .o .java .class

.cpp.o:
	echo CXX $<
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

.java.class:
	echo JAVAC $<
	$(JAVAC) -encoding UTF8 -sourcepath ../java/ -classpath $(SCROLLVIEW_CLASSPATH) $<


# Target rules

$(STATIC_LIB): $(OBJ)
	echo AR $@
	$(AR) rcs $@ $(OBJ)

$(TRAINING_STATIC_LIB): $(TRAINING_OBJ)
	echo AR $@
	$(AR) rcs $@ $(TRAINING_OBJ)

$(LIB): $(STATIC_LIB)
	echo LD $@
	$(LD) -shared -o $@ -Wl,-soname,$(LIBNAME).$(LIBVERSION) -Wl,--whole-archive $(STATIC_LIB) -Wl,--no-whole-archive $(LDFLAGS) $(LEPT_LDFLAGS)
	chmod -x $@
	ln -sf $(LIBNAME) $@.$(LIBVERSION)

$(TRAINING_LIB): $(TRAINING_STATIC_LIB)
	echo LD $@
	$(LD) -shared -o $@ -Wl,-soname,$(TRAINING_LIBNAME).$(LIBVERSION) -Wl,--whole-archive $(TRAINING_STATIC_LIB) -Wl,--no-whole-archive $(LDFLAGS) $(LEPT_LDFLAGS)
	chmod -x $@
	ln -sf $(TRAINING_LIBNAME) $@.$(LIBVERSION)

$(BIN): ../src/api/tesseractmain.cpp $(LIB)
	echo CXX $@
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) ../src/api/tesseractmain.cpp -o $@ -L.. -ltesseract $(LDFLAGS) $(LEPT_LDFLAGS)

$(TRAINING_BIN): $(LIB) $(TRAINING_LIB) $(TRAINING_BINSRC)
	echo CXX $@
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(PANGOCPPFLAGS) $@.cpp ../src/training/common/commontraining.cpp -o $@ -L.. -ltesseract -ltesseract_training $(LDFLAGS) $(ICU_LDFLAGS) $(PANGO_LDFLAGS) $(LEPT_LDFLAGS)

$(TRAINING_LEGACY_BIN): $(LIB) $(TRAINING_LIB) $(TRAINING_LEGACY_BINSRC)
	echo CXX $@
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(PANGOCPPFLAGS) $@.cpp ../src/training/common/commontraining.cpp -o $@ -L.. -ltesseract -ltesseract_training $(LDFLAGS) $(ICU_LDFLAGS) $(PANGO_LDFLAGS) $(LEPT_LDFLAGS)

$(SCROLLVIEW): $(SCROLLVIEW_CLASSES)
	echo JAR $@
	cd java && $(JAR) cf `basename $(SCROLLVIEW)` `echo $(SCROLLVIEW_CLASSES) $(SCROLLVIEW_EXTRAS) | sed 's/java\///g'`

../src/config_auto.h:
	touch $@

# Virtual rules

training: $(TRAINING_BIN)

training-legacy: $(TRAINING_LEGACY_BIN)

install: all
	echo Installing executables to $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	echo Installing headers to $(DESTDIR)$(PREFIX)/include/tesseract
	mkdir -p $(DESTDIR)$(PREFIX)/include/tesseract
	cp -f $(INSTHDR) $(DESTDIR)$(PREFIX)/include/tesseract
	# TODO: optionally build and install manpages
	#echo Installing manual pages to $(DESTDIR)$(MANPREFIX)/man1
	#mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	#cp -f $(MAN) $(DESTDIR)$(MANPREFIX)/man1
	echo Installing data to $(DESTDIR)$(TESSDATAPREFIX)/tessdata
	mkdir -p $(DESTDIR)$(TESSDATAPREFIX)/tessdata
	cp -f $(MISCDATA) $(DESTDIR)$(TESSDATAPREFIX)/tessdata
	mkdir -p $(DESTDIR)$(TESSDATAPREFIX)/tessdata/configs
	cp -f $(CONFIGS) $(DESTDIR)$(TESSDATAPREFIX)/tessdata/configs
	mkdir -p $(DESTDIR)$(TESSDATAPREFIX)/tessdata/tessconfigs
	cp -f $(TESSCONFIGS) $(DESTDIR)$(TESSDATAPREFIX)/tessdata/tessconfigs
	echo Installing pkg-config file to $(DESTDIR)$(PREFIX)/lib/pkgconfig
	mkdir -p $(DESTDIR)$(PREFIX)/lib/pkgconfig
	sed "s|@prefix@|$(DESTDIR)$(PREFIX)|g; s|@exec_prefix@|$$\{prefix\}|g; s|@bindir@|$$\{exec_prefix\}\/bin|g; s|@datarootdir@|$$\{prefix\}\/share|g; s|@datadir@|$$\{datarootdir\}|g; s|@libdir@|$$\{exec_prefix\}\/lib|g; s|@includedir@|$$\{prefix\}\/include/tesseract|g; s|@PACKAGE_NAME@|tesseract|g; s|@VERSION@|$(VERSION)|g; s|@OPENCL_LIB@|$(OPENCL_LDFLAGS)|g" < ../tesseract.pc.in > $(DESTDIR)$(PREFIX)/lib/pkgconfig/tesseract.pc
	echo Installing libraries to $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp -f $(STATIC_LIB) $(DESTDIR)$(PREFIX)/lib
	cp -f $(LIB) $(DESTDIR)$(PREFIX)/lib/$(LIBNAME).$(VERSION)
	cd $(DESTDIR)$(PREFIX)/lib && ln -sf $(LIBNAME).$(VERSION) $(LIBNAME).$(LIBVERSION)
	cd $(DESTDIR)$(PREFIX)/lib && ln -sf $(LIBNAME).$(VERSION) $(LIBNAME)

install-training: training
	echo Installing executables to $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(TRAINING_BIN) $(DESTDIR)$(PREFIX)/bin
	cp -f $(TRAINING_EXTRA) $(DESTDIR)$(PREFIX)/bin
	# TODO: optionally build and install manpages
	#echo Installing manual pages to $(DESTDIR)$(MANPREFIX)/man1
	#mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	#cp -f $(TRAINING_MAN) $(DESTDIR)$(MANPREFIX)/man1
	#echo Installing manual pages to $(DESTDIR)$(MANPREFIX)/man5
	#mkdir -p $(DESTDIR)$(MANPREFIX)/man5
	#cp -f $(TRAINING_MAN5) $(DESTDIR)$(MANPREFIX)/man5
	echo Installing libraries to $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp -f $(TRAINING_STATIC_LIB) $(DESTDIR)$(PREFIX)/lib
	cp -f $(TRAINING_LIB) $(DESTDIR)$(PREFIX)/lib/$(TRAINING_LIBNAME).$(VERSION)
	cd $(DESTDIR)$(PREFIX)/lib && ln -sf $(TRAINING_LIBNAME).$(VERSION) $(TRAINING_LIBNAME).$(LIBVERSION)
	cd $(DESTDIR)$(PREFIX)/lib && ln -sf $(TRAINING_LIBNAME).$(VERSION) $(TRAINING_LIBNAME)

install-training-legacy: training-legacy
	echo Installing executables to $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(TRAINING_LEGACY_BIN) $(DESTDIR)$(PREFIX)/bin

uninstall:
	echo Removing executables from $(DESTDIR)$(PREFIX)/bin
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN)
	echo Removing libraries from $(DESTDIR)$(PREFIX)/lib
	rm -f $(DESTDIR)$(PREFIX)/lib/$(STATIC_LIB)
	cd $(DESTDIR)$(PREFIX)/lib && rm -f $(LIBNAME) $(LIBNAME).$(VERSION) $(LIBNAME).$(LIBVERSION)
	echo Removing headers from $(DESTDIR)$(PREFIX)/include/tesseract
	rm -rf $(DESTDIR)$(PREFIX)/include/tesseract
	echo Removing manual pages from $(DESTDIR)$(MANPREFIX)/man1
	cd $(DESTDIR)$(MANPREFIX)/man1 && for i in $(MAN); do rm -f `basename $$i`; done
	echo Removing $(DESTDIR)$(TESSDATAPREFIX)/tessdata
	rm -rf $(DESTDIR)$(TESSDATAPREFIX)/tessdata

uninstall-training:
	echo Removing executables from $(DESTDIR)$(PREFIX)/bin
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(TRAINING_BIN) $(TRAINING_LEGACY_BIN)
	echo Removing libraries from $(DESTDIR)$(PREFIX)/lib
	rm -f $(DESTDIR)$(PREFIX)/lib/$(TRAINING_STATIC_LIB)
	cd $(DESTDIR)$(PREFIX)/lib && rm -f $(TRAINING_LIBNAME) $(TRAINING_LIBNAME).$(VERSION) $(TRAINING_LIBNAME).$(LIBVERSION)
	echo Removing manual pages from $(DESTDIR)$(MANPREFIX)/man1
	cd $(DESTDIR)$(MANPREFIX)/man1 && for i in $(TRAINING_MAN); do rm -f `basename $$i`; done
	cd $(DESTDIR)$(MANPREFIX)/man5 && for i in $(TRAINING_MAN5); do rm -f `basename $$i`; done

clean:
	echo Removing all object, executable and library files from source directory
	rm -f $(OBJ) $(TRAINING_OBJ) autoconfig
	rm -f $(TRAINING_BIN) $(TRAINING_LIB) $(TRAINING_STATIC_LIB)
	rm -f $(SCROLLVIEW_CLASSES) $(SCROLLVIEW)
	rm -f `echo $(SCROLLVIEW_EXTRAS) | sed 's/\\\\//g'`
	rm -f $(BIN) $(LIB) $(STATIC_LIB)
	rm -f $(LIB).$(LIBVERSION) $(TRAINING_LIB).$(LIBVERSION)
	rm -rf ../doc/html

dist: clean
	echo Creating tesseract source tarball
	mkdir tesseract-ocr-$(VERSION)
	cp -r ../src \
	      ../AUTHORS ../ChangeLog ../COPYING ../INSTALL ../NEWS ../README ../ReleaseNotes \
	      ../Makefile ../config.mk ../config.uname ../tesseract.pc.in \
	      ../tesseract-ocr-$(VERSION)
	mkdir tesseract-ocr-$(VERSION)/tessdata
	cp $(MISCDATA) tesseract-ocr-$(VERSION)/tessdata
	mkdir tesseract-ocr-$(VERSION)/tessdata/configs
	cp $(CONFIGS) tesseract-ocr-$(VERSION)/tessdata/configs
	mkdir tesseract-ocr-$(VERSION)/tessdata/tessconfigs
	cp $(TESSCONFIGS) tesseract-ocr-$(VERSION)/tessdata/tessconfigs
	cp vs2010/port/strtok_r.* tesseract-ocr-$(VERSION) # hack to make mingw work
	tar c --exclude=.svn tesseract-ocr-$(VERSION) | gzip > ../tesseract-ocr-$(VERSION).tar.gz
	rm -r tesseract-ocr-$(VERSION)
	echo Creating Visual Studio project files package
	zip -q -9 -r --exclude=*.svn* --exclude=*strtok_r* ../tesseract-ocr-$(VERSION)-visualstudio.zip vs2010
	echo Creating doxygen docs tarball
	doxygen doc/Doxyfile
	tar c doc/html | gzip > ../tesseract-ocr-$(VERSION)-doc-html.tar.gz

autoconfig:
	sh config.uname >> config.mk
	touch $@


.SILENT:
