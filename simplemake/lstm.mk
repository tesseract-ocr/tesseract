LSTM_INSTHDR = \
	../src/lstm/convolve.h \
	../src/lstm/fullyconnected.h \
	../src/lstm/functions.h \
	../src/lstm/input.h \
	../src/lstm/lstm.h \
	../src/lstm/lstmrecognizer.h \
	../src/lstm/maxpool.h \
	../src/lstm/network.h \
	../src/lstm/networkio.h \
	../src/lstm/networkscratch.h \
	../src/lstm/parallel.h \
	../src/lstm/plumbing.h \
	../src/lstm/recodebeam.h \
	../src/lstm/reconfig.h \
	../src/lstm/reversed.h \
	../src/lstm/series.h \
	../src/lstm/static_shape.h \
	../src/lstm/stridemap.h \
	../src/lstm/tfnetwork.h \
	../src/lstm/weightmatrix.h

LSTM_SRC = \
	../src/lstm/convolve.cpp \
	../src/lstm/fullyconnected.cpp \
	../src/lstm/functions.cpp \
	../src/lstm/input.cpp \
	../src/lstm/lstm.cpp \
	../src/lstm/lstmrecognizer.cpp \
	../src/lstm/maxpool.cpp \
	../src/lstm/network.cpp \
	../src/lstm/networkio.cpp \
	../src/lstm/parallel.cpp \
	../src/lstm/plumbing.cpp \
	../src/lstm/recodebeam.cpp \
	../src/lstm/reconfig.cpp \
	../src/lstm/reversed.cpp \
	../src/lstm/series.cpp \
	../src/lstm/stridemap.cpp \
	../src/lstm/tfnetwork.cpp \
	../src/lstm/weightmatrix.cpp

LSTM_OBJ = $(LSTM_SRC:.cpp=.o)
$(LSTM_OBJ): $(LSTM_INSTHDR)
