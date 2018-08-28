#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include "filter.h"

//wrapper funtions
void Pipe(int *fd) {
  if (pipe(fd) == -1) {
    perror("pipe");
    exit(1);
  }
}

void Dup2(int old_fd, int new_fd) {
  if (dup2(old_fd, new_fd) == -1) {
    perror("dup2");
    exit(1);
  }
}

void Close(int fd) {
  if (close(fd) == -1) {
    perror("close");
    exit(1);
  }
}

int main(int argc, char** argv) {
  char* endptr;
  int n = strtol(argv[1], &endptr, 10);
  if (argc != 2 || n <= 0 || strlen(endptr) != 0) {
    fprintf(stderr, "Usage:\n\tpfact n\n");
    exit(1);
  }
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    perror("signal");
    exit(1);
  }

  int fd[n][2];
  int old_pid = 0;
  int new_pid = getpid();

  int num;
  int original_list[n-1];
  int filtered_list[n-1];

  int factors[n-1];
  int is_prime = 1;
  for (int i = 0; i < n-1; i++) {
    factors[i] = 0;
  }

  int num_filters = 0;

  int num_factors = 0;
  int factor1 = 0, factor2 = 0;

  int k = n-1;

  int i = 2;
  int j;

  if (n < 5) { //while loop won't run
    if (n == 4) {
      printf("4 2 2\n");
      printf("Number of filters = 0\n");
    } else {
      printf("%d is prime\n", n);
      printf("Number of filters = 0\n");
    }
  }
  while (i < sqrt(n)) {
    if (old_pid < new_pid) {
      Pipe(fd[i]);
      int r = fork();

      if (r < 0) {
        perror("fork");
        exit(1);

      } else if (r > 0) { //parent
        Close(fd[i][0]);
        Dup2(fd[i][1], STDOUT_FILENO);
        Close(fd[i][1]);
        if (i == 2) {
          for (j = 2; j <= n; j++) {
            printf("%d\n", j);
          }
        } else {
          while (filtered_list[0] != i) {
            i++;
          }
          for (j = 0; j < n-1; j++) {
            if (j < k) {
              printf("%d\n", filtered_list[j]);
            } else {
              printf("0\n");
            }
          }
        }

      } else { //child
        Close(fd[i][1]);
        Dup2(fd[i][0], STDIN_FILENO);
        Close(fd[i][0]);
        for (j = 0; j < n-1; j++) {
          scanf("%d", &num);
          original_list[j] = num;
        }
        k = 0;
        int is_filter = 0;
        for (j = 0; j < n-1; j++) {
          if (original_list[j] % i != 0) {
            filtered_list[k] = original_list[j];
            k++;
          } else if (original_list[j] % i == 0 && original_list[j] != 0) {
            is_filter = 1;
          }
        }
        if (is_filter == 1) {
          if (n % i == 0) {
            factors[i] = i;
            num_factors++;
          }
          if (num_factors < 2) {
            num_filters++;
          }
        }
        int root = floor(sqrt(n));
        if (i == root || i == sqrt(n) - 1) {
          for (j = 0; j < n-1; j++) {
            if (factors[j] != 0) {
              is_prime = 0;
            }
          }
          if (is_prime == 0 || n / sqrt(n) == root) {
            for (j = 0; j < n-1; j++) {
              if (factors[j] != 0 && factor1 == 0 && factor2 == 0) {
                factor1 = factors[j];
              } else if (factors[j] != 0 && factor1 != 0 && factor2 == 0) {
                factor2 = factors[j];
              }
            }
            if (factor1 != 0 && factor2 != 0) {
              printf("%d is not the product of two primes\n", n);
            } else {
              for (j = 0; j < n-1; j++) {
                if (original_list[j] != 0 && n % original_list[j] == 0) {
                  factors[root+j] = original_list[j];
                }
              }
              int product = 1;
              for (j = 0; j < n-1; j++) {
                for (int l = 0; l < n-1; l++) {
                  if (factors[j] * factors[l] == n) {
                    product = 1;
                    factor1 = factors[l];
                    factor2 = factors[j];
                  }
                }
              }
              if (product == 1 && factor1 * factor2 == n && (factor2 % factor1 != 0 || factor2 == factor1)) {
                printf("%d %d %d\n", n, factor1, factor2);
              } else {
                printf("%d is not the product of two primes\n", n);
              }
            }
          } else {
            printf("%d is prime\n", n);
          }
          printf("Number of filters = %d\n", num_filters);
        }
      }

      old_pid = new_pid;
      new_pid = getpid();
    }

    i++;
  }

  return 0;
}
