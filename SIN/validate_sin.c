#include <stdio.h>
#include <stdlib.h>

int populate_array(int, int *);
int check_sin(int *);


int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }

    int* sin_array = malloc(sizeof(int) * 9);
    populate_array(strtol(argv[1], NULL, 10), sin_array);

    int check = check_sin(sin_array);
    if (check == 0) {
        printf("Valid SIN\n");
    } else {
        printf("Invalid SIN\n");
    }
    free(sin_array);
  
    return 0;
}
