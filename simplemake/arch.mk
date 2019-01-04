ARCH_INSTHDR = \
               ../src/arch/dotproduct.h \
               ../src/arch/dotproductavx.h \
               ../src/arch/dotproductsse.h \
               ../src/arch/intsimdmatrix.h \
               ../src/arch/intsimdmatrixavx2.h \
               ../src/arch/intsimdmatrixsse.h \
               ../src/arch/simddetect.h

ARCH_SRC = \
           ../src/arch/dotproduct.cpp \
           ../src/arch/dotproductavx.cpp \
           ../src/arch/dotproductsse.cpp \
           ../src/arch/intsimdmatrix.cpp \
           ../src/arch/intsimdmatrixavx2.cpp \
           ../src/arch/intsimdmatrixsse.cpp \
           ../src/arch/simddetect.cpp

ARCH_OBJ = $(ARCH_SRC:.cpp=.o)
$(ARCH_OBJ): $(ARCH_INSTHDR)
