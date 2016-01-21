OPENCL_HDR = \
             ../opencl/oclkernels.h \
             ../opencl/openclwrapper.h \
             ../opencl/opencl_device_selection.h

OPENCL_SRC = ../opencl/openclwrapper.cpp

OPENCL_OBJ = $(OPENCL_SRC:.cpp=.o)
$(OPENCL_OBJ): $(OPENCL_HDR)
