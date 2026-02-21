// select.c: Single-option terminal selector
#include "select.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>

void enable_raw_mode(struct termios *orig) {
    tcgetattr(STDIN_FILENO, orig);
    struct termios raw = *orig;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode(struct termios *orig) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig);
}

int summon_select(const char *options[]) {
    int option_count = 0;
    while (options[option_count] != NULL) option_count++;
    struct termios orig;
    enable_raw_mode(&orig);
    int cursor = 0;
    while (1) {
        printf("\033[2J\033[H"); // clear screen
        printf("Use ↑/↓ to move, enter to select\n\n");
        for (int i = 0; i < option_count; i++) {
            printf("%s %s\n", (i == cursor ? ">" : " "), options[i]);
        }
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == '\x1b') {
                char seq[2];
                read(STDIN_FILENO, &seq[0], 1);
                read(STDIN_FILENO, &seq[1], 1);
                if (seq[0] == '[') {
                    if (seq[1] == 'A' && cursor > 0)
                        cursor--;
                    else if (seq[1] == 'B' && cursor < option_count - 1)
                        cursor++;
                }
            } else if (c == '\n') {
                break;
            }
        }
    }
    disable_raw_mode(&orig);
    return cursor;
}
