#include <stdio.h>

#include "library.h"

int main() {
    printf("Mifare Nested:\n");
    char *test = run_full_nested(0x9a22bf95, 0xcba8a6e2, 0x652e32b2, 0x08ad, 0x430059a8, 0xd43444b6, 0x08ad, 700, 900);
    printf("%s\n", test);
    //run_nested(0x7a075780, 0x85ed380c, 0x71851edf, 0xc94a7473, 0xd7b7fdc2);
    return 0;
}
