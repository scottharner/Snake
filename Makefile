# 1. Name of your final executable
TARGET = snake.exe

# 2. List the C files you want to include (add new ones here separated by spaces or backslashes)
SRCS = platform/windows/main.c \
       game/game.c 

# 3. Compiler and flags (including Allegro paths and inline rule)
CC = gcc
CFLAGS = -I"d:\allegro-mingw-4.2.2\include" -fgnu89-inline -Wall -Wextra
LDFLAGS = -L"d:\allegro-mingw-4.2.2\lib" -lalleg

# 4. Automatically convert the .c list to a .o (object file) list
OBJS = $(SRCS:.c=.o)

# Default rule called when you just type 'make'
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile individual .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up compiled files and start fresh
clean:
	rm -f $(OBJS) $(TARGET) 2>/dev/null || true
