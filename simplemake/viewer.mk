VIEWER_HDR = \
	../src/viewer/scrollview.h \
	../src/viewer/svmnode.h \
	../src/viewer/svutil.h

VIEWER_SRC = \
	../src/viewer/scrollview.cpp \
	../src/viewer/svmnode.cpp \
	../src/viewer/svpaint.cpp \
	../src/viewer/svutil.cpp

VIEWER_OBJ = $(VIEWER_SRC:.cpp=.o)
$(VIEWER_OBJ): $(VIEWER_HDR)
