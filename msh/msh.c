// The MIT License (MIT)
// 
// Copyright (c) 2024 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#define MAX_COMMAND_SIZE 255
#define MAX_NUM_ARGUMENTS 10

int main(int argc, char *argv[]) {
    char *cmd = (char *)malloc(MAX_COMMAND_SIZE);
    char *tokens[MAX_NUM_ARGUMENTS];
    FILE *input_stream = stdin;

    char error_message[30] = "An error has occurred\n";

    // Batch mode
    if (argc == 2) {
        input_stream = fopen(argv[1], "r");
        if (input_stream == NULL) {
            fprintf(stderr, "Error opening file: %s\n", argv[1]);
            exit(1);
        }
    } else if (argc > 2) {
        fprintf(stderr, "Usage: msh [batchFile]\n");
        exit(1);
    }

    while (1) {

        if (input_stream == stdin) {
            printf("msh> ");
        }

        if (fgets(cmd, MAX_COMMAND_SIZE, input_stream) == NULL) {
            // End of file (Ctrl+D)
            break;
        }

        // Parse input
        int token_count = 0;
        char *arg_ptr;
        char *working_str = strdup(cmd);

        // Tokenize the input string
        while (((arg_ptr = strsep(&working_str, " \t\n")) != NULL) && (token_count < MAX_NUM_ARGUMENTS)) {
            tokens[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
            if (strlen(tokens[token_count]) == 0) {
                free(tokens[token_count]);
            } else {
                token_count++;
            }
        }

        // Empty command
        if (token_count == 0) {
            free(working_str);
            continue;
        }

        // Redirection

        for(int i = 0; i < token_count; i++) {
          if(strcmp(tokens[i], ">") == 0) {
              if(i == token_count - 1 || tokens[i+1] == NULL || strcmp(tokens[i+1], ">") == 0) {
                  write(STDERR_FILENO, error_message, strlen(error_message));
                  free(working_str);
                  continue;
              }
          }
        }

        // Built-in commands
        if (strcmp(tokens[0], "exit") == 0) {
            if (token_count > 1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            } else {
                exit(0);
            }
        } else if (strcmp(tokens[0], "cd") == 0) {
            if (token_count != 2) {
                write(STDERR_FILENO, error_message, strlen(error_message));
            } else if (chdir(tokens[1]) != 0) {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            free(working_str);
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            tokens[token_count] = NULL;
            if (execvp(tokens[0], tokens) == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(EXIT_FAILURE);
            }
        } else if (pid > 0) {
            // Parent process
            wait(NULL);
        } else {
            // Fork failed
            perror("fork");
        }

        free(working_str);
    }

    free(cmd);
    if (input_stream != stdin) {
        fclose(input_stream);
    }

    return 0;
}