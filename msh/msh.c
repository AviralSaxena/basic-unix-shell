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
#include <fcntl.h>

#define WHITESPACE " \t\n"
#define MAX_COMMAND_SIZE 255
#define MAX_NUM_ARGUMENTS 32

int main(int argc, char *argv[])
{
    char *tokens[MAX_NUM_ARGUMENTS];
    FILE *input_stream = stdin;

    char error_message[30] = "An error has occurred\n";

    // Batch mode
    if (argc == 2)
    {
        input_stream = fopen(argv[1], "r");
        if (input_stream == NULL)
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
    }
    // No more than one file for batch mode.
    else if (argc > 2)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    while (1)
    {
        // Don't print in batch mode
        if (input_stream == stdin)
        {
            printf("msh> ");
        }

        // Read the input command
        char *working_str = (char *)malloc(MAX_COMMAND_SIZE);
        if(fgets(working_str, MAX_COMMAND_SIZE, input_stream) == NULL){
            // EOF
            break;
        }

        // Tokenize the input string
        int token_count = 0;
        char *arg_ptr;
        while (((arg_ptr = strsep(&working_str,WHITESPACE)) != NULL) && (token_count < MAX_NUM_ARGUMENTS))
        {
            tokens[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
            if (strlen(tokens[token_count]) == 0)
            {
                free(tokens[token_count]);
            }
            else
            {
                token_count++;
            }
        }
        tokens[token_count]=NULL;

        // Empty command
        if (token_count == 0)
        {
            free(working_str);
            continue;
        }

        // cd Command
        else if (strcmp(tokens[0], "cd") == 0)
        {
            // Only one argument allowed to cd
            if (token_count != 2)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            // Change directory or throw an error
            else if (chdir(tokens[1]) != 0)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            free(working_str);
            continue;
        }

        // exit command
        if (strcmp(tokens[0], "exit") == 0)
        {
            // No arguments allowed
            if (token_count > 1)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            }
            else
            {
                exit(0);
            }
        }

        // Redirection
        int redirection_error = 0;
        int redirection_successful = 0;
        for (int i = 0; i < token_count; i++)
        {
            if (strcmp(tokens[i], ">") == 0){
                // Throw an error if there are more than one redirection target or multiple '>'
                if (tokens[i+2] != NULL)
                {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    free(working_str);
                    redirection_error = 1;
                    break;
                }
                else{
                    pid_t redirection_pid = fork();

                    if (redirection_pid == 0){
                        int fd = open( tokens[i+1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
                        if( fd < 0 )
                        {
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            redirection_error=1;             
                        }
                        dup2( fd, 1 );
                        close( fd );
                        tokens[i]=NULL;
                        execvp( tokens[0], tokens);
                        redirection_successful = 1;
                    }
                    else if (redirection_pid>0){
                        wait(NULL);
                    }
                }
            }
        }
        if (redirection_error  || redirection_successful) continue;

        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process. Execute and Terminate.
            tokens[token_count] = NULL;
            if (execvp(tokens[0], tokens) == -1)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(EXIT_FAILURE);
            }
        }
        else if (pid > 0)
        {
            // Parent process. Wait for child to finish and then go to the next iteration.
            wait(NULL);
        }
        free(working_str);
    }

    if (input_stream != stdin)
    {
        fclose(input_stream);
    }

    return 0;
}