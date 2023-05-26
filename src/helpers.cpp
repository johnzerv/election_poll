#include <csignal>
#include <cstring>
#include <arpa/inet.h>

#include "helpers.hpp"

// Returns total lines of a file
int get_input_lines(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
        
    int lines = 0;
    char ch;

    ch = getc(fp);
    while (ch != EOF) {
        lines++;

        while ((ch = getc(fp)) != '\n' && ch != EOF);
    }

    if (fclose(fp) != 0) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }

    return lines - 1;
}

void read_safely(int fd, const void *buf) {
    size_t length;
    read(fd, &length, sizeof(size_t));
    length = ntohl(length);    

    ssize_t n_bytes = 0;

    for (ssize_t n_read; (size_t)n_bytes < length; ) {
        if ((n_read = read(fd, (void*)buf+n_bytes, length-n_bytes)) < 0) {
            // std::cout << "\n\n ERRNO == " << errno << "\n\n";
            if (errno == EINTR) {   // Signal received before read anything
                continue;
            }
            
            perror("read");
            exit(EXIT_FAILURE);
        }
        else {
            n_bytes += n_read;
        }
    }
}

void write_safely(int fd, const void *buf, size_t count) {
    size_t length = htonl(count);
    write(fd, &length, sizeof(size_t));
    ssize_t n_bytes = 0;


    for (ssize_t n_written; (size_t)n_bytes < count; ) {
        if ((n_written = write(fd, (void*)buf+n_bytes, count-n_bytes)) < 0) {
            if (errno == EINTR) {       // Signal received before write anything
                continue;
            }
            
            perror("read");
            exit(EXIT_FAILURE);
        }
        else {
            n_bytes += n_written;
        }
    }
}

void read_line_from_fd(int fd, char *str) {
    char ch[1];
    int str_index = 0;

    // Read byte per byte the input until '\r' or '\n'
    do {
        read(fd, ch, 1);
        str[str_index++] = ch[0];

        if ((ch[0] == '\n' || ch[0] == '\r') && str_index == 1) {   // Skip new line at beginning
            str_index--;
            ch[0] = -1;
        }
    } while (ch[0] != '\r' && ch[0] != '\n');

    // Special handling on return carriage character
    if (ch[0] == '\r') {  
        read(fd, ch, 1); // Just consume '\n'
        str_index--;            // Do not count '\r'
    }

    str[str_index-1] = '\0';      // Replace '\n' with '\0'
}

void sigint_handler(int signo) {
    sigint_received = true;
}

void set_sigint_handler() {
    struct sigaction act_sigint;

    act_sigint.sa_handler = sigint_handler;

    sigemptyset(&act_sigint.sa_mask);
    act_sigint.sa_flags = 0;

    sigaction(SIGINT, &act_sigint, NULL);
}

// Block SIGINT (ctrl-C) signal from a thread (and every thread current thread will create)
void block_sigint() {
    sigset_t mask;
    sigemptyset(&mask);         // Clear the mask
    sigaddset(&mask, SIGINT);   // Add only SIGINT which we want to block

    pthread_sigmask(SIG_BLOCK, &mask, NULL);    // Block it
}

// Unblock SIGINT (ctrl-C) signal from a thread likewise above block function
void unblock_sigint() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
}