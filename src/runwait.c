#define _POSIX_C_SOURCE 199309L
#include "common.h"
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static void usage(const char *a) {
    fprintf(stderr, "Usage: %s <cmd> [args]\n", a);
    exit(1);
}

static double d(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(int c, char **v) {
    if (c < 2)
        usage(v[0]);

    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    
    if (pid == 0) {
        execvp(v[1], &v[1]);
        
        perror("execvp");
        exit(1);
    }
    
    int status;
    waitpid(pid, &status, 0);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double elapsed = d(start, end);
    
    printf("Child PID: %d\n", pid);
    
    if (WIFEXITED(status)) {
        printf("Exit code: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Killed by signal: %d\n", WTERMSIG(status));
    }
    
    printf("Elapsed time: %.3f seconds\n", elapsed);
    
    return 0;
}
