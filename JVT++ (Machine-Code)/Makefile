# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Target executable name
TARGET = sample

# Source files
SOURCES = Machine-code.c functions.c 

# Header files (for dependency tracking)
HEADERS = functions.h mips64.h

# Default target - builds the executable directly from source files
all: $(TARGET)

# Compile and link all source files directly into executable (no .o files)
$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

# Clean up compiled files
clean:
	rm -f $(TARGET) $(TARGET).exe

# Run the program (example usage)
run: $(TARGET)
	./$(TARGET) input.txt

# Phony targets (not actual files)
.PHONY: all clean run