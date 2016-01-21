NEURAL_HDR = \
             ../neural_networks/runtime/input_file_buffer.h \
             ../neural_networks/runtime/neural_net.h \
             ../neural_networks/runtime/neuron.h

NEURAL_SRC = \
             ../neural_networks/runtime/input_file_buffer.cpp \
             ../neural_networks/runtime/neural_net.cpp \
             ../neural_networks/runtime/neuron.cpp \
             ../neural_networks/runtime/sigmoid_table.cpp

NEURAL_OBJ = $(NEURAL_SRC:.cpp=.o)
$(NEURAL_OBJ): $(NEURAL_HDR)
