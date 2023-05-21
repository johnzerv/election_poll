#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <iostream>
#include <unistd.h>

void read_line_from_fd(int fd, char *str);  // Reads until '\r' or '\n' from a file descriptor

#endif // HELPERS_H