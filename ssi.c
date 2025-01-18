//in ssi.c

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include <time.h>
#include <arpa/inet.h>

// Global variables to store current directory block and path
uint32_t current_directory_block = 51;  // Start from the root directory by default
char current_directory_path[512] = "/"; // Current directory path, starting with root

#define DIR_ENTRY_SIZE 64   // Directory entry is 64 bytes
#define FAT_FREE 0x00000000 // Mark for free block in FAT


// Function to update the prompt with the current directory
void update_prompt(char *username) {
    if (strcmp(current_directory_path, "/") == 0) {
        // If in the root directory, display just "$"
        printf("%s:$ ", username);
    } else {
        // Otherwise, show the full path prefixed by "~"
        printf("%s:~/%s$ ", username, current_directory_path + 1);  // Skip the leading slash
    }
}

// Function to display the contents of the current directory
void execute_ls(FILE *disk) {
    dir_entry_t entries[100];  // Assuming a max of 100 entries per directory
    //printf("current dir block in execute ls: %d\n", current_directory_block);
    int count = read_directory(disk, current_directory_block, entries, 100);  // Read directory entries

    if (count == -1) {
        printf("Error reading directory\n");
        return;
    }
    display_directory(entries, count);  // Display the entries in the directory
}


int create_file(FILE *disk, const char *file_name) {
    if (strlen(file_name) > FILENAME_MAX_LEN - 1) {
        printf("File name too long\n");
        return -1;
    }

    // Read the FAT into memory
    uint32_t *fat = malloc(32 * 512);
    if (!fat) {
        printf("Error allocating memory for FAT\n");
        return -1;
    }
    for (int i = 0; i < 32; i++) {
        if (read_block(disk, 1 + i, ((char *)fat) + (i * 512)) != 512) {
            printf("Error reading FAT block %d\n", i);
            free(fat);
            return -1;
        }
    }

    // Read current directory entries
    dir_entry_t entries[100];
    int count = read_directory(disk, current_directory_block, entries, 100);
    if (count == -1) {
        printf("Error reading current directory\n");
        free(fat);
        return -1;
    }

    // Check for duplicate file name
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].filename, file_name) == 0) {
            printf("File '%s' already exists\n", file_name);
            free(fat);
            return -1;
        }
    }

    // Find a free directory entry
    int free_slot = -1;
    for (int i = 0; i < 100; i++) {
        if (entries[i].status == 0x00) { // Unused entry
            free_slot = i;
            break;
        }
    }

    if (free_slot == -1) {
        printf("No free slots in the current directory\n");
        free(fat);
        return -1;
    }

    // Optionally allocate a block for the file (or set it as empty)
    uint32_t first_block = 0xFFFFFFFF; // No blocks allocated for an empty file
    for (uint32_t i = 0; i < (32 * 512) / sizeof(uint32_t); i++) {
        if (fat[i] == FAT_FREE) {
            first_block = i;
            fat[i] = ntohl(0xFFFFFFFF); // Mark as end of chain
            break;
        }
    }

    if (first_block == 0xFFFFFFFF) {
        printf("No free blocks available for the file\n");
        free(fat);
        return -1;
    }

    // Update the directory entry
    memset(&entries[free_slot], 0, sizeof(dir_entry_t));
    entries[free_slot].status = 0x03; // Mark as file
    entries[free_slot].starting_block = ntohl(first_block);
    entries[free_slot].number_of_blocks = ntohl(1);
    entries[free_slot].file_size = ntohl(0);
    strncpy(entries[free_slot].filename, file_name, FILENAME_MAX_LEN);
    memset(entries[free_slot].unused, 0xFF, 6);

    // Write the updated directory and FAT back to disk
    if (write_block(disk, current_directory_block, entries) != 512) {
        printf("Error writing directory block\n");
        free(fat);
        return -1;
    }
    for (int i = 0; i < 32; i++) {
        if (write_block(disk, 1 + i, ((char *)fat) + (i * 512)) != 512) {
            printf("Error writing FAT block %d\n", i);
            free(fat);
            return -1;
        }
    }

    free(fat);
    printf("File '%s' created successfully\n", file_name);
    return 0;
}


// Find the parent directory block by traversing from the root to the parent
uint32_t find_parent_block(FILE *disk, uint32_t current_block) {
    if (strcmp(current_directory_path, "/") == 0) {
        // Already at root, return root block
        return 51;
    }

    char buffer[512];
    dir_entry_t entries[100];
    uint32_t parent_block = 51;  // Start traversal from root block
    char path_copy[512];
    strcpy(path_copy, current_directory_path);  // Copy path to avoid modifying global

    // Remove the last segment from the path to get the parent path
    char *last_slash = strrchr(path_copy, '/');
    if (last_slash) {
        *last_slash = '\0';  // Truncate at the last slash
    }
    if (strlen(path_copy) == 0) {
        strcpy(path_copy, "/");  // If path becomes empty, reset to root
    }

    // Split the path into components and traverse
    char *token = strtok(path_copy, "/");
    while (token) {
        int count = read_directory(disk, parent_block, entries, 100);
        if (count == -1) {
            printf("Error reading directory during traversal\n");
            return 51;  // Default to root on error
        }

        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(entries[i].filename, token) == 0) { // Directory
                parent_block = entries[i].starting_block;
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("Error: Parent directory not found during traversal\n");
            return 51;  // Default to root on error
        }

        token = strtok(NULL, "/");  // Move to the next component
    }

    return parent_block;
}

// Function to change the current directory
int change_directory(FILE *disk, const char *dir_name) {
    // Handle the "cd .." case
    if (strcmp(dir_name, "..") == 0) {
        if (current_directory_block == 51) {
            printf("Already at root directory\n");
            return 0;
        }

        uint32_t parent_block = find_parent_block(disk, current_directory_block);
        current_directory_block = parent_block;

        // Update the path
        char *last_slash = strrchr(current_directory_path, '/');
        if (last_slash) {
            *last_slash = '\0';  // Remove the last component
        }
        if (strlen(current_directory_path) == 0) {
            strcpy(current_directory_path, "/");  // Reset to root if path becomes empty
        }

        return 0;
    }
    // Handle changing to a specific subdirectory
    dir_entry_t entries[100];
    int count = read_directory(disk, current_directory_block, entries, 100);

    if (count == -1) {
        printf("Error reading directory\n");
        return -1;
    }

    // Search for the directory entry matching dir_name
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].filename, dir_name) == 0) { // Ensure it's a directory
            // Update the current directory block and path
            current_directory_block = entries[i].starting_block;

            // Update the path to include the new directory
            if (strcmp(current_directory_path, "/") == 0) {
                strcat(current_directory_path, dir_name);
            } else {
                strcat(current_directory_path, "/");
                strcat(current_directory_path, dir_name);
            }

            return 0;
        }
    }

    printf("Directory '%s' not found\n", dir_name);
    return -1;
}

int create_directory(FILE *disk, const char *dir_name) {
    if (strlen(dir_name) > FILENAME_MAX_LEN - 1) {
        printf("Directory name too long\n");
        return -1;
    }

    // Allocate memory for the entire FAT (32 blocks × 512 bytes)
    uint32_t *fat = malloc(32 * 512);
    if (!fat) {
        printf("Error allocating memory for FAT\n");
        return -1;
    }

    // Read the entire FAT into memory
    for (int i = 0; i < 32; i++) {
        if (read_block(disk, 1 + i, ((char *)fat) + (i * 512)) != 512) {
            printf("Error reading FAT block %d\n", i);
            free(fat);
            return -1;
        }
    }

    // Find the first 8 contiguous free blocks in the FAT
    uint32_t free_blocks[8];
    int found_blocks = 0;
    for (uint32_t i = 0; i < (32 * 512) / sizeof(uint32_t); i++) {
        if (fat[i] == FAT_FREE) {
            free_blocks[found_blocks++] = i;
            if (found_blocks == 8) {
                break;
            }
        } else {
            found_blocks = 0; // Reset if not contiguous
        }
    }

    if (found_blocks < 8) {
        printf("No 8 contiguous free blocks available\n");
        free(fat);
        return -1;
    }

    // Update the FAT to link the 8 blocks
    for (int i = 0; i < 7; i++) {
        fat[free_blocks[i]] = ntohl(free_blocks[i + 1]); // Point to the next block
    }
    fat[free_blocks[7]] = ntohl(0xFFFFFFFF); // Mark the last block as end of chain

    // Write the updated FAT back to the disk
    for (int i = 0; i < 32; i++) {
        if (write_block(disk, 1 + i, ((char *)fat) + (i * 512)) != 512) {
            printf("Error writing FAT block %d\n", i);
            free(fat);
            return -1;
        }
    }

    free(fat); // Free allocated memory for FAT

    // Read the parent directory to find a free slot
    dir_entry_t entries[100];
    int count = read_directory(disk, current_directory_block, entries, 100);
    if (count == -1) {
        printf("Error reading parent directory\n");
        return -1;
    }

    // Check for duplicate directory name
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].filename, dir_name) == 0) {
            printf("Directory '%s' already exists\n", dir_name);
            return -1;
        }
    }

    // Find a free slot in the parent directory
    int free_slot = -1;
    for (int i = 0; i < 100; i++) {
        if (entries[i].status == 0x00) { // Unused entry
            free_slot = i;
            break;
        }
    }

    if (free_slot == -1) {
        printf("No free slots in parent directory\n");
        return -1;
    }

    // Populate the directory entry
    memset(&entries[free_slot], 0, sizeof(dir_entry_t));
    entries[free_slot].status = 0x05; // Mark as directory
    entries[free_slot].starting_block = ntohl(free_blocks[0]); // First block of the directory
    entries[free_slot].number_of_blocks = ntohl(8); // 8 blocks
    entries[free_slot].file_size = ntohl(0); // Directory has no size
    memset(entries[free_slot].creation_time, 0x00, 7);
    memset(entries[free_slot].modification_time, 0x00, 7);
    strncpy(entries[free_slot].filename, dir_name, FILENAME_MAX_LEN);
    memset(entries[free_slot].unused, 0xFF, 6);

    // Write the updated parent directory back to the disk
    if (write_block(disk, current_directory_block, entries) != 512) {
        printf("Error writing parent directory\n");
        return -1;
    }

    // Initialize the 8 blocks allocated for the directory
    char buffer[512] = {0};
    for (int i = 0; i < 8; i++) {
        if (write_block(disk, free_blocks[i], buffer) != 512) {
            printf("Error initializing directory block %d\n", free_blocks[i]);
            return -1;
        }
    }

    printf("Directory '%s' created successfully\n", dir_name);
    return 0;
}



