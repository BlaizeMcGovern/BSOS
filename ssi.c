// In ssi.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"

// Global variable to store the current directory block
uint32_t current_directory_block = 51;  // Start from the root directory by default

void execute_ls(FILE *disk) {
    dir_entry_t entries[100];  // Assuming a max of 100 entries per directory
    int count = read_directory(disk, current_directory_block, entries, 100);  // Now read_directory is available

    if (count == -1) {
        printf("Error reading directory\n");
        return;
    }
    display_directory(entries, count);
/*
    printf("Listing files in current directory:\n");
    for (int i = 0; i < count; ++i) {
        printf("%s\n", entries[i].filename);
    }
*/
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
        printf("%s:~$ ", username);

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
        } else {
            printf("Unknown command: %s\n", command);
        }
    }

    fclose(disk);
    return 0;
}

