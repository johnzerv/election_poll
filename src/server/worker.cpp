#include <cstring>

#include "server.hpp"
#include "helpers.hpp"


void* worker_routine(void *arguments) {
    block_sigint();
    
    WorkerArgs *args = (WorkerArgs *)arguments;
    
    while (1) {
        // Check if the buffer is empty
        pthread_mutex_lock(&(args->sync_units->buffer_mutex));

        while(args->buffer->size() <= 0 && !sigint_received) {
            pthread_cond_wait(&(args->sync_units->buffer_nonempty), &(args->sync_units->buffer_mutex)); // If it is, block
        }

        if (sigint_received) {  // If SIGINT received, break the loop in order to exit
            pthread_mutex_unlock(&(args->sync_units->buffer_mutex));
            break;
        }

        int sock_fd = args->buffer->front();   // File descriptor returned by accept
        args->buffer->pop();

        pthread_cond_signal(&(args->sync_units->buffer_nonfull));
        pthread_mutex_unlock(&(args->sync_units->buffer_mutex));
        
        write_safely(sock_fd, "SEND NAME PLEASE\n", sizeof(char) * strlen("SEND NAME PLEASE\n"));

        char *voter = new char[MAX_VOTER_NAME_LENGTH];
        read_line_from_fd(sock_fd, voter);

        pthread_mutex_lock(&(args->sync_units->log_mutex)); // Lock log_mutex because shared data will be converted

        // If voter has already voted, send the appropriate message and close the connection gracefully
        if (args->voters_to_votes->find(std::string(voter)) != args->voters_to_votes->end()) {
            write_safely(sock_fd, "ALREADY VOTED\n", sizeof(char) * strlen("ALREADY VOTED\n"));
            
            delete(voter);
            if (shutdown(sock_fd, SHUT_RDWR) < 0) {
                perror("shutdown");
                exit(EXIT_FAILURE);
            }
            close(sock_fd);
        }
        else {
            // Ask client for vote
            write_safely(sock_fd, "SEND VOTE PLEASE\n", sizeof(char) * strlen("SEND VOTE PLEASE\n"));

            char *party = new char[MAX_PARTY_NAME_LENGTH];
            read_line_from_fd(sock_fd, party);

            std::string space = " ";
            std::string voter_and_party = voter + space + party + "\n";

            // Update poll-log file
            write_safely(args->log_fd, voter_and_party.c_str(), strlen(voter_and_party.c_str())  * sizeof(char));   // Update poll-log file
            (*(args->voters_to_votes))[voter] = party;
            (*(args->parties_to_votes))[party]++;

            // Send the voter's name and the vote to client
            write_safely(sock_fd, "VOTE for Party ", sizeof(char) * strlen("VOTE for Party "));
            write_safely(sock_fd, party, strlen(party) * sizeof(char));
            write_safely(sock_fd, " RECORDED\n", sizeof(char) * strlen(" RECORDED\n"));

            delete(voter);
            delete(party);

            if (shutdown(sock_fd, SHUT_RDWR) < 0) {
                perror("shutdown");
                exit(EXIT_FAILURE);
            }
            close(sock_fd);
        }
        pthread_mutex_unlock(&(args->sync_units->log_mutex));
    }

    pthread_exit(NULL);
}