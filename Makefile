SOURCES = \
    programmer.c \
	utils.c \

# Executable name
TARGET = templater

# Compiler options
CC = gcc
CFLAGS = -O3 -Wall

# Subsitute *.c -> *.o to get object file names
OBJS = $(SOURCES:.c=.o)

# Build the program. Require all object files first.
all: $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

.PHONY: clean

# Remove all build artifacts
clean:
	rm -f $(OBJS) $(TARGET)