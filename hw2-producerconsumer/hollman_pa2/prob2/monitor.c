#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#include "monitor.h"

// 10 million (which is ridiculous)
#define BUFFER_SIZE 10000000

#ifndef SEM_VALUE_MAX
#define SEM_VALUE_MAX BUFFER_SIZE
#endif

// it's possible for the maximum semaphore size to be smaller than our buffer
#define BUFFER_SEM_SIZE SEM_VALUE_MAX < BUFFER_SIZE ? SEM_VALUE_MAX : BUFFER_SIZE

sem_t *mutex;
sem_t *empty;
sem_t *full;

// circular array-based character buffer
char buffer[BUFFER_SIZE];
// buffer index that should be used for next input/write
int buffer_in;
// buffer index that should be used for next output/read
int buffer_out;

void mon_init() 
{
    sem_unlink("/_procon_mutex");
    sem_unlink("/_procon_empty");
    sem_unlink("/_procon_full");
    
    mutex = sem_open("/_procon_mutex", O_CREAT, (S_IRUSR | S_IWUSR), 1);
    empty = sem_open("/_procon_empty", O_CREAT, (S_IRUSR | S_IWUSR), BUFFER_SEM_SIZE);
    full = sem_open("/_procon_full",  O_CREAT, (S_IRUSR | S_IWUSR), 0); 
    
    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED)
    {
        printf("sem_open failed\n");
        exit(-1);
    }
    
    buffer_in = 0;
    buffer_out = 0;   
}

void mon_insert(char alpha)
{
    sem_wait(empty);
    sem_wait(mutex);

    buffer[buffer_in] = alpha;    
    printf("Insert: '%c' -> [%d]\n", alpha, buffer_in);
    
    buffer_in++;
    buffer_in %= BUFFER_SIZE;     
        
    sem_post(mutex);
    sem_post(full);       
}


char mon_remove()
{
    sem_wait(full);
    sem_wait(mutex);
    
    char alpha = buffer[buffer_out];
    buffer[buffer_out] = '\0';
    
    printf("Remove: '%c' <- [%d]\n", alpha, buffer_out);
    
    buffer_out++;
    buffer_out %= BUFFER_SIZE;        
        
    sem_post(mutex); 
    sem_post(empty);
    
    return alpha;
}