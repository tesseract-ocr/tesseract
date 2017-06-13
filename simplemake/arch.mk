ARCH_INSTHDR = \
               ../arch/dotproductavx.h \
               ../arch/dotproductsse.h \
               ../arch/simddetect.h

ARCH_SRC = \
           ../arch/dotproductavx.cpp \
           ../arch/dotproductsse.cpp \
           ../arch/simddetect.cpp

ARCH_OBJ = $(ARCH_SRC:.cpp=.o)
$(ARCH_OBJ): $(ARCH_INSTHDR)
