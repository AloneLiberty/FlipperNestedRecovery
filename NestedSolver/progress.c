#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "progress.h"
#define PERCENTAGE(V, T) (100 - (((T - V) * 100) / T))

void print_progress(uint32_t count, uint32_t max) {
    uint32_t i = 0;
    const char prefix[] = "Key recovery: [";
    const char suffix[] = "]";
    const size_t prefix_length = sizeof(prefix) - 1;
    const size_t suffix_length = sizeof(suffix) - 1;
    char *buffer = calloc(100 + prefix_length + suffix_length + 1, 1);

    strcpy(buffer, prefix);
    for (; i < 100; ++i) {
        buffer[prefix_length + i] = i < PERCENTAGE(count, max) ? '#' : ' ';
    }

    strcpy(&buffer[prefix_length + i], suffix);
    printf("\b\r%c[2K\r%s (%u%%)", 27, buffer, PERCENTAGE(count, max));
    fflush(stdout);
    free(buffer);
}

void exit_progress() {
    printf("\33[2K\r");
    fflush(stdout);
}