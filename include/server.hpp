#ifndef SERVER_H
#define SERVER_H

#include <queue>
#include <map>
#include <pthread.h>

#define BACKLOG 20

typedef struct {
    pthread_mutex_t buffer_mutex;   // Mutex that protects the buffer of connections
    pthread_mutex_t log_mutex;      // Mutex that protects the access to shared data

    pthread_cond_t buffer_nonfull;     // Condition in which the buffer is full
    pthread_cond_t buffer_nonempty;// Condition in which the buffer is non-full
} SyncUnits;

typedef struct master_args {
    std::queue<int> *buffer;
    size_t buffer_size;
    int num_workers_threads;
    
    int log_fd, stats_fd;

    std::map<std::string, std::string> *votes;
    unsigned int *voters;

    int sock;   // File descriptor for socket working on
} MasterArgs;

typedef struct worker_args {
    std::queue<int> *buffer;
    size_t buffer_size;

    int log_fd, stats_fd;

    std::map<std::string, std::string> *votes;
    unsigned int *voters;
    
    SyncUnits *sync_units;

} WorkerArgs;


void* master_routine(void *arguments);
void* worker_routine(void *arguments);

#endif // SERVER_H