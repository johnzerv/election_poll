#include "../include/helpers.h"

// TODO : read safely
void read_line_from_fd(int fd, char *str) {
    char ch[1];
    int str_index = 0;

    do {
        if (read(fd, ch, 1) < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        str[str_index++] = ch[0];
    } while (ch[0] != '\r' && ch[0] != '\n');

    if (ch[0] == '\r') {  
        if (read(fd, ch, 1) < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        str_index--;
    }

    str[str_index] = '\0';
}