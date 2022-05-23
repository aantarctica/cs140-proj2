# CS 140 Project 2 | File Server

## Project Specifications
The file server is a C program which accepts user inputs that can write, read, or empty a file. It logs the valid commands taken from the user input into "commands.txt". This stores the read and empty logs into respective files "read.txt" and "empty.txt".

### Accepted User Inputs
1. write < path/to/file > < string >
2. read < path/to/file >
3. empty < path/to/file >

The three commands above are the only valid commands executed by the file server. Make sure that the < path/to/file > and < string > do not exceed 50 characters.

## Executing the file server
1. Before running the file server, make sure that the following files are in the same directory: file_server.c, functions.c, and defs.h.
2. To execute the file server, make sure you are in a linux environment. If you are using Windows, one option is to install Ubuntu from Microsoft Store.
3. Open the terminal.
4. Compile the file_server.c file using `gcc file_server.c -lpthread -o file_server`
5. Execute the compiled file server using `./file_server`
6. Type the valid commands as specified in the above section.
7. Confirm the changes done to your files after execution of the command
8. To exit, press CTRL + C on your keyboard.
9. You may check the commands.txt, read.txt, and empty.txt files to see the log of the commands you used as input. You may delete them after.
