CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -D_POSIX_C_SOURCE=200809L -I.
TARGET = TOS

SRC = \
	main.c \
	common/queue.c \
	common/util.c \
	core/kernel/kernel.c \
	core/process/process.c \
	core/scheduler/scheduler.c \
	user/shell/parser.c \
	user/shell/executor.c \
	user/shell/builtins.c \
	user/shell/shell.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
