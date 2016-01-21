VIEWER_HDR = \
             ../viewer/scrollview.h \
             ../viewer/svmnode.h \
             ../viewer/svutil.h

VIEWER_SRC = \
             ../viewer/scrollview.cpp \
             ../viewer/svmnode.cpp \
             ../viewer/svpaint.cpp \
             ../viewer/svutil.cpp

VIEWER_OBJ = $(VIEWER_SRC:.cpp=.o)
$(VIEWER_OBJ): $(VIEWER_HDR)
