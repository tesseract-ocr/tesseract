OPENCL_HDR = \
	../src/opencl/oclkernels.h \
	../src/opencl/openclwrapper.h

OPENCL_SRC = ../src/opencl/openclwrapper.cpp

OPENCL_OBJ = $(OPENCL_SRC:.cpp=.o)
$(OPENCL_OBJ): $(OPENCL_HDR)
