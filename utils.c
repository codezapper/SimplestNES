short check_bit(int number, int bit) {
    if (number & (1 << bit)) {
        return 1;
    }

    return 0;
}

unsigned char set_bit(unsigned char value, int bit) {
    return value | (1 << bit);
}

unsigned char clear_bit(unsigned char value, int bit) {
    return value | (1 << bit);
}

