#include <cstring>
#include <fcntl.h>  // open syscall
#include <sys/types.h>  // sockets
#include <sys/socket.h> // sockets 
#include <netinet/in.h> // internet sockets 
#include <unistd.h>     // read, write, close
#include <netdb.h>      // gethostbyaddr
#include <stdlib.h>

#include "helpers.hpp"
#include "client.hpp"

void* voter_routine(void *arguments) {
    VoterArgs *args = (VoterArgs *)arguments;

    int port, sock;
    struct sockaddr_in server;
    struct hostent *hostent;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if ((hostent = gethostbyname(args->server_name.c_str())) == nullptr) {
        perror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    port = args->port;
    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, hostent->h_addr, hostent->h_length);
    server.sin_port = htons(port);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("exit");
        exit(EXIT_FAILURE);
    }
    
    std::string voter_name = args->firstname + " " + args->lastname + "\n";

    char *msg_buffer = new char[sizeof(char) * MAX_MSG_LENGTH];

    read_line_from_fd(sock, msg_buffer);  // Server's message : SEND NAME PLEASE
    if (strcmp(msg_buffer, "SEND NAME PLEASE")) {
        perror("Wrong message from server");
        exit(EXIT_FAILURE);
    }

    write_safely(sock, (void*)voter_name.c_str(), sizeof(char) * (strlen(voter_name.c_str())));

    read_line_from_fd(sock, msg_buffer);  // Server's message : SEND VOTE PLEASE or ALREADY VOTED

    if (strcmp(msg_buffer, "ALREADY VOTED")) {
        if (strcmp(msg_buffer, "SEND VOTE PLEASE")) {
            perror("Wrong message from server");
            exit(EXIT_FAILURE);
        }

        std::string party = args->party + "\n";
        write_safely(sock, (void*)party.c_str(), sizeof(char) * (strlen(party.c_str())));

        read_line_from_fd(sock, msg_buffer);  // Server's last message
    }

    delete(msg_buffer);

    if (shutdown(sock, SHUT_RDWR) < 0) {
        perror("shutdown");
        exit(EXIT_FAILURE);
    }
    close(sock);

    pthread_exit(nullptr);
}