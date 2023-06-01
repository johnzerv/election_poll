#include <cassert>
#include <cstring>
#include <fcntl.h>      // open syscall
#include <sys/types.h>  // sockets
#include <sys/socket.h> // sockets 
#include <netinet/in.h> // internet sockets 
#include <unistd.h>     // read, write, close syscalls
#include <netdb.h>      // gethostbyaddr
#include <stdlib.h>

#include "helpers.hpp"
#include "client.hpp"

using namespace std;

bool sigint_received = false;   // For sake of compilation, reduntant in client

int main(int argc, char *argv[]) {
    assert(argc == 4);  // Number of command line arguments must be 4

    // Command line args
    string server_name = argv[1];
    int port = stoi(argv[2]);
    string input_file_name = argv[3];

    int num_lines = get_input_lines(input_file_name.c_str());   // Number of total lines of given file

    // Pthread's info
    pthread_t voters[num_lines];
    bool actual_votes[num_lines];   // Bool array that specifies if the line was just the '\n' character or an actual vote
    VoterArgs voter_args[num_lines];

    // Assuming that all lines are not empty
    for (int i = 0; i < num_lines; i++) {
        actual_votes[i] = true;
    }

    int input_fd = open(input_file_name.c_str(), O_RDONLY, 0666);

    // For each line in given file, create a struct of arguments and create a thread-voter to handle the vote-line
    for (int i = 0; i < num_lines; i++) {
        voter_args[i].server_name = server_name;
        voter_args[i].port = port;

        // Read the current line from the file
        char *vote_tmp = new char[sizeof(char) * MAX_MSG_LENGTH], *vote = vote_tmp; // Keep the initial pointer in order to delete it correctly afterwards
        read_line_from_fd(input_fd, vote);

        // If the line is empty, just skip the line and don't create thread (save also this info)
        if (!strcmp(vote, "")) {
            delete vote_tmp;
            actual_votes[i] = false;
            continue;
        }

        // Extract the firstname of voter and update voter's arguments
        char *voter_firstname;
        if ((voter_firstname = strtok(vote_tmp, " ")) == nullptr) {
            perror("Wrong format of input file");
            exit(EXIT_FAILURE);
        }
        voter_args[i].firstname = string(voter_firstname);

        // Extract the lastname of voter and update voter's arguments
        char *voter_lastname;
        if ((voter_lastname = strtok(nullptr, " ")) == nullptr) {
            perror("Wrong format of input file");
            exit(EXIT_FAILURE);
        }
        voter_args[i].lastname = string(voter_lastname);

        // Extract the party voter voted and update voter's arguments
        char *party = new char[sizeof(char) * MAX_MSG_LENGTH], *party_token;
        if ((party_token = strtok(nullptr, " ")) == nullptr) {
            perror("Wrong format of input file");
            exit(EXIT_FAILURE);
        }
        strcpy(party, party_token);
        
        // Consider that a party might have multiple words on his name
        while ((party_token = strtok(nullptr, " ")) != nullptr) {
            strcat(party, " ");
            strcat(party, party_token);
        }
        voter_args[i].party = string(party);

        // Create a voter-thread that votes the party he/she wants
        if (pthread_create(&(voters[i]), nullptr, voter_routine, (void*)&(voter_args[i])) != 0) {
            perror("pthread_create on client");
            exit(EXIT_FAILURE);
        }

        delete party;
        delete vote;
    }

    for (int i = 0; i < num_lines; i++) {
        if (actual_votes[i] == false) { // If the line in the given input file was empty, there is no thread that has been created for it
            continue;
        }

        void *status;
        if (pthread_join(voters[i], (void **)&status) != 0) {
            perror("pthread join on clinet");
            exit(EXIT_FAILURE);
        }
    }

    close(input_fd);
    return 0;
}