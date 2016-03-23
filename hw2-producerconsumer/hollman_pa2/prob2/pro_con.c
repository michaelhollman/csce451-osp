#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#include "monitor.h"

#define NUM_PRO_CON_THREADS 10

#define verbose(...) if(IS_VERBOSE) { fprintf(stdout, __VA_ARGS__); fflush(stdout); }
bool IS_VERBOSE = false;

int insert_limit = -1;
int insert_count = 0;

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
    int id = (int)(unsigned long long)producerId;
    while (true)
    {
        char alpha = get_rand_char();
        mon_insert(alpha);
        verbose("Producer (%02d): '%c'\n", id, alpha);
    } 
}

// method used for consumer pthread initialization
void *consumer_thread(void *consumerId)
{
    int id = (int)(unsigned long long)consumerId;
    while (true)
    {
        char alpha = mon_remove();
        verbose("Consumer (%02d): '%c'\n", id, alpha);
        
        // terminating condition from argvs
        if (insert_limit > 0 && ++insert_count >= insert_limit)
        {
            verbose("Insertion limit reached for early termination\n");
            exit(0);
        }
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
    
    mon_init();
      
    // create and start producers
    pthread_t producers[NUM_PRO_CON_THREADS];
    for (int i = 0; i < NUM_PRO_CON_THREADS; i++)
    {
        pthread_create(&producers[i], NULL, producer_thread, (void *)(unsigned long long)i);
    }
    
    // create and start consumers
    pthread_t consumers[NUM_PRO_CON_THREADS];
    for (int i = 0; i < NUM_PRO_CON_THREADS; i++)
    {
        pthread_create(&consumers[i], NULL, consumer_thread, (void *)(unsigned long long)i);
    }

    pthread_exit(NULL);
}