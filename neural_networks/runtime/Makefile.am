AM_CPPFLAGS += \
    -DUSE_STD_NAMESPACE \
    -I$(top_srcdir)/cutil -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/dict \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

noinst_HEADERS = \
    input_file_buffer.h neural_net.h neuron.h

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_neural.la
else
lib_LTLIBRARIES = libtesseract_neural.la
libtesseract_neural_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
endif

libtesseract_neural_la_SOURCES = \
    input_file_buffer.cpp neural_net.cpp neuron.cpp sigmoid_table.cpp


