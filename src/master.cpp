#include <sys/types.h>
#include <sys/socket.h>

#include "server.hpp"
#include "helpers.hpp"

void* master_routine(void *arguments) {
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
        if (pthread_create(&(worker_threads[i]), nullptr, worker_routine, (void*)&worker_args) != 0) {
            perror("pthread_create on master");
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
            if (errno == EINTR) {
                if (sigint_received) {
                    pthread_cond_broadcast(&(worker_args.sync_units->buffer_nonempty));
                    pthread_mutex_unlock(&(worker_args.sync_units->buffer_mutex));
                    break;
                }
            }

            perror("accept");
            exit(EXIT_FAILURE);
        }

        pthread_mutex_lock(&(worker_args.sync_units->buffer_mutex));
        while (args.buffer->size() >= args.buffer_size) {
            pthread_cond_wait(&(worker_args.sync_units->buffer_nonfull), &(worker_args.sync_units->buffer_mutex));
        }

        args.buffer->push(sock);

        pthread_cond_signal(&(worker_args.sync_units->buffer_nonempty));
        pthread_mutex_unlock(&(worker_args.sync_units->buffer_mutex));

    }

    for (int i = 0; i < args.num_workers_threads; i++) {
        int status;
        if (pthread_join(worker_threads[i], (void **)&status) != 0) {
            perror("pthread join  on master");
            exit(EXIT_FAILURE);
        }
    }

    pthread_mutex_destroy(&(worker_args.sync_units->buffer_mutex));
    pthread_mutex_destroy(&(worker_args.sync_units->log_mutex));

    pthread_cond_destroy(&(worker_args.sync_units->buffer_nonempty));
    pthread_cond_destroy(&(worker_args.sync_units->buffer_nonfull));

    delete worker_args.sync_units;

    pthread_exit(NULL);
}