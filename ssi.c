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

// Function to change the current directory
int change_directory(FILE *disk, const char *dir_name) {
    dir_entry_t entries[100];
    int count = read_directory(disk, current_directory_block, entries, 100);

    if (count == -1) {
        printf("Error reading directory\n");
        return -1;
    }

    // Search for the directory entry matching dir_name
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].filename, dir_name) == 0){
            // Update the current directory block and path
            current_directory_block = entries[i].starting_block;

            // Update the path to include the new directory
            strcat(current_directory_path, dir_name);

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

