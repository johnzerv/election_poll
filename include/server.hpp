#ifndef SERVER_H
#define SERVER_H

#include <queue>
#include <map>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BACKLOG 20000
#define MAX_VOTER_NAME_LENGTH 100
#define MAX_PARTY_NAME_LENGTH 100
#define RESULT_DIR "results/"

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

    std::map<std::string, std::string> *voters_to_votes;
    std::map<std::string, int> *parties_to_votes;

    int sock;   // File descriptor for socket working on
} MasterArgs;

typedef struct worker_args {
    std::queue<int> *buffer;
    size_t buffer_size;

    int log_fd, stats_fd;

    std::map<std::string, std::string> *voters_to_votes;
    std::map<std::string, int> *parties_to_votes;
    
    SyncUnits *sync_units;

} WorkerArgs;


void* master_routine(void *arguments);
void* worker_routine(void *arguments);

#endif // SERVER_H