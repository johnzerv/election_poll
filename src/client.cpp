#include <iostream>
#include <cassert>
#include <cstring>
#include <fcntl.h>

#include <stdio.h>
#include <sys/types.h> /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <unistd.h> /* read , write , close */
#include <netdb.h> /* g e t h o s t b y a d d r */
#include <stdlib.h> /* exit */
#include <string.h> /* strlen */

#include "helpers.hpp"
#include "client.hpp"

using namespace std;

bool sigint_received = false;

int main(int argc, char *argv[]) {
    assert(argc == 4);  // Number of command line arguments must be 4

    string server_name = argv[1];
    int port = stoi(argv[2]);
    string input_file_name = argv[3];

    int num_lines = get_input_lines(input_file_name.c_str());
    pthread_t voters[num_lines];
    VoterArgs voter_args[num_lines];

    int input_fd = open(input_file_name.c_str(), O_RDONLY, 0666);

    for (int i = 0; i < num_lines; i++) {
        voter_args[i].server_name = server_name;
        voter_args[i].port = port;

        // Read the current line from the file
        char *vote = new char[200];
        read_line_from_fd(input_fd, vote);

        // Extract the firstname of voter
        char *voter_firstname = new char[100];
        if ((voter_firstname = strtok(vote, " ")) == nullptr) {
            perror("Wrong format of input file");
            exit(EXIT_FAILURE);
        }
        voter_args[i].firstname = string(voter_firstname);

        // Extract the lastname of voter
        char *voter_lastname = new char[100];
        if ((voter_lastname = strtok(nullptr, " ")) == nullptr) {
            perror("Wrong format of input file");
            exit(EXIT_FAILURE);
        }
        voter_args[i].lastname = string(voter_lastname);

        // Extract the party voter voted
        char *party = new char [100];
        if ((party = strtok(nullptr, " ")) == nullptr) {
            perror("Wrong format of input file");
            exit(EXIT_FAILURE);
        }
        voter_args[i].party = string(party);

        // Create a voter-thread that votes the party he/she wants
        if (pthread_create(&(voters[i]), nullptr, voter_routine, (void*)&(voter_args[i])) != 0) {
            perror("pthread_create on client");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_lines; i++) {
        int status;
        if (pthread_join(voters[i], (void **)&status) != 0) {
            perror("pthread join on clinet");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

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

    cout << "Connecting to " << args->server_name << " port " << port << endl;
    
    string voter_name = args->firstname + " " + args->lastname + "\n";

    char *msg_buffer = new char[sizeof(char) * MAX_MSG_LENGTH];

    read_line_from_fd(sock, msg_buffer);
    if (strcmp(msg_buffer, "SEND NAME PLEASE")) {
        perror("Wrong message from server");
        exit(EXIT_FAILURE);
    }

    write_safely(sock, (void*)voter_name.c_str(), sizeof(char) * (strlen(voter_name.c_str())));

    read_line_from_fd(sock, msg_buffer);

    if (strcmp(msg_buffer, "ALREADY VOTED")) {
        if (strcmp(msg_buffer, "SEND VOTE PLEASE")) {
            perror("Wrong message from server");
            exit(EXIT_FAILURE);
        }

        string party = args->party + "\n";
        write_safely(sock, (void*)party.c_str(), sizeof(char) * (strlen(party.c_str())));
    }

    delete(msg_buffer);

    pthread_exit(nullptr);
}