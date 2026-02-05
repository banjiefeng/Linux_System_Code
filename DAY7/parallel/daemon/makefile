CC = gcc
CFLAGS = -Wall -Wextra -g 

TARGET = mydeamon

# 默认目标
all: $(TARGET)

$(TARGET): mydaemon.c
	$(CC) $(CFLAGS) -o $@ $^ 

# 清理
clean:
	rm -f $(TARGET)

# 运行
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
