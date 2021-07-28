#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define BUFFER_SIZE 200
#define USER_READY 1
#define USER_NOT_READY 0

typedef struct buffer
{
    char data[BUFFER_SIZE];
    int user_status;
    int student_pid;
    int teacher_pid;
} Buffer;

Buffer *shared_memory_ptr;

void teacher_sig_handler(int signal_id)
{
    if (signal_id == SIGUSR2)
    {
        printf("Student: ");
        puts(shared_memory_ptr->data);
    }
}

int main()
{
    key_t key = ftok("shared_mem_chat", 99);
    int shared_memory_id = shmget(key, sizeof(Buffer), IPC_CREAT | 0666);
    shared_memory_ptr = (Buffer *)shmat(shared_memory_id, NULL, 0);
    shared_memory_ptr->teacher_pid = getpid();
    shared_memory_ptr->user_status = USER_NOT_READY;

    signal(SIGUSR2, teacher_sig_handler);
    while (1)
    {
        sleep(1);
        printf("Teacher: ");
        fgets(shared_memory_ptr->data, BUFFER_SIZE, stdin);
        shared_memory_ptr->user_status = USER_READY;

        kill(shared_memory_ptr->student_pid, SIGUSR1);

        while (shared_memory_ptr->user_status == USER_READY)
            continue;
    }

    shmdt((void *)shared_memory_ptr);
    return EXIT_SUCCESS;
}
