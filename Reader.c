#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

FILE * fptr;


void *Reader()
{   
    extern pthread_mutex_t lock_cpu_data;
    extern volatile sig_atomic_t done;
    extern char * cpu_data_array;

    uint32_t data_size = 0;
    uint32_t page_size = sysconf(_SC_PAGE_SIZE);
    cpu_data_array = (char *)calloc(page_size, sizeof(char));
    while(!done)
    {
        long read_size = 0;
        long read_idx = 0;
        
        fptr = fopen("/proc/stat", "r" );
        read_idx = 0;
        while(pthread_mutex_trylock(&lock_cpu_data) != 0); //mutex lock
        do
        {   
            fflush(fptr);
            read_size = fread(&cpu_data_array[read_idx], sizeof(char), page_size, fptr);
            read_idx += read_size;
        }while(read_size);
        sleep(1);
        pthread_mutex_unlock(&lock_cpu_data); //mutex unlock
        fclose(fptr);
    }
    return NULL;
}