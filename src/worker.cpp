#include <cstring>

#include "server.hpp"
#include "helpers.hpp"


void* worker_routine(void *arguments) {
    WorkerArgs *args = (WorkerArgs *)arguments;
    
    while (1) {
        pthread_mutex_lock(&(args->sync_units->buffer_mutex));

        while(args->buffer->size() <= 0 && !sigint_received) {
            pthread_cond_wait(&(args->sync_units->buffer_nonempty), &(args->sync_units->buffer_mutex));
        }

        if (sigint_received) {
            break;
        }

        int sock_fd = args->buffer->front();   // File descriptor returned by accept
        args->buffer->pop();

        pthread_cond_signal(&(args->sync_units->buffer_nonfull));
        pthread_mutex_unlock(&(args->sync_units->buffer_mutex));
        
        write(sock_fd, "SEND NAME PLEASE\n", sizeof("SEND NAME PLEASE\n")); // TODO : write_safely
        char *voter = new char[100];
        read_line_from_fd(sock_fd, voter);

        pthread_mutex_lock(&(args->sync_units->log_mutex));
        if (args->voters_to_votes->find(std::string(voter)) != args->voters_to_votes->end()) {
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
            (*(args->voters_to_votes))[std::string(voter)] = std::string(party);                      // Update voters info-stats
            (*(args->parties_to_votes))[std::string(party)]++;

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