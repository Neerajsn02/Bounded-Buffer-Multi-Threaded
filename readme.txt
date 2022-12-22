AUTHOR: NEERAJ SURESH NARAYANAN
CCID: nnarayan
Student ID: 1666155

IMPLEMENTATION DETAILS - 

NOTE: The action of getting input from user has not been made, please redirect input from a different file
      > ./prodcon 3 4 < test_commands.txt
      Please only execute this way, using input redirection from a file

SYNCHRONIZATION DETAILS - 

Synchronization has been done by using 2 Mutexes
1. The first mutex (&mutex_taskqueue) makes sure that only 1 thread can work on the task queue at a given time.
2. 2nd mutex (&mutex_filewrite) ensures that only 1 thread can write to a file at a time.

The edge case of when the taskqueue is empty is handled using condition variables and signalling.
If the taskqueue is empty, the thread calls pthread_cond_wait().
When the producer adds an item to the taskqueue, it signals the waiting threads using pthread_cond_signal().