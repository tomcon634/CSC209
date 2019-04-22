#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int i;
    int iterations;
    int old_pid = 0;
    int new_pid = getpid();

    if (argc != 2) {
        fprintf(stderr, "Usage: childcreates <iterations>\n");
        exit(1);
    }

    iterations = strtol(argv[1], NULL, 10);

    for (i = 0; i < iterations; i++) {
        if (old_pid < new_pid) {
            int n = fork();
            if (n < 0) {
                perror("fork");
                exit(1);
            }
            old_pid = new_pid;
            new_pid = getpid();
            printf("ppid = %d, pid = %d, i = %d\n", getppid(), getpid(), i);
        }
    }

    return 0;
}
