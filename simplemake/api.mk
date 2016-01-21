API_INSTHDR = \
              ../api/apitypes.h \
              ../api/baseapi.h \
              ../api/capi.h \
              ../api/renderer.h

API_SRC = \
          ../api/baseapi.cpp \
          ../api/capi.cpp \
          ../api/pdfrenderer.cpp \
          ../api/renderer.cpp

API_OBJ = $(API_SRC:.cpp=.o)
$(API_OBJ): $(API_INSTHDR)
