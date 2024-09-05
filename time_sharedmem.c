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
    struct timeval *ptr;
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Configure the size of the shared memory
    if (ftruncate(shm_fd, SIZE) == -1) {
        perror("ftruncate");
        return 1;
    }

    // Map the shared memory
    ptr = (struct timeval *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Get the current time, and store it in the shared memory
    gettimeofday(ptr, NULL);

    // Fork a child process
    pid = fork();

    if (pid < 0) { // Handle potential fork error
        perror("fork");
        return 1;
    } else if (pid == 0) { // Child Process
        execvp(argv[1], &argv[1]);
        perror("execvp");
    } else { // Parent Process
        wait(NULL);
        struct timeval end;
        gettimeofday(&end, NULL);
        printf("Elapsed time: %f seconds\n", (end.tv_sec - ptr->tv_sec) + ((end.tv_usec - ptr->tv_usec) / 1000000.0));
    }
    
    // Unmap the shared memory
    shm_unlink(name);
    return 0;
}