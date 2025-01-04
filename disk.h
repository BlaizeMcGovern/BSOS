// disk.h
#ifndef DISK_H
#define DISK_H

#include <stdint.h>

// Define the directory entry structure
#define FILENAME_MAX_LEN 31

#pragma pack(push, 1)  // Disable padding for this structure
typedef struct {
    uint8_t status;
    uint32_t starting_block;
    uint32_t number_of_blocks;
    uint32_t file_size;
    uint8_t creation_time[7];
    uint8_t modification_time[7];
    char filename[FILENAME_MAX_LEN];
    uint8_t unused[6];
} dir_entry_t;
#pragma pack(pop)

// Function prototypes for reading blocks and listing directories
int read_block(FILE *disk, uint32_t block_number, void *buffer);
int read_directory(FILE *disk, uint32_t dir_block, dir_entry_t *entries, int max_entries);  // Declare here
void display_directory(const dir_entry_t *entries, int count);
#endif // DISK_H

