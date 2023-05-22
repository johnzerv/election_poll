#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <iostream>
#include <unistd.h>

extern bool sigint_received;

void read_line_from_fd(int fd, char *str);  // Reads until '\r' or '\n' from a file descriptor

void set_sigint_handler();
void block_sigint();
void unblock_sigint();
void sigint_handler(int signo);

#endif // HELPERS_H