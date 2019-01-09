AM_CPPFLAGS += $(OPENCL_CFLAGS) \
    -I$(top_srcdir)/src/ccutil \
    -I$(top_srcdir)/src/ccstruct \
    -I$(top_srcdir)/src/ccmain

noinst_HEADERS = \
    openclwrapper.h oclkernels.h

noinst_LTLIBRARIES = libtesseract_opencl.la

libtesseract_opencl_la_SOURCES = \
    openclwrapper.cpp
