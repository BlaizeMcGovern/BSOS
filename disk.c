// disk.c

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "disk.h"
#include <arpa/inet.h> // For ntohl

// Function to read a block from the disk image
int read_block(FILE *disk, uint32_t block_number, void *buffer) {
    // Move the file pointer to the correct block
    //printf("block number in read block: %d\n", block_number);
    if (fseek(disk, block_number * 512, SEEK_SET) != 0) {
        return -1;  // Error seeking to the correct block
    }

    // Read the block data into the buffer
    size_t bytes_read = fread(buffer, 1, 512, disk);
    if (bytes_read != 512) {
        return -1;  // Error reading the block
    }

    return 512;  // Successfully read 512 bytes
}

// Function to read directory entries
int read_directory(FILE *disk, uint32_t dir_block, dir_entry_t *entries, int max_entries) {
    char buffer[512];  // Temporary buffer to store block data
    int entry_count = 0;
    //printf("block number in read_dir: %d\n", dir_block);

    if (read_block(disk, dir_block, buffer) != 512) {
        return -1;  // Error reading block
    }

    // Process the buffer to fill the directory entries
    for (int i = 0; i < 8 && entry_count < max_entries; ++i) {  // 8 entries per 512 byte block
        dir_entry_t *entry = (dir_entry_t *)(buffer + i * 64);  // Each entry is 64 bytes

    	// Check if the entry is in use
        if (entry->status != 0x00) {
		
	    memcpy(entries[entry_count].filename, entry->filename, FILENAME_MAX_LEN);
		//printf("entrie: %s\n", entry->filename);
            // Copy the rest of the fields as well
            entries[entry_count].status = entry->status;
            entries[entry_count].starting_block = ntohl(entry->starting_block);
            entries[entry_count].number_of_blocks = ntohl(entry->number_of_blocks);
            entries[entry_count].file_size = ntohl(entry->file_size);
            memcpy(entries[entry_count].creation_time, entry->creation_time, 7);
            memcpy(entries[entry_count].modification_time, entry->modification_time, 7);
            entry_count++;
        }
    }

    return entry_count;  // Return the number of entries read
}

// Function for displaying the directory
void display_directory(const dir_entry_t *entries, int count) {
    for (int i = 0; i < count; ++i) {
        // Determine if the entry is a directory (0x05) or a file (0x03)
	//printf("entry: %s status: %x\n", entries[i].filename, entries[i].status);
        char entry_type = (entries[i].status == 0x05) ? 'D' : 'F';  // 0x05 indicates directory, 0x03 indicates file
        printf("%c %10s\n", entry_type, entries[i].filename);
    
    }

}


