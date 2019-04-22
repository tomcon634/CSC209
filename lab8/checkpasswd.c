#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 10

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
  char user_id[MAXLINE];
  char password[MAXLINE];

  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }

  int fd[2];
  pipe(fd);

  int r = fork();

  if (r < 0) {
    perror("fork");
    exit(1);
  } else if (r > 0) { // parent
    close(fd[1]);
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);

    /*int val;
    read(STDIN_FILENO, &val, MAX_PASSWORD);
    if (val == 0) {
      printf(SUCCESS);
    } else if (val == 2) {
      printf(INVALID);
    } else if (val == 3) {
      printf(NO_USER);
    } else {
      perror("validate");
      exit(1);
    }*/
  } else { // child
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);
    close(fd[1]);
    execl("./validate", "validate", NULL);
  }

  int val = 0;
  //read(STDIN_FILENO, &val, sizeof(int));  
  if (val == 0) {
    printf(SUCCESS);
  } else if (val == 2) {
    printf(INVALID);
  } else if (val == 3) {
    printf(NO_USER);
  } else {
    perror("validate");
    exit(1);
  }


  return 0;
}
