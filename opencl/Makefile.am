AM_CPPFLAGS += -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct -I$(top_srcdir)/ccmain 
if USE_OPENCL
AM_CPPFLAGS += -I$(OPENCL_HDR_PATH)
endif
noinst_HEADERS = \
    openclwrapper.h oclkernels.h opencl_device_selection.h 

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_opencl.la
else
lib_LTLIBRARIES = libtesseract_opencl.la
libtesseract_opencl_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_opencl_la_LIBADD = \
    ../ccutil/libtesseract_ccutil.la \
    ../viewer/libtesseract_viewer.la
if USE_OPENCL
libtesseract_opencl_la_LDFLAGS += $(OPENCL_LIB)
endif
endif

libtesseract_opencl_la_SOURCES = \
    openclwrapper.cpp
