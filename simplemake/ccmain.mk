CCMAIN_HDR = \
             ../ccmain/control.h \
             ../ccmain/cube_reco_context.h \
             ../ccmain/cubeclassifier.h \
             ../ccmain/docqual.h \
             ../ccmain/equationdetect.h \
             ../ccmain/fixspace.h \
             ../ccmain/mutableiterator.h \
             ../ccmain/output.h \
             ../ccmain/paragraphs.h \
             ../ccmain/paragraphs_internal.h \
             ../ccmain/paramsd.h \
             ../ccmain/pgedit.h \
             ../ccmain/reject.h \
             ../ccmain/tessbox.h \
             ../ccmain/tessedit.h \
             ../ccmain/tesseractclass.h \
             ../ccmain/tesseract_cube_combiner.h \
             ../ccmain/tessvars.h \
             ../ccmain/werdit.h

CCMAIN_INSTHDR = \
                 ../ccmain/ltrresultiterator.h \
                 ../ccmain/pageiterator.h \
                 ../ccmain/resultiterator.h \
                 ../ccmain/osdetect.h \
                 ../ccmain/thresholder.h

CCMAIN_SRC = \
             ../ccmain/adaptions.cpp \
             ../ccmain/applybox.cpp \
             ../ccmain/control.cpp \
             ../ccmain/cube_control.cpp \
             ../ccmain/cube_reco_context.cpp \
             ../ccmain/cubeclassifier.cpp \
             ../ccmain/docqual.cpp \
             ../ccmain/equationdetect.cpp \
             ../ccmain/fixspace.cpp \
             ../ccmain/fixxht.cpp \
             ../ccmain/ltrresultiterator.cpp \
             ../ccmain/osdetect.cpp \
             ../ccmain/output.cpp \
             ../ccmain/pageiterator.cpp \
             ../ccmain/pagesegmain.cpp \
             ../ccmain/pagewalk.cpp \
             ../ccmain/par_control.cpp \
             ../ccmain/paragraphs.cpp \
             ../ccmain/paramsd.cpp \
             ../ccmain/pgedit.cpp \
             ../ccmain/recogtraining.cpp \
             ../ccmain/reject.cpp \
             ../ccmain/resultiterator.cpp \
             ../ccmain/superscript.cpp \
             ../ccmain/tesseract_cube_combiner.cpp \
             ../ccmain/tessbox.cpp \
             ../ccmain/tessedit.cpp \
             ../ccmain/tesseractclass.cpp \
             ../ccmain/tessvars.cpp \
             ../ccmain/tfacepp.cpp \
             ../ccmain/thresholder.cpp \
             ../ccmain/werdit.cpp

CCMAIN_OBJ = $(CCMAIN_SRC:.cpp=.o)
$(CCMAIN_OBJ): $(CCMAIN_HDR) $(CCMAIN_INSTHDR)
