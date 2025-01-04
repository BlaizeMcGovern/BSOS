#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USERNAME_FILE "username.txt"

// Function to get the saved username
char *get_saved_username() {
    static char username[256];
    
    FILE *file = fopen(USERNAME_FILE, "r");
    if (file == NULL) {
        return NULL; // No saved username
    }
    
    if (fgets(username, sizeof(username), file) == NULL) {
        fclose(file);
        return NULL; // Handle error or empty file
    }
    username[strcspn(username, "\n")] = '\0';  // Remove newline character
    fclose(file);
    
    return username;
}

// Function to display the prompt in the format username:~/directory
void display_prompt(char *username, const char *current_dir) {
    printf("%s:~/%s$ ", username, current_dir);
}

int main() {
    char username[256];
    
    // Try to retrieve the saved username
    char *saved_username = get_saved_username();
    if (saved_username == NULL) {
        // Ask for username if not saved
        printf("Enter your username: ");
        if (fgets(username, sizeof(username), stdin) == NULL) {
            perror("Error reading username");
            exit(EXIT_FAILURE);
        }
        username[strcspn(username, "\n")] = '\0';  // Remove newline character

        // Save the username to a file for future use
        FILE *file = fopen(USERNAME_FILE, "w");
        if (file == NULL) {
            perror("Error saving username");
            exit(EXIT_FAILURE);
        }
        fprintf(file, "%s", username);
        fclose(file);
    } else {
        // Use the saved username, ensuring null termination after strncpy
        strncpy(username, saved_username, sizeof(username) - 1);
        username[sizeof(username) - 1] = '\0';  // Explicitly null-terminate
    }

    char current_dir[] = "root"; // Start at the root directory
    
    while (1) {
        // Display the prompt
        display_prompt(username, current_dir);

        // Read user input (a command)
        char command[256];
        if (fgets(command, sizeof(command), stdin) != NULL) {
            // Remove the newline character at the end of the command
            command[strcspn(command, "\n")] = '\0';
            
            // If user enters 'exit', break out of the loop to quit the shell
            if (strcmp(command, "exit") == 0) {
                printf("Exiting shell...\n");
                break;
            }
            
            // If the user enters 'diskinfo', run the diskinfo program
            if (strcmp(command, "diskinfo") == 0) {
                int status = system("./diskinfo");  // Run the diskinfo program
                if (status == -1) {
                    perror("Error running diskinfo");
                }
                continue;
            }
            
            // You can add further command handling logic here
            
            printf("Command entered: %s\n", command); // For now, just display the command
        } else {
            perror("Error reading command");
            break;
        }
    }

    return 0;
}

