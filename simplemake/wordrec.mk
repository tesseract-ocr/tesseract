WORDREC_HDR = \
	../src/wordrec/wordrec.h

WORDREC_LEGACY_HDR = \
	../src/wordrec/associate.h \
	../src/wordrec/chop.h \
	../src/wordrec/drawfx.h \
	../src/wordrec/findseam.h \
	../src/wordrec/language_model.h \
	../src/wordrec/lm_consistency.h \
	../src/wordrec/lm_pain_points.h \
	../src/wordrec/lm_state.h \
	../src/wordrec/outlines.h \
	../src/wordrec/params_model.h \
	../src/wordrec/plotedges.h \
	../src/wordrec/render.h

WORDREC_SRC = \
	../src/wordrec/tface.cpp \
	../src/wordrec/wordrec.cpp

WORDREC_LEGACY_SRC = \
	../src/wordrec/associate.cpp \
	../src/wordrec/chop.cpp \
	../src/wordrec/chopper.cpp \
	../src/wordrec/drawfx.cpp \
	../src/wordrec/findseam.cpp \
	../src/wordrec/gradechop.cpp \
	../src/wordrec/language_model.cpp \
	../src/wordrec/lm_consistency.cpp \
	../src/wordrec/lm_pain_points.cpp \
	../src/wordrec/lm_state.cpp \
	../src/wordrec/outlines.cpp \
	../src/wordrec/params_model.cpp \
	../src/wordrec/pieces.cpp \
	../src/wordrec/plotedges.cpp \
	../src/wordrec/render.cpp \
	../src/wordrec/segsearch.cpp \
	../src/wordrec/wordclass.cpp

WORDREC_OBJ = $(WORDREC_SRC:.cpp=.o)
$(WORDREC_OBJ): $(WORDREC_HDR)

WORDREC_LEGACY_OBJ = $(WORDREC_LEGACY_SRC:.cpp=.o)
$(WORDREC_LEGACY_OBJ): $(WORDREC_HDR)
