AM_CPPFLAGS += -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct -I$(top_srcdir)/ccmain $(OPENCL_CFLAGS)
noinst_HEADERS = \
    openclwrapper.h oclkernels.h opencl_device_selection.h 

noinst_LTLIBRARIES = libtesseract_opencl.la

libtesseract_opencl_la_SOURCES = \
    openclwrapper.cpp
