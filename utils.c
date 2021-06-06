short check_bit(int number, int bit) {
    if (number & (1 << bit)) {
        return 1;
    }

    return 0;
}

char set_bit(char value, int bit) {
    return value | (1 << bit);
}

char clear_bit(char value, int bit) {
    return value | (1 << bit);
}

