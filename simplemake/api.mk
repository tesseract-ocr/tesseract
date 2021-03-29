API_HDR = \
	../include/tesseract/baseapi.h \
	../include/tesseract/capi.h \
	../include/tesseract/ocrclass.h \
	../include/tesseract/osdetect.h \
	../include/tesseract/renderer.h \
	../include/tesseract/resultiterator.h \
	../src/api/pdf_ttf.h

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
$(API_OBJ): $(API_HDR)
