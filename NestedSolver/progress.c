#include <stdio.h>
#include <stdlib.h>
#include "progress.h"
#define PERCENTAGE(V, T) (100 - (((T - V) * 100) / T))
#ifdef _WIN32
    #include <Windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

void print_progress(uint32_t count, uint32_t max) {
    uint32_t i = 0;
    const char prefix[] = "Key recovery: [";
    const char suffix[] = "]";
    const size_t prefix_length = sizeof(prefix) - 1;
    const size_t suffix_length = sizeof(suffix) - 1;
    char *buffer;
    size_t bar_width, terminal_width;
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    terminal_width = csbi.dwSize.X;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    terminal_width = w.ws_col;
#endif

    if (terminal_width < prefix_length + suffix_length + 8) {
        printf("\r[%u/%u]", count, max);
        return;
    }

    bar_width = terminal_width - prefix_length - suffix_length - 8;
    buffer = calloc(bar_width + 1, 1);

    for (; i < bar_width; ++i) {
        buffer[i] = i < PERCENTAGE(count, max) * bar_width / 100 ? '#' : ' ';
    }

    printf("\r%s%s%s (%u%%)", prefix, buffer, suffix, PERCENTAGE(count, max));
    fflush(stdout);

    free(buffer);
}

void exit_progress() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    DWORD written;
    COORD cursor = {0, csbi.dwCursorPosition.Y};
    FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ', csbi.dwSize.X, cursor, &written);
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor);
#else
    printf("\r\033[2K");
#endif
    fflush(stdout);
}