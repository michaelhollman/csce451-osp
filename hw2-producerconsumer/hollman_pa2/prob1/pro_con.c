#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

// 10 million (which is ridiculous)
#define BUFFER_SIZE 10000000

#ifndef SEM_VALUE_MAX
#define SEM_VALUE_MAX BUFFER_SIZE
#endif

// it's possible for the maximum semaphore size to be smaller than our buffer
#define BUFFER_SEM_SIZE SEM_VALUE_MAX < BUFFER_SIZE ? SEM_VALUE_MAX : BUFFER_SIZE
#define NUM_PRO_CON_THREADS 4

#define verbose(...) if(IS_VERBOSE) { fprintf(stdout, __VA_ARGS__); fflush(stdout); }
bool IS_VERBOSE = false;

int insert_limit = -1;
int insert_count = 0;

sem_t *mutex;
sem_t *empty;
sem_t *full;

// circular array-based character buffer
static char buffer[BUFFER_SIZE];
// buffer index that should be used for next input/write
static int buffer_in;
// buffer index that should be used for next output/read
static int buffer_out;

// helper method to generate a random alpha character
char get_rand_char()
{
    if (rand() % 2 == 0)
    {
        return 'a' + rand() % 26;
    }
    return 'A' + rand() % 26;
}

// method used for producer pthread initialization
void *producer_thread(void *producerId)
{
    int id = (int)producerId;
    while (true)
    {
        sem_wait(empty);
        sem_wait(mutex);
        verbose("enter crit\n");

        char rc = get_rand_char();
        buffer[buffer_in] = rc;
        
        printf("Producer (%02d): '%c' -> [%d]\n", id, rc, buffer_in);
        
        buffer_in++;
        buffer_in %= BUFFER_SIZE;        
        
        // terminating condition from argvs
        if (insert_limit > 0 && ++insert_count >= insert_limit)
        {
            verbose("Insertion limit reached for early termination\n");
            exit(0);
        }
        
        verbose("exit crit\n");        
        sem_post(mutex); 
        sem_post(full);       
    } 
}

// method used for consumer pthread initialization
void *consumer_thread(void *consumerId)
{
    int id = (int)consumerId;
    while (true)
    {
        sem_wait(full);
        sem_wait(mutex);
        verbose("enter crit\n");

        char rc = buffer[buffer_out];
        buffer[buffer_out] = '\0';
        
        printf("Consumer (%02d): '%c' <- [%d]\n", id, rc, buffer_out);
        
        buffer_out++;
        buffer_out %= BUFFER_SIZE;        
        
        verbose("exit crit\n");        
        sem_post(mutex); 
        sem_post(empty);
    }       
}

int main(int argc, char **argv)
{
    if (argc > 1) 
    {
        IS_VERBOSE = strcmp(argv[1], "-v") == 0 || (argc > 2 && strcmp(argv[2], "-v") == 0);
        
        int potentialLimit = strtol(argv[1], NULL, 10);
        if (potentialLimit > 0) 
        {
            insert_limit = potentialLimit;
        }   
    }
    
    // initialize and set up buffer, semaphores, etc.
    sem_unlink("/_procon_mutex");
    sem_unlink("/_procon_empty");
    sem_unlink("/_procon_full");
    mutex = sem_open("/_procon_mutex", O_CREAT, (S_IRUSR | S_IWUSR), 0);
    empty = sem_open("/_procon_empty", O_CREAT, (S_IRUSR | S_IWUSR), BUFFER_SEM_SIZE);
    full  = sem_open("/_procon_full",  O_CREAT, (S_IRUSR | S_IWUSR), 0);
    
    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED)
    {
        printf("sem_open failed\n");
        exit(-1);
    }
    
    buffer_in = 0;
    buffer_out = 0;
        
    // create and start producers
    pthread_t producers[NUM_PRO_CON_THREADS];
    for (int i = 0; i < NUM_PRO_CON_THREADS; i++)
    {
        pthread_create(&producers[i], NULL, producer_thread, (void *)i);
    }
    
    // create and start consumers
    pthread_t consumers[NUM_PRO_CON_THREADS];
    for (int i = 0; i < NUM_PRO_CON_THREADS; i++)
    {
        pthread_create(&consumers[i], NULL, consumer_thread, (void *)i);
    }
    
    // allow prod/cons to enter critical sections
    sem_post(mutex);
    
    pthread_exit(NULL);
}