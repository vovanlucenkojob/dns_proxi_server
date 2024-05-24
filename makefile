# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2

# Linker flags
LDFLAGS = -lldns

# Source files
SRCS = config.c dns_proxy.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = dns_proxy

# Default target
all: $(TARGET)

# Rule to link the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and the executable
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean 
