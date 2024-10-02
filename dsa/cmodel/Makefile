# 编译器和编译选项
CXX = g++-11
CXXFLAGS = -I$(SYSTEMC_HOME)/include -I./include -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc -lm

# 源文件和目标文件
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

# 可执行文件名
TARGET = simChain

# 默认目标
all: $(TARGET)

# 链接目标
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

# 编译源文件
%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS)

# 清理目标
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
