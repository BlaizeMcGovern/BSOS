//in ssi.c


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include <time.h>

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
    int count = read_directory(disk, current_directory_block, entries, 100);  // Read directory entries

    if (count == -1) {
        printf("Error reading directory\n");
        return;
    }
    display_directory(entries, count);  // Display the entries in the directory
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
            if (strcmp(entries[i].filename, token) == 0 && entries[i].status == 0x02) { // Directory
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


int main() {
    FILE *disk = fopen("BSOS.img", "rb");
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
        } else {
            printf("Unknown command: %s\n", command);
        }
    }

    fclose(disk);
    return 0;
}

