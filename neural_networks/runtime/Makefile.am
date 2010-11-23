SUBDIRS =
AM_CPPFLAGS = \
    -DUSE_STD_NAMESPACE \
    -I$(top_srcdir)/cutil -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/dict \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer

include_HEADERS = \
    input_file_buffer.h neural_net.h neuron.h

lib_LTLIBRARIES = libtesseract_neural.la
libtesseract_neural_la_SOURCES = \
    input_file_buffer.cpp neural_net.cpp neuron.cpp sigmoid_table.cpp

libtesseract_neural_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
