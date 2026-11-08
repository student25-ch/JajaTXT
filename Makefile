# 编译器
CC = gcc

# 编译选项：-Wall 开启警告，-O2 开启二级优化（对读取大文件极其重要），-I. 指定头文件目录
CFLAGS = -Wall -O2 -I.

# 目标可执行文件名
TARGET = JajaTXT.exe

# 自动获取当前目录下所有的 .c 文件
SRCS = $(wildcard *.c)

# 将 .c 文件替换为对应的 .o 目标文件
OBJS = $(SRCS:.c=.o)

# 默认目标
all: $(TARGET)

# 链接生成可执行文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 编译每个 .c 文件为 .o 文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理编译产物
clean:
	rm -f $(OBJS) $(TARGET)