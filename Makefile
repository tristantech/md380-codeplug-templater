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

all: $(TARGET)

# Build the program. Require all object files first.
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Remove all build artifacts and codeplugs generated in the test.
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET) test/codeplug_*.rdt

.PHONY: test
test: $(TARGET)
	./$(TARGET) test/blank_codeplug.rdt test/manifest.txt test/