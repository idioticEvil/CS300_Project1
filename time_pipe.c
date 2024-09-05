#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#define READ_END 0
#define WRITE_END 1

int main(int argc, char *argv[]) {
    // Check for the correct number of arguments
    if (argc < 2) {
        printf("Usage: ./time_pipe <command> [args...]\n");
        return 1;
    }

    pid_t pid; // Define the process ID
    int fd[2]; // Define the pipe
    struct timeval start, end; // Define the start and end times

    // Make the pipe
    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    // Fork a child process
    pid = fork();

    if (pid < 0) { // Handle potential fork error
        perror("fork");
        return 1;
    } else if (pid == 0) { // Child Process
        close(fd[READ_END]); // Close read end of the pipe
        gettimeofday(&start, NULL); // Get the start time
        write(fd[WRITE_END], &start, sizeof(struct timeval)); // Write the start time to the pipe
        execvp(argv[1], &argv[1]); // Execute the command
        perror("execvp");
        _exit(1);
    } else { // Parent Process
        close(fd[WRITE_END]); // Close write end of the pipe
        wait(NULL); // Wait for the child process to finish
        read(fd[READ_END], &start, sizeof(struct timeval)); // Read the start time from the pipe
        gettimeofday(&end, NULL); // Get the end time
        double elapsed = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);
        printf("Elapsed time: %f seconds\n", elapsed);
        close(fd[READ_END]); // Close read end of the pipe
    }
    return 0;
}

/*
     __         _                 
  /\ \ \  ___  | |_   ___  ___  _ 
 /  \/ / / _ \ | __| / _ \/ __|(_)
/ /\  / | (_) || |_ |  __/\__ \ _ 
\_\ \/   \___/  \__| \___||___/(_)                             
- This program times how long a command takes to run using shared memory
- Example Code adapted from ZyBooks Sections 3.3 and 3.7
- In accordance with the Responsible AI Use Policy, assistance from ChatGPT 
  and GitHub Copilot Chat was used to understand how to approach the problem 
  and fix bugs. However, AI was not used to write the code.
*/