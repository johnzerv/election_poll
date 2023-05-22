#include <csignal>

#include "helpers.hpp"

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

void set_sigint_handler() {
    struct sigaction act_sigint;

    act_sigint.sa_handler = sigint_handler;

    sigemptyset(&act_sigint.sa_mask);
    act_sigint.sa_flags = 0;

    sigaction(SIGINT, &act_sigint, NULL);
}

void sigint_handler(int signo) {
    sigint_received = true;
}

void block_sigint() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

void unblock_sigint() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
}