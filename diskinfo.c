#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h> // For ntohs and ntohl

// Define the file system superblock structure
typedef struct __attribute__((packed)) superblock_t {
    uint8_t fs_id[8];               // File system identifier (8 bytes)
    uint16_t block_size;            // Block size (2 bytes)
    uint32_t file_system_block_count; // File system size in blocks (4 bytes)
    uint32_t fat_start_block;       // Block where FAT starts (4 bytes)
    uint32_t fat_block_count;       // Number of blocks in FAT (4 bytes)
    uint32_t root_dir_start_block;  // Block where root directory starts (4 bytes)
    uint32_t root_dir_block_count;  // Number of blocks in root directory (4 bytes)
} superblock_t;

int main() {
    const char *filename = "BSOS.img"; // Replace with your disk image file name
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    superblock_t superblock;

    // Read the superblock from the file
    if (fread(&superblock, sizeof(superblock_t), 1, file) != 1) {
        perror("Error reading superblock");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Convert values to host byte order
    uint16_t block_size = ntohs(superblock.block_size);
    uint32_t file_system_block_count = ntohl(superblock.file_system_block_count);
    uint32_t fat_start_block = ntohl(superblock.fat_start_block);
    uint32_t fat_block_count = ntohl(superblock.fat_block_count);
    uint32_t root_dir_start_block = ntohl(superblock.root_dir_start_block);
    uint32_t root_dir_block_count = ntohl(superblock.root_dir_block_count);

    // Display the superblock information
    printf("File System Identifier: %.8s\n", superblock.fs_id);
    printf("Block Size: %u bytes\n", block_size);
    printf("File System Size: %u blocks\n", file_system_block_count);
    printf("FAT Start Block: %u\n", fat_start_block);
    printf("Number of Blocks in FAT: %u\n", fat_block_count);
    printf("Root Directory Start Block: %u\n", root_dir_start_block);
    printf("Number of Blocks in Root Directory: %u\n", root_dir_block_count);

    fclose(file);
    return EXIT_SUCCESS;
}

