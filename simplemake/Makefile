include config.mk

VERSION = 3.05
LIBVERSION = 3

CPPFLAGS = -DUSE_STD_NAMESPACE -DVERSION=\"$(VERSION)\" \
           -DTESSDATA_PREFIX=$(TESSDATAPREFIX) \
           -Wall \
           -fPIC \
           -std=c++11 \
           -I../ccmain -I../api -I../ccutil -I../ccstruct -I../viewer -I../textord -I../dict -I../classify -I../display -I../wordrec -I../cutil -I../neural_networks/runtime -I../cube -I../opencl \
           $(LEPT_CPPFLAGS) \
           $(PANGO_CPPFLAGS) \
           $(OPENCL_CPPFLAGS) $(NOGRAPHICS_CPPFLAGS) $(EMBEDDED_CPPFLAGS) $(VISIBILITY_CPPFLAGS) \
           $(EXTRA_CPPFLAGS)
LDFLAGS = -lpthread $(EXTRA_LDFLAGS)

BIN = ../tesseract
MAN = ../doc/tesseract.1
LIB = ../libtesseract.so
STATIC_LIB = ../libtesseract.a
TRAINING_LIB = ../libtesseract_training.so
TRAINING_STATIC_LIB = ../libtesseract_training.a
SCROLLVIEW = ../java/ScrollView.jar

# Have this rule first so it's the default target
all: autoconfig $(BIN) $(LIB) $(STATIC_LIB)


# Source and header files

include api.mk
include ccmain.mk
include ccstruct.mk
include ccutil.mk
include classify.mk
include cube.mk
include cutil.mk
include dict.mk
include neural_networks.mk
include opencl.mk
include textord.mk
include training.mk
include viewer.mk
include wordrec.mk

include tessdata.mk
include scrollview.mk

OBJ = $(API_OBJ) $(CCMAIN_OBJ) $(CCSTRUCT_OBJ) $(CCUTIL_OBJ) $(CLASSIFY_OBJ) $(CUBE_OBJ) $(CUTIL_OBJ) $(DICT_OBJ) $(NEURAL_OBJ) $(OPENCL_OBJ) $(TEXTORD_OBJ) $(VIEWER_OBJ) $(WORDREC_OBJ) $(EXTRA_OBJ)

INSTHDR = $(API_INSTHDR) $(CCMAIN_INSTHDR) $(CCSTRUCT_INSTHDR) $(CCUTIL_INSTHDR)


# Inference rules

.SUFFIXES: .cpp .o .a .so .java .class

.cpp.o:
	echo CXX $<
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

.a.so:
	echo LD $@
	$(LD) -shared -o $@ -Wl,-soname,$@.$(LIBVERSION) -Wl,--whole-archive $< -Wl,--no-whole-archive $(LDFLAGS) $(LEPT_LDFLAGS)
	chmod -x $@
	ln -sf $@ $@.$(LIBVERSION)

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

$(BIN): ../api/tesseractmain.cpp $(LIB)
	echo CXX $@
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) ../api/tesseractmain.cpp -o $@ -L.. -ltesseract $(LDFLAGS) $(LEPT_LDFLAGS)

$(TRAINING_BIN): $(LIB) $(TRAINING_LIB) $(TRAINING_BINSRC)
	echo CXX $@
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $@.cpp -o $@ -L.. -ltesseract -ltesseract_training $(LDFLAGS) $(ICU_LDFLAGS) $(PANGO_LDFLAGS) $(LEPT_LDFLAGS)

$(SCROLLVIEW): $(SCROLLVIEW_CLASSES)
	echo JAR $@
	cd java && $(JAR) cf `basename $(SCROLLVIEW)` `echo $(SCROLLVIEW_CLASSES) $(SCROLLVIEW_EXTRAS) | sed 's/java\///g'`


# Virtual rules

training: $(TRAINING_BIN)

install: all
	echo Installing executables to $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	echo Installing headers to $(DESTDIR)$(PREFIX)/include/tesseract
	mkdir -p $(DESTDIR)$(PREFIX)/include/tesseract
	cp -f $(INSTHDR) $(DESTDIR)$(PREFIX)/include/tesseract
	echo Installing manual pages to $(DESTDIR)$(MANPREFIX)/man1
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f $(MAN) $(DESTDIR)$(MANPREFIX)/man1
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
	cp -f $(LIB) $(DESTDIR)$(PREFIX)/lib/$(LIB).$(VERSION)
	cd $(DESTDIR)$(PREFIX)/lib && ln -sf $(LIB).$(VERSION) $(LIB).$(LIBVERSION)
	cd $(DESTDIR)$(PREFIX)/lib && ln -sf $(LIB).$(VERSION) $(LIB)

install-training: training
	echo Installing executables to $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(TRAINING_BIN) $(DESTDIR)$(PREFIX)/bin
	echo Installing manual pages to $(DESTDIR)$(MANPREFIX)/man1
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f $(TRAINING_MAN) $(DESTDIR)$(MANPREFIX)/man1
	echo Installing manual pages to $(DESTDIR)$(MANPREFIX)/man5
	mkdir -p $(DESTDIR)$(MANPREFIX)/man5
	cp -f $(TRAINING_MAN5) $(DESTDIR)$(MANPREFIX)/man5
	echo Installing libraries to $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp -f $(TRAINING_STATIC_LIB) $(DESTDIR)$(PREFIX)/lib
	cp -f $(TRAINING_LIB) $(DESTDIR)$(PREFIX)/lib/$(TRAINING_LIB).$(VERSION)
	cd $(DESTDIR)$(PREFIX)/lib && ln -sf $(TRAINING_LIB).$(VERSION) $(TRAINING_LIB).$(LIBVERSION)
	cd $(DESTDIR)$(PREFIX)/lib && ln -sf $(TRAINING_LIB).$(VERSION) $(TRAINING_LIB)

uninstall:
	echo Removing executables from $(DESTDIR)$(PREFIX)/bin
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN)
	echo Removing libraries from $(DESTDIR)$(PREFIX)/lib
	rm -f $(DESTDIR)$(PREFIX)/lib/$(STATIC_LIB)
	cd $(DESTDIR)$(PREFIX)/lib && rm -f $(LIB) $(LIB_MAJVER)
	echo Removing headers from $(DESTDIR)$(PREFIX)/include/tesseract
	rm -rf $(DESTDIR)$(PREFIX)/include/tesseract
	echo Removing manual pages from $(DESTDIR)$(MANPREFIX)/man1
	cd $(DESTDIR)$(MANPREFIX)/man1 && for i in $(MAN); do rm -f `basename $$i`; done
	echo Removing $(DESTDIR)$(TESSDATAPREFIX)/tessdata
	rm -rf $(DESTDIR)$(TESSDATAPREFIX)/tessdata

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
	cp -r ../api ../ccmain ../ccstruct ../ccutil ../classify ../cube ../cutil ../dict ../doc ../java \
	      ../neural_networks ../opencl ../testing ../textord ../training ../viewer ../wordrec \
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
