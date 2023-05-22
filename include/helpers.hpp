#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <iostream>
#include <unistd.h>

extern bool sigint_received;

void read_line_from_fd(int fd, char *str);  // Reads until '\r' or '\n' from a file descriptor

void read_safely(int fd, void *buf, size_t count);
void write_safely(int fd, void *buf, size_t count);

void sigint_handler(int signo);
void set_sigint_handler();  // Setup handler for SIGINT signal
void block_sigint();        // Blocks signal SIGINT from a thread
void unblock_sigint();      // Unblocks signal SIGINT from a thread

#endif // HELPERS_H