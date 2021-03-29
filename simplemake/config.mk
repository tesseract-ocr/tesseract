# Install paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man
TESSDATAPREFIX = $(PREFIX)/share/tesseract-ocr

# Options and optional dependencies (uncomment lines to enable features)
#OPENCL_CPPFLAGS = -DUSE_OPENCL -I/opt/AMDAPP/include
#OPENCL_LDFLAGS = -lOpenCL -ltiff
#NOGRAPHICS_CPPFLAGS = -DGRAPHICS_DISABLED
#EMBEDDED_CPPFLAGS = -DEMBEDDED
#VISIBILITY_CPPFLAGS = -DTESS_EXPORTS -fvisibility=hidden -fvisibility-inlines-hidden
ARCH_CPPFLAGS = -mavx -msse4.1
EXTRA_CPPFLAGS = -DDISABLED_LEGACY_ENGINE

# Optimization flags (if disabled, comment out the marked lines in arch.mk too)
OPTIMIZATION_CPPFLAGS = -DAVX -mavx -DAVX2 -mavx2 -DSSE4_1 -msse4.1
# Optimization for android
#OPTIMIZATION_CPPFLAGS = -DAVX -mavx -DAVX2 -mavx2 -DSSE4_1 -msse4.1 -mfpu=neon

# Enable legacy engine (uncomment lines to enable)
#EXTRA_OBJ = $(CCMAIN_LEGACY_OBJ) $(CCSTRUCT_LEGACY_OBJ) $(CCUTIL_LEGACY_OBJ) $(CLASSIFY_LEGACY_OBJ) $(CUTIL_LEGACY_OBJ) $(DICT_LEGACY_OBJ) $(TEXTORD_LEGACY_OBJ) $(WORDREC_LEGACY_OBJ)
#EXTRA_CPPFLAGS =

# Dependencies needed for training tools
ICU_LDFLAGS = -licui18n -licuuc -licudata
PANGO_CPPFLAGS = -DPANGO_ENABLE_ENGINE -pthread -I/usr/include/pango-1.0 -I/usr/include/cairo \
	-I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
	-I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/lib/i386-linux-gnu/glib-2.0/include \
	-I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/harfbuzz
PANGO_LDFLAGS = -lpangocairo-1.0 -lcairo -lpangoxft-1.0 -lpangoft2-1.0 -lpango-1.0 \
	-lfreetype -lfontconfig -lgobject-2.0 -lglib-2.0 -lharfbuzz

# Mandatory dependencies
LEPT_CPPFLAGS = -I/usr/include/leptonica -I/usr/local/include/leptonica
LEPT_LDFLAGS = -llept -lz -lpng -ljpeg -ltiff

# Needed for static linking (uncomment lines to enable static linking)
#EXTRA_CPPFLAGS = -static
#LEPT_LDFLAGS = -llept -lz -lpng -ljpeg -lgif -ltiff -lwebp -ljbig -lopenjp2 -llzma -lpcre -lffi -lexpat -lfreetype -lthai -ldatrie -lgmodule-2.0 -lglib-2.0 -lharfbuzz -lgraphite2 -lpixman-1 -ldl

# Dependencies needed for scrollview
SCROLLVIEW_CLASSPATH = java/piccolo2d-core-3.0.jar:java/piccolo2d-extras-3.0.jar

# Build tools
CXX = g++
LD = $(CXX)
AR = ar
JAVAC = javac
JAR = jar
