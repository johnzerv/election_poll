#include <iostream>
#include <string>
#include <cassert>
#include <queue>
#include <map>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h> 
#include <cstring>

#include "server.hpp"
#include "helpers.h"

using namespace std;

void sigint_handler(int signo) {
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    assert(argc == 6);      // Command line arguments must be exactly six

    int numWorkerThreads = stoi(argv[2]);
    int bufferSize = stoi(argv[3]);
    assert(numWorkerThreads > 0);    // numWorkerthreads must be greater than zero
    assert(bufferSize > 0);         // bufferSize must be greater than zero too

    char *poll_log_name = new char[100], *poll_stats_name = new char[100];
    strcpy(poll_log_name, argv[4]);
    strcpy(poll_stats_name, argv[5]);

    int log_fd = open((string(RESULT_DIR + string(poll_log_name)).c_str()), O_RDWR | O_CREAT | O_TRUNC, 0666);
    int stats_fd = open((string(RESULT_DIR) + string(poll_stats_name)).c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);

    // Statistics
    map<string,string> *voters_to_votes = new map<string, string>();
    map<string, int> *parties_to_votes = new map<string, int>();
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
    
    // Create master thread
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
        perror("pthread_create on master");
        exit(EXIT_FAILURE);
    }

    // Wait for master thread
    int status;
    if (pthread_join(master_thread, (void **)&status) != 0) {
        perror("pthread_join on master");
        exit(EXIT_FAILURE);
    }
    
    map <string, int>::iterator it;
    for (it = parties_to_votes->begin(); it != parties_to_votes->end(); it++) {
        string tmp_str = it->first + " " + to_string(it->second);

        cout << tmp_str << endl;
    }


    close(sock);
    close(log_fd);
    close(stats_fd);

    delete(master_args.buffer);
    delete(voters_to_votes);
    delete(parties_to_votes);
    delete poll_log_name;
    delete poll_stats_name;

    return 0;
}

void* master_routine(void *arguments) {
    signal(SIGINT, sigint_handler);
    MasterArgs args = *(MasterArgs *)arguments;

    WorkerArgs worker_args;
    worker_args.buffer = args.buffer;
    worker_args.buffer_size = args.buffer_size;
    worker_args.voters_to_votes = args.voters_to_votes;
    worker_args.parties_to_votes = args.parties_to_votes;
    worker_args.log_fd = args.log_fd;
    worker_args.stats_fd = args.stats_fd;

    worker_args.sync_units = new SyncUnits;
    
    pthread_mutex_init(&(worker_args.sync_units->buffer_mutex), nullptr);
    pthread_mutex_init(&(worker_args.sync_units->log_mutex), nullptr);

    pthread_cond_init(&(worker_args.sync_units->buffer_nonfull), nullptr);
    pthread_cond_init(&(worker_args.sync_units->buffer_nonempty), nullptr);

    // Create worker threads
    pthread_t worker_threads[args.num_workers_threads];

    for (int i = 0; i < args.num_workers_threads; i++) {
        if (pthread_create(&worker_threads[i], nullptr, worker_routine, (void*)&worker_args) != 0) {
            perror("pthread_create on worker");
            exit(EXIT_FAILURE);
        }
    }

    // Loop for accepting connections from server
    int sock;
    struct sockaddr_in client;
    socklen_t client_len;

    while(1) {
        // Accept connection
        if ((sock = accept(args.sock, (struct sockaddr *)&client, &client_len)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        cout << "Accepted connection" << endl;

        pthread_mutex_lock(&(worker_args.sync_units->buffer_mutex));
        while (args.buffer->size() >= args.buffer_size) {
            cout << "PROFANWS MPIKA EDW " << endl;
            pthread_cond_wait(&(worker_args.sync_units->buffer_nonfull), &(worker_args.sync_units->buffer_mutex));
        }


        args.buffer->push(sock);

        pthread_cond_signal(&(worker_args.sync_units->buffer_nonempty));
        pthread_mutex_unlock(&(worker_args.sync_units->buffer_mutex));

    }

    for (int i = 0; i < args.num_workers_threads; i++) {
        int status;
        if (pthread_join(worker_threads[i], (void **)&status) != 0) {
            perror("pthread join  on worker threads");
            exit(EXIT_FAILURE);
        }
    }
    pthread_exit(NULL);
}

void* worker_routine(void *arguments) {
    WorkerArgs *args = (WorkerArgs *)arguments;
    
    while (1) {
        pthread_mutex_lock(&(args->sync_units->buffer_mutex));

        while(args->buffer->size() <= 0) {
            pthread_cond_wait(&(args->sync_units->buffer_nonempty), &(args->sync_units->buffer_mutex));
        }

        int sock_fd = args->buffer->front();   // File descriptor returned by accept
        args->buffer->pop();

        pthread_cond_signal(&(args->sync_units->buffer_nonfull));
        pthread_mutex_unlock(&(args->sync_units->buffer_mutex));
        
        write(sock_fd, "SEND NAME PLEASE\n", sizeof("SEND NAME PLEASE\n")); // TODO : write_safely
        char *voter = new char[100];
        read_line_from_fd(sock_fd, voter);

        pthread_mutex_lock(&(args->sync_units->log_mutex));
        if (args->voters_to_votes->find(string(voter)) != args->voters_to_votes->end()) {
            write(sock_fd, "ALREADY VOTED\n", sizeof("ALREADY VOTED\n"));
            
            delete(voter);
            close(sock_fd);
        }
        else {
            write(sock_fd, "SEND VOTE PLEASE\n", sizeof("SEND VOTE PLEASE\n"));

            char *party = new char[100];
            read_line_from_fd(sock_fd, party);

            char *voter_and_party = new char[sizeof(((strlen(voter) + strlen(party)) * sizeof(char)) + 3 * sizeof(char))];    // 3 for : ' ', '\n' and '\0'
            strcpy(voter_and_party, voter);
            strcat(voter_and_party, " ");
            strcat(voter_and_party, party);
            strcat(voter_and_party, "\n");

            // Update poll-log file
            write(args->log_fd, voter_and_party, strlen(voter_and_party) * sizeof(char));   // Update poll-log file
            (*(args->voters_to_votes))[string(voter)] = string(party);                      // Update voters info-stats
            (*(args->parties_to_votes))[string(party)]++;

            write(sock_fd, "VOTE for Party ", sizeof("VOTE for Party "));
            write(sock_fd, party, strlen(party) * sizeof(char));
            write(sock_fd, " RECORDED\n", sizeof(" RECORDED\n"));

            delete(voter);
            delete(party);
            delete(voter_and_party);
            close(sock_fd);
        }
        pthread_mutex_unlock(&(args->sync_units->log_mutex));
    }

    pthread_exit(NULL);
}