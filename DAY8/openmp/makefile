CC = gcc
CFLAGS = -Wall -Wextra -g -lpthread -fopenmp

TARGET = main

# 默认目标
all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $@ $^ 

# 清理
clean:
	rm -f $(TARGET)

# 运行
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
