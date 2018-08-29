#include <stdio.h>
#include <stdlib.h>


int check_permissions(char *permission_field, char *required_permissions) {
    for (int i = 0; i < 9; i++) {
        if (permission_field[i+1] == '-' && required_permissions[i] != '-') {
            return 1;
        }
    }
    return 0;
}


int main(int argc, char** argv) {
    char line[31];
    int size;

    int count = 0;
    int larger = 0;

    if (!(argc == 2 || argc == 3)) {
        fprintf(stderr, "USAGE: count_large size [permissions]\n");
        return 1;
    } else if (argc == 2) {
        size = strtol(argv[1], NULL, 10);
        while (scanf("%s", line) != EOF) {
            if ((count-2) % 9 == 4) {
                if (strtol(line, NULL, 10) > size) {
                    larger += 1;
                }
            }
            count += 1;
        }
        printf("%d\n", larger);
    } else {
        int good_permissions = 0;
        size = strtol(argv[1], NULL, 10);
        while (scanf("%s", line) != EOF) {
            if ((count-2) % 9 == 0 && check_permissions(line, argv[2]) == 0) {
                good_permissions = 1;
            } else if ((count-2) % 9 == 4) {
                if (strtol(line, NULL, 10) > size && good_permissions == 1) {
                    larger += 1;
                    good_permissions = 0;
                }
            }
            count += 1;
        }
        printf("%d\n", larger);
    }

    return 0;
}
