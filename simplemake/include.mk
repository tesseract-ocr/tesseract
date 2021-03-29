INSTHDR = \
	../include/tesseract/baseapi.h \
	../include/tesseract/capi.h \
	../include/tesseract/export.h \
	../include/tesseract/ltrresultiterator.h \
	../include/tesseract/ocrclass.h \
	../include/tesseract/osdetect.h \
	../include/tesseract/pageiterator.h \
	../include/tesseract/publictypes.h \
	../include/tesseract/renderer.h \
	../include/tesseract/resultiterator.h \
	../include/tesseract/unichar.h \
	../include/tesseract/version.h

../include/tesseract/version.h: ../include/tesseract/version.h.in
	sed -e 's/@PACKAGE_VERSION@/$(VERSION)/g;s/@GENERIC_MAJOR_VERSION@/$(VERSION_MAJ)/g;s/@GENERIC_MINOR_VERSION@/$(VERSION_MIN)/g;s/@GENERIC_MICRO_VERSION@/$(VERSION_MIC)/g' < ../include/tesseract/version.h.in > ../include/tesseract/version.h
