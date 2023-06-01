System Programming Course -- Assignment II\
May 2023\
Author : Yannis Zervakis

## Abstract
In this project we implemented an multi-threaded app that simulates a poll based on client-server protocol. Server (`poller`) splits it's job into one **master thread** and multiple  **worker threads**. Client (`pollSwayer`) splits it's job into multiple **voter threads**. Server and client communicate over a (Internet) socket  with multiple connections. Server sends some messages to client who replies with voter's names and parties that server has to process.

### Share Data, Synchronization, Termination
Between master and worker threads there are some shared data. The reason that all worker threads have the same structure of `WorkerArgs` is that each worker thread must refers to same data. So firstly, the buffer is implemented as a queue (STL) that master thread pushes at the end of it connections that worker threads pops them from the beginning in order to communicate with the connected client. We can also suppose that the `log` file is also shared data because each worker thread wants to has access to it and convert it. For the synchronization, we have a structure called SyncUnits that contains two conditions variables (`buffer_nonfull` and `buffer_nonempty`) and two mutexes (`buffer_mutex` and `log_mutex`). Mutex `buffer_mutex` locks when a thread is trying to have access to the buffer. Correspondigly, `log_mutex` locks when a worker thread is trying to has access to the `log` file. Client (`pollSwayer`) terminates when all votes have been sent. Server (`poller`) terminates upon receiving signal `SIGINT`. For this reason, there is a global variable called `sigint_received` that is initialized with the value `false`. When the program receives signal `SIGINT`, **only** the master thread will execute the corresponding handler, which sets the value of `sigint_received` to `true`. Note that, in order to let only master execute the handler, after creation of master thread, server must block the signal `SIGINT` (`block_sigint`). The same applies to worker threads.

### Server
Server (`poller`) gets some command line arguments (port number in which socket will be created, number of worker threads, size of buffer that holds connections over the socket, a `log` file on which the votes will be stored, and `statistics` file on which the number of votes for each party will be stored). First of all, server does some command line arguments checks, about the number of worker threads that master thread will produce and the size of buffer. Then creates a socket from which server will listen to connections, with the usual sequence of system calls (socket, bind, listen). After that, creates the required arguments for master thread and the master thread too (pthread_create). At the end, server waits (pthread_join) for master thread to do it's job, updates the `statistics` file and close the socket gracefully (shutdown, close).

#### Master
Master thread is at bottom responsible for the connections over the socket. Master routine (`master.cpp`) is the function that the master thread runs. Firstly, creates the required arguments for each worker thread and does some initializations too (structure members and mutexes' and condition variables' initializations with the corresponding calls from pthread library). Notice that every worker thread has the same structure of arguments. Afterward, master thread gets in the basic loop for accepting connections. The structure of the loop is simple. First of all we call the `accept` system call for accepting a new connection. If `accept` failed, then if `SIGINT` signal received we just wake up all worker threads (`broadcast` and `unlock`) in order to terminate themselves. Then, if `accept` returned successfully, we lock the the mutex that refers to buffer access. If the buffer is full then master thread waits on a condition variable called `nonfull` (waiting for an worker thread to release positions in the buffer). If the buffer is non-full we just push the socket's file descriptor into it, and wake up a signle thread (`signal`, `unlock`) to manage the work on this connection. After all connections (in fact after `SIGINT` is received) master thread waits for all worker threads to exit from their routines (`pthread_join`), and destroys the space was allocated for mutexes and condition variables.

#### Worker
An worker thread works on a connection, receives from clients votes and updates servers stored data about the votes. So, each worker thread runs an worker routine (`worker.cpp`) which has basically a `while(1)`. Firstly, it checks if the buffer is empty and if it is, worker threads waits on a condition called `nonempty`. If the buffer isn't empty, if `SIGINT` signal got received, the loop breaks and exit is called. If not, pops the first socket file descriptor on the buffer, wake up the master thread for continue to accepting connections because buffer now is non full (`signal`, `unlock`) and starts the communication with a client on the connection. Worker sends some messages to client (`SEND NAME PLEASE`, `SEND VOTE PLEASE`, `ALREADY VOTED`, \<VOTE>) and receives some information about votes (voters name's, parties). With these information updates the `log` file and close the connection gracefully.

### Client
Client (`pollSwayer`) is a multithreaded program too. Client's job is to open a file filled with votes (voter's name and the party he voted) and for each line in this file to create a new thread that is going to communicate with server and send the vote. So, client's code is just a loop on the number of lines of input file, in which a voter thread is created with required arguments. Note that client assumes that the given file is ends with `'\n'` character. Client also skips empty lines by using a bool array that holds the actual votes. If the line was empty, then `pthread_join` system call won't be executed for that thread.

#### Voter
Voter threads send votes and receive messages from server (in fact, from workers). Each voter thread runs the thread-function which is `voter_routine`. In this routine, the thread  is connected to the socket corresponding to the server's name and port. Receives some messages from server and sends the name and the party part from a vote.

### Bash Scripts
The three bash scripts have similar structure. For `create_input.sh`, we just creating random firstname and lastname using `$RANDOM` variable in order to select the random length of name and random characters from an array with all candidate characters. Code of `tallyVotes.sh` and `processLogFile.sh` is simalar. Note that for having a map between parties and num of votes that each party got, associative arrays was used.

### Makefile, Compilation and Execution
Programs `poller` and `pollSwayer` supports separate compilation using a Makefile. By typing `make` inside the project's directory two executable programs are created in same directory. Note that, all results after the executions are stored in `results` directory. For executing `poller` (to be inside project's base directory is needed) type :
```
./poller <port> <numWorkerThreads> <bufferSize> <poll-log> <poll-stats>
```
 For executing `pollSwayer` (to be inside project's base directory is needed) type :
 ```
 ./pollSwayer <serverName> <port> <inputFile>
 ```
 Here is an example of a sequence of command executions :

Terminal no.1
```cd scripts
./create_input.sh politicalParties.txt 2000
./tallyVotes.sh tallyResultFile
make
./poller 8008 20 25 log.txt stats.txt
^C (after all commands from terminal no.2)
```
Terminal no.2
```
./pollSwayer localhost 8008 results/inputFile 
cd scripts/
./processLogFile.sh ../results/log.txt
cd ../results/
diff pollerResultsFile tallyResultsFile
cd ../
make clean
```

In the above example, the result of diff command must be nothing. 