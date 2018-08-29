
int populate_array(int sin, int *sin_array) {
    if (sin < 100000000) {
        return 1;
    }

    for (int i = 0; i < 9; i++) {
        int count = 1;
        for (int j = 0; j < i; j++) {
            count *= 10;
        }
        sin_array[8-i] = (sin/count) % 10;
    }
    return 0;
}

int check_sin(int *sin_array) {
    for (int i = 0; i < 9; i++) {
        if (i % 2 == 0) {
            sin_array[i] *= 1;
        } else {
            sin_array[i] *= 2;
        }
    }

    for (int i = 0; i < 9; i++) {
        if (sin_array[i] > 9) {
            sin_array[i] = sin_array[i] % 10 + sin_array[i] / 10;
        }
    }

    int sum = 0;
    for (int i = 0; i < 9; i++) {
        sum += sin_array[i];
    }

    if (sum % 10 == 0) {
        return 0;
    }
    return 1;
}
