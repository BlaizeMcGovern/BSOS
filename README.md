# Simple File System and Shell in C
## Overview
This project implements a simple file system and a shell for interacting with the file system in C. The file system is based on a disk image with a fixed block size of 512 bytes, and the shell allows users to interact with the file system, view disk information, and eventually manage files and directories.

The file system is structured with essential components like the File Allocation Table (FAT), root directory, and basic metadata stored in the disk image. The shell allows users to interact with the file system by running commands such as diskinfo and eventually viewing directories and files.
## Features
File System:

  512-byte block size.
  FAT table to track the allocation status of blocks.
  A root directory to store files.
  Basic file system operations (such as listing disk info).

Shell:

  Prompts for a username when launched and remembers it between sessions.
  Displays a prompt in the form username:~/directory when running commands.
  Runs the diskinfo program to display file system information.
  (Future updates will allow interaction with directories and files).
## Getting started
To use the shell and view disk information, follow these steps:
### Prerequisites
Linux-based system.
GCC compiler installed.
### Installation
Clone the repository:

Copy code
Navigate into the project directory:

cd simple-file-system
Compile the project:


make
Run the shell:

./ssi

### Usage
When you run the shell for the first time, you will be prompted to enter a username.
The shell will remember your username for future sessions.
You can run the command diskinfo to view basic information about the file system, including block size, total number of blocks, FAT location, and root directory information.
### Example
```bash
$ ./ssi
Enter your username: test
test:~/directory$ diskinfo
File System Identifier: EX
Block Size: 512 bytes
File System Size: 6400 blocks
FAT Start Block: 1
Number of Blocks in FAT: 32
Root Directory Start Block: 33
Number of Blocks in Root Directory: 64
```
### Directory Structure
ssi.c: The main shell program.
diskinfo.c: A program to display file system information.
Makefile: Used to build the project.
diskimage.img: A sample disk image used by the file system.
### Future Features
Directory and File Management:
List files and directories in the root directory.
Create, delete, and modify files.
Navigate between directories.
### Error Handling:
Improved error messages and checks for invalid operations.
Enhanced Shell:
Support for additional commands and more advanced file system operations.
