#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define BUFFER_SIZE 200
#define USER_READY 1
#define USER_NOT_READY 0

// SHARED buffer structure
typedef struct buffer
{
    char data[BUFFER_SIZE];
    int user_status;
    int user1_pid;
    int user2_pid;
} Buffer;

Buffer *shared_memory_ptr;

// USER-2 signal handler function
// if signal is for SIGUSR2, from SIGUSR1 
// displays message from SIGUSR1 to SIGUSR2
void user2_sig_handler(int signal_id)
{
    if (signal_id == SIGUSR2)
    {
        printf("User 1: ");
        puts(shared_memory_ptr->data);
    }
}

int main()
{
    // is use to generate a unique key
    key_t key = ftok("shared_mem_chat", 99);

    // Get shared memory segment
    int shared_memory_id = shmget(key, sizeof(Buffer), IPC_CREAT | 0666);

    //      void *shmat(int shmid ,void *shmaddr ,int shmflg);
    // Before using shared memory segment, attach to it using shmat(). 
    //      shmid : is shared memory id. 
    //      shmaddr : OS will automatically choose the address.
    shared_memory_ptr = (Buffer *)shmat(shared_memory_id, NULL, 0);

    shared_memory_ptr->user2_pid = getpid();
    shared_memory_ptr->user_status = USER_NOT_READY;

    // listening for any signals from SIGUSR1
    // USER-2 catches the kill signal sent from USER-1
    signal(SIGUSR2, user2_sig_handler);

    while (1)
    {
        sleep(1);

        // starts the communication first
        printf("User 2: ");
        // pointer to data, number of chars to copy, file stream: stdin by default
        fgets(shared_memory_ptr->data, BUFFER_SIZE, stdin);

        // finished writing
        // declares that it is ready to accept reply
        shared_memory_ptr->user_status = USER_READY;

        // sends signal to SIGUSR1
        // sayign SIGUSR2 is ready
        kill(shared_memory_ptr->user1_pid, SIGUSR1);

        while (shared_memory_ptr->user_status == USER_READY)
            continue;
    }

    shmdt((void *)shared_memory_ptr);

    return EXIT_SUCCESS;
}
