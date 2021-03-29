ARCH_HDR = \
	../src/arch/dotproduct.h \
	../src/arch/intsimdmatrix.h \
	../src/arch/simddetect.h

ARCH_SRC = \
	../src/arch/dotproduct.cpp \
	../src/arch/intsimdmatrix.cpp \
	../src/arch/simddetect.cpp

ARCH_SRC_OPTIMIZED = \
	../src/arch/dotproductavx.cpp \
	../src/arch/dotproductsse.cpp \
	../src/arch/intsimdmatrixavx2.cpp \
	../src/arch/intsimdmatrixsse.cpp
	#../src/arch/dotproductfma.cpp \
	#../src/arch/intsimdmatrixneon.cpp

# If optimizations are disabled, replace the following line with this one:
#ARCH_OBJ = $(ARCH_SRC:.cpp=.o)
ARCH_OBJ = $(ARCH_SRC:.cpp=.o) $(ARCH_SRC_OPTIMIZED:.cpp=.o)
$(ARCH_OBJ): $(ARCH_HDR)
