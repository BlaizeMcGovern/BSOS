# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

# Targets for executables
TARGET1 = diskinfo
TARGET2 = ssi

# Source files
SRC1 = diskinfo.c
SRC2 = ssi.c

# Build rules
all: $(TARGET1) $(TARGET2)

$(TARGET1): $(SRC1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(SRC1)

$(TARGET2): $(SRC2)
	$(CC) $(CFLAGS) -o $(TARGET2) $(SRC2)

# Clean up
clean:
	rm -f $(TARGET1) $(TARGET2)

