API_INSTHDR = \
              ../src/api/apitypes.h \
              ../src/api/baseapi.h \
              ../src/api/capi.h \
              ../src/api/renderer.h \
              ../src/api/tess_version.h

API_SRC = \
          ../src/api/altorenderer.cpp \
          ../src/api/baseapi.cpp \
          ../src/api/capi.cpp \
          ../src/api/hocrrenderer.cpp \
          ../src/api/lstmboxrenderer.cpp \
          ../src/api/pdfrenderer.cpp \
          ../src/api/renderer.cpp \
          ../src/api/wordstrboxrenderer.cpp

API_OBJ = $(API_SRC:.cpp=.o)
$(API_OBJ): $(API_INSTHDR)

../src/api/tess_version.h: ../src/api/tess_version.h.in
	sed -e 's/@PACKAGE_VERSION@/$(VERSION)/g' < ../src/api/tess_version.h.in > ../src/api/tess_version.h
