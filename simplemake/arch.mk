ARCH_INSTHDR = \
               ../src/arch/dotproduct.h \
               ../src/arch/dotproductavx.h \
               ../src/arch/dotproductsse.h \
               ../src/arch/intsimdmatrix.h \
               ../src/arch/simddetect.h

ARCH_SRC = \
           ../src/arch/dotproduct.cpp \
           ../src/arch/intsimdmatrix.cpp \
           ../src/arch/simddetect.cpp

# If optimizations are disabled, comment out these lines
ARCH_SRC_OPTIMIZED = \
           ../src/arch/dotproductavx.cpp \
           ../src/arch/dotproductsse.cpp \
           ../src/arch/intsimdmatrixavx2.cpp \
           ../src/arch/intsimdmatrixsse.cpp

ARCH_OBJ = $(ARCH_SRC:.cpp=.o) $(ARCH_SRC_OPTIMIZED:.cpp=.o)
$(ARCH_OBJ): $(ARCH_INSTHDR)
