#ifndef CLIENT_H
#define CLIENT_H

#include <string>

#define MAX_MSG_LENGTH 200

typedef struct voter_args {
    std::string firstname;
    std::string lastname;
    std::string party;

    std::string server_name;
    int port;
} VoterArgs;

void* voter_routine(void *arguments);

#endif // CLIENT_H