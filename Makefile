# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

# Target executable
TARGET = diskinfo

# Source files
SRC = diskinfo.c

# Build rules
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Clean up
clean:
	rm -f $(TARGET)

