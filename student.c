#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define BUFFER_SIZE 200
#define USER_READY 1
#define USER_NOT_READY -1
#define FILLED 0

// SHARED buffer structure
typedef struct buffer
{
    char data[BUFFER_SIZE];
    int user_status;
    int student_pid;
    int teacher_pid;
} Buffer;

Buffer *shared_memory_ptr;

// USER-1 signal handler function
// if signal is for SIGUSR1, from SIGUSR2 
// displays message from SIGUSR2 to SIGUSR1
void student_sig_handler(int signal_id)
{
    if (signal_id == SIGUSR1)
    {
        printf("Teacher: ");
        puts(shared_memory_ptr->data);
        
        // after this point,
        // program enters the while(1) loop
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
    
    // USER-1 pid & status info
    // initially, user is not ready
    shared_memory_ptr->student_pid = getpid();
    shared_memory_ptr->user_status = USER_NOT_READY;

    // listening for any signals from SIGUSR2
    // USER-1 catches the kill signal sent from USER-2
    signal(SIGUSR1, student_sig_handler);

    // if USER-1 is not ready, infinite loop
    // if USER-1 is ready, sleep for a second, send control to SIGUSR2
    //      accept the USER-1 message from std input
    //      update USER-1 status, saying it has filled buffer  
    //      USER-1 sends a KILL signal to USER-2 implying it has sent sent a message   
    while (1)
    {
        while (shared_memory_ptr->user_status != USER_READY)
            continue;

        sleep(1);

        printf("Student: ");
        fgets(shared_memory_ptr->data, BUFFER_SIZE, stdin);

        shared_memory_ptr->user_status = FILLED;

        kill(shared_memory_ptr->teacher_pid, SIGUSR2);
    }

    // detach shared memory segment
    shmdt((void *)shared_memory_ptr);

    // destroy shared memory, after detaching
    shmctl(shared_memory_id, IPC_RMID, NULL);
    
    return EXIT_SUCCESS;
}
