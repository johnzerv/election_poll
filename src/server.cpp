#include <iostream>
#include <cassert>
#include <stdlib.h> 

#include "server.hpp"
#include "helpers.hpp"

using namespace std;

bool sigint_received = false;

int main(int argc, char *argv[]) {
    assert(argc == 6);      // Command line arguments must be exactly six

    int numWorkerThreads = stoi(argv[2]);
    int bufferSize = stoi(argv[3]);
    assert(numWorkerThreads > 0);    // numWorkerthreads must be greater than zero
    assert(bufferSize > 0);         // bufferSize must be greater than zero too

    set_sigint_handler();           // Define SIGINT handler

    string poll_log_name = argv[4];
    string poll_stats_name = argv[5];

    int log_fd = open((string(RESULT_DIR) + poll_log_name).c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    int stats_fd = open((string(RESULT_DIR) + poll_stats_name).c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);

    // Statistics
    map<string,string> *voters_to_votes = new map<string, string>();
    map<string, int> *parties_to_votes = new map<string, int>();

    // Socket info
    int port = stoi(argv[1]), sock;
    struct sockaddr_in server;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    server.sin_family = AF_INET;    // Internet Domain
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    // Bind socket to address
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind socket");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(sock, BACKLOG) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    cout << "Listening for connections to port " << port << endl;
    
    // Create master thread and args
    pthread_t master_thread;
    MasterArgs master_args;
    master_args.buffer = new queue<int>();
    master_args.buffer_size = bufferSize;
    master_args.num_workers_threads = numWorkerThreads;
    master_args.voters_to_votes = voters_to_votes;
    master_args.parties_to_votes = parties_to_votes;
    master_args.sock = sock;
    master_args.log_fd = log_fd;
    master_args.stats_fd = stats_fd;

    if (pthread_create(&master_thread, nullptr, master_routine, (void*)&master_args) != 0) {
        perror("pthread_create on server");
        exit(EXIT_FAILURE);
    }

    block_sigint();      // Block SIGINT from getting handled by server (main) and worker threads (unblock in master thread)

    // Wait for master thread
    int status;
    if (pthread_join(master_thread, (void **)&status) != 0) {
        perror("pthread_join on server");
        exit(EXIT_FAILURE);
    }
    
    // Update poll statistics file
    map <string, int>::iterator it;
    int total_number_of_votes;
    for (it = parties_to_votes->begin(); it != parties_to_votes->end(); it++) {
        string tmp_str = it->first + " " + to_string(it->second) + "\n";

        write_safely(stats_fd, tmp_str.c_str(), sizeof(char) * strlen(tmp_str.c_str()), true);

        total_number_of_votes += it->second;
    }
    write_safely(stats_fd, to_string(total_number_of_votes).c_str(), sizeof(char) * (strlen(to_string(total_number_of_votes).c_str())), true);


    if (shutdown(sock, SHUT_RDWR) < 0) {
        perror("shutdown");
        exit(EXIT_FAILURE);
    }
    close(sock);
    close(log_fd);
    close(stats_fd);

    delete(master_args.buffer);
    delete(voters_to_votes);
    delete(parties_to_votes);

    return 0;
}