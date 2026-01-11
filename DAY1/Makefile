# ==================== 基础 Makefile ====================
# 最简单的 Makefile，适合单文件项目

# 编译器
CC = gcc
CXX = g++

# 编译选项
CFLAGS = -Wall -Wextra -g
CXXFLAGS = -Wall -Wextra -g -std=c++11

# 目标程序
TARGET = main

# 默认目标
all: $(TARGET)

# 编译 C 程序
$(TARGET): myGetline.c
	$(CC) $(CFLAGS) -o $@ $^

# 编译 C++ 程序				
# $(TARGET): main.cpp
# 	$(CXX) $(CXXFLAGS) -o $@ $^

# 清理
clean:
	rm -f $(TARGET)

# 运行
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
