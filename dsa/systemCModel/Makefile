# Compiler And Flag
CXX = g++-11
CXXFLAGS = -g -O0 -I$(SYSTEMC_HOME)/include -I./include -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc -lm

# Source File and Target File
SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)

#  Executable FileName
TARGET = simChain

# Default Targe 
all: $(TARGET)

# Linking Target
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

%.o: src/%.cpp
	$(CXX) -c $< $(CXXFLAGS)

clean:
	rm -f $(OBJS) $(TARGET) *.vcd ./output/out.txt

.PHONY: all clean
