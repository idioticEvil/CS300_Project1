#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Check for the correct number of arguments
    if (argc < 2) {
        printf("Usage: ./time_sharedmem <command> [args...]\n");
        return 1;
    }

    // Define the process ID
    pid_t pid;

    // Define the size of the shared memory
    const int SIZE = sizeof(struct timeval);
    const char *name = "OS";

    // Create the shared memory
    int shm_fd;
    struct timeval *start;
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Configure the size of the shared memory
    if (ftruncate(shm_fd, SIZE) == -1) {
        perror("ftruncate");
        shm_unlink(name);
        return 1;
    }

    // Map the shared memory
    start = (struct timeval *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (start == MAP_FAILED) {
        perror("mmap");
        shm_unlink(name);
        return 1;
    }

    // Fork a child process
    pid = fork();

    if (pid < 0) { // Handle potential fork error
        perror("fork");
        return 1;
    } else if (pid == 0) { // Child Process
        // Get the current time, and store it in the shared memory
        gettimeofday(start, NULL);
        execvp(argv[1], &argv[1]);
        perror("execvp");
        _exit(1);
    } else { // Parent Process
        wait(NULL);
        struct timeval end;
        gettimeofday(&end, NULL);
        double elapsed = (end.tv_sec - start->tv_sec) + ((end.tv_usec - start->tv_usec) / 1000000.0);
        printf("Elapsed time: %f seconds\n", elapsed);
    }
    
    // Unmap the shared memory
    munmap(start, SIZE);
    close(shm_fd);
    shm_unlink(name);

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