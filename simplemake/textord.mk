TEXTORD_HDR = \
	../src/textord/alignedblob.h \
	../src/textord/baselinedetect.h \
	../src/textord/bbgrid.h \
	../src/textord/blkocc.h \
	../src/textord/blobgrid.h \
	../src/textord/ccnontextdetect.h \
	../src/textord/cjkpitch.h \
	../src/textord/colfind.h \
	../src/textord/colpartition.h \
	../src/textord/colpartitiongrid.h \
	../src/textord/colpartitionset.h \
	../src/textord/devanagari_processing.h \
	../src/textord/drawtord.h \
	../src/textord/edgblob.h \
	../src/textord/edgloop.h \
	../src/textord/fpchop.h \
	../src/textord/gap_map.h \
	../src/textord/imagefind.h \
	../src/textord/linefind.h \
	../src/textord/makerow.h \
	../src/textord/oldbasel.h \
	../src/textord/pithsync.h \
	../src/textord/pitsync1.h \
	../src/textord/scanedg.h \
	../src/textord/sortflts.h \
	../src/textord/strokewidth.h \
	../src/textord/tabfind.h \
	../src/textord/tablefind.h \
	../src/textord/tablerecog.h \
	../src/textord/tabvector.h \
	../src/textord/textlineprojection.h \
	../src/textord/textord.h \
	../src/textord/topitch.h \
	../src/textord/tordmain.h \
	../src/textord/tovars.h \
	../src/textord/underlin.h \
	../src/textord/wordseg.h \
	../src/textord/workingpartset.h

TEXTORD_LEGACY_HDR = \
	../src/textord/equationdetectbase.h

TEXTORD_SRC = \
	../src/textord/alignedblob.cpp \
	../src/textord/baselinedetect.cpp \
	../src/textord/bbgrid.cpp \
	../src/textord/blkocc.cpp \
	../src/textord/blobgrid.cpp \
	../src/textord/ccnontextdetect.cpp \
	../src/textord/cjkpitch.cpp \
	../src/textord/colfind.cpp \
	../src/textord/colpartition.cpp \
	../src/textord/colpartitiongrid.cpp \
	../src/textord/colpartitionset.cpp \
	../src/textord/devanagari_processing.cpp \
	../src/textord/drawtord.cpp \
	../src/textord/edgblob.cpp \
	../src/textord/edgloop.cpp \
	../src/textord/fpchop.cpp \
	../src/textord/gap_map.cpp \
	../src/textord/imagefind.cpp \
	../src/textord/linefind.cpp \
	../src/textord/makerow.cpp \
	../src/textord/oldbasel.cpp \
	../src/textord/pithsync.cpp \
	../src/textord/pitsync1.cpp \
	../src/textord/scanedg.cpp \
	../src/textord/sortflts.cpp \
	../src/textord/strokewidth.cpp \
	../src/textord/tabfind.cpp \
	../src/textord/tablefind.cpp \
	../src/textord/tablerecog.cpp \
	../src/textord/tabvector.cpp \
	../src/textord/textlineprojection.cpp \
	../src/textord/textord.cpp \
	../src/textord/topitch.cpp \
	../src/textord/tordmain.cpp \
	../src/textord/tospace.cpp \
	../src/textord/tovars.cpp \
	../src/textord/underlin.cpp \
	../src/textord/wordseg.cpp \
	../src/textord/workingpartset.cpp

TEXTORD_LEGACY_SRC = \
	../src/textord/equationdetectbase.cpp

TEXTORD_OBJ = $(TEXTORD_SRC:.cpp=.o)
$(TEXTORD_OBJ): $(TEXTORD_HDR)

TEXTORD_LEGACY_OBJ = $(TEXTORD_LEGACY_SRC:.cpp=.o)
$(TEXTORD_LEGACY_OBJ): $(TEXTORD_HDR) $(TEXTORD_LEGACY_HDR)