//for writing blocks
int write_block(FILE *disk, uint32_t block_number, const void *buffer) {
    if (fseek(disk, block_number * 512, SEEK_SET) != 0) {
	    //printf("made it here\n");
        return -1;
    }
/*
    if (fwrite(buffer, 1, 512, disk) != 512) {
	        printf("Error writing to disk: %s\n", strerror(errno));
		    printf("ferror() returned: %d\n", ferror(disk));
    }
*/
    return fwrite(buffer, 1, 512, disk) == 512 ? 512 : -1;
}


int delete_file(FILE *disk, const char *file_name) {
    // Read current directory entries
    dir_entry_t entries[100];
    int count = read_directory(disk, current_directory_block, entries, 100);
    if (count == -1) {
        printf("Error reading current directory\n");
        return -1;
    }

    // Find the file in the directory
    int file_index = -1;
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].filename, file_name) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        printf("File '%s' not found\n", file_name);
        return -1;
    }

    // Read the FAT into memory
    uint32_t *fat = malloc(32 * 512);
    if (!fat) {
        printf("Error allocating memory for FAT\n");
        return -1;
    }
    for (int i = 0; i < 32; i++) {
        if (read_block(disk, 1 + i, ((char *)fat) + (i * 512)) != 512) {
            printf("Error reading FAT block %d\n", i);
            free(fat);
            return -1;
        }
    }

    // Free all blocks used by the file in the FAT
    uint32_t block = ntohl(entries[file_index].starting_block);
    while (block != 0xFFFFFFFF) {
        uint32_t next_block = ntohl(fat[block]);
        fat[block] = FAT_FREE;
        block = next_block;
    }

    // Mark the directory entry as unused
    memset(&entries[file_index], 0, sizeof(dir_entry_t));

    // Write the updated directory and FAT back to disk
    if (write_block(disk, current_directory_block, entries) != 512) {
        printf("Error writing directory block\n");
        free(fat);
        return -1;
    }
    for (int i = 0; i < 32; i++) {
        if (write_block(disk, 1 + i, ((char *)fat) + (i * 512)) != 512) {
            printf("Error writing FAT block %d\n", i);
            free(fat);
            return -1;
        }
    }

    free(fat);
    printf("File '%s' deleted successfully\n", file_name);
    return 0;
}


int main() {
    FILE *disk = fopen("dir_testing.img", "r+b");
    if (!disk) {
        perror("Failed to open disk image");
        return 1;
    }

    char username[256];
    printf("Enter your username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        perror("Error reading username");
        fclose(disk);
        return 1;  // Return on error
    }
    username[strcspn(username, "\n")] = '\0';  // Remove newline

    while (1) {
        update_prompt(username);  // Display the current directory in the prompt

        char command[256];
        if (fgets(command, sizeof(command), stdin) == NULL) {
            perror("Error reading command");
            fclose(disk);
            return 1;  // Return on error
        }
        command[strcspn(command, "\n")] = '\0';  // Remove newline

        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "ls") == 0) {
            execute_ls(disk);  // Call the ls handler
        } else if (strncmp(command, "cd ", 3) == 0) {
            // Change directory
	    const char *dir_name = command + 3;
            change_directory(disk,dir_name);
        } else if (strncmp(command, "mkdir ", 6) == 0) {
    		const char *dir_name = command + 6;
    		create_directory(disk, dir_name);
	}else if (strncmp(command, "touch ", 6) == 0) {
    		const char *file_name = command + 6;
    		create_file(disk, file_name); 
	}else if (strncmp(command, "rm ", 3) == 0) {
    		const char *file_name = command + 3;
    		delete_file(disk, file_name);	
    	}else {
            printf("Unknown command: %s\n", command);
        }
    }

    fclose(disk);
    return 0;
}

