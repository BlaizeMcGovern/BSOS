# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

# Target executables
TARGET_SSI = ssi
TARGET_DISKINFO = diskinfo

# Source files
SRC_SSI = ssi.c
SRC_DISKINFO = diskinfo.c
SRC_DISK = disk.c  # Add disk.c here

# Build rules
all: $(TARGET_SSI) $(TARGET_DISKINFO)

$(TARGET_SSI): $(SRC_SSI) disk.h
	$(CC) $(CFLAGS) -o $(TARGET_SSI) $(SRC_SSI) $(SRC_DISK)

$(TARGET_DISKINFO): $(SRC_DISKINFO) disk.h
	$(CC) $(CFLAGS) -o $(TARGET_DISKINFO) $(SRC_DISKINFO) $(SRC_DISK)

# Clean up
clean:
	rm -f $(TARGET_SSI) $(TARGET_DISKINFO)

