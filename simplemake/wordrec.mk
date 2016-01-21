WORDREC_HDR = \
              ../wordrec/associate.h \
              ../wordrec/chop.h \
              ../wordrec/chopper.h \
              ../wordrec/drawfx.h \
              ../wordrec/findseam.h \
              ../wordrec/gradechop.h \
              ../wordrec/language_model.h \
              ../wordrec/lm_consistency.h \
              ../wordrec/lm_pain_points.h \
              ../wordrec/lm_state.h \
              ../wordrec/measure.h \
              ../wordrec/outlines.h \
              ../wordrec/params_model.h \
              ../wordrec/plotedges.h \
              ../wordrec/render.h \
              ../wordrec/wordrec.h

WORDREC_SRC = \
              ../wordrec/associate.cpp \
              ../wordrec/chop.cpp \
              ../wordrec/chopper.cpp \
              ../wordrec/drawfx.cpp \
              ../wordrec/findseam.cpp \
              ../wordrec/gradechop.cpp \
              ../wordrec/language_model.cpp \
              ../wordrec/lm_consistency.cpp \
              ../wordrec/lm_pain_points.cpp \
              ../wordrec/lm_state.cpp \
              ../wordrec/outlines.cpp \
              ../wordrec/params_model.cpp \
              ../wordrec/pieces.cpp \
              ../wordrec/plotedges.cpp \
              ../wordrec/render.cpp \
              ../wordrec/segsearch.cpp \
              ../wordrec/tface.cpp \
              ../wordrec/wordclass.cpp \
              ../wordrec/wordrec.cpp

WORDREC_OBJ = $(WORDREC_SRC:.cpp=.o)
$(WORDREC_OBJ): $(WORDREC_HDR)
