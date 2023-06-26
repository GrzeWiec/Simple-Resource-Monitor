#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

void *Printer()
{
    extern pthread_mutex_t lock_cpu_usage;
    extern volatile sig_atomic_t done;
    extern char * cpu_data_array;
    extern float * cpu_usage_array;

    uint8_t cpu_number = 0;
    uint32_t data_size = strlen(cpu_data_array);
    
    sleep(2);
    
    for(uint32_t i = 2; i < data_size; i++)
    if((cpu_data_array[i] == 'u')&&(cpu_data_array[i-1] == 'p')&&(cpu_data_array[i-2] == 'c')) 
        cpu_number++;

    while(!done)   
    {   
        while(pthread_mutex_trylock(&lock_cpu_usage) != 0);
        for(int curr_cpu = 0; curr_cpu < cpu_number; curr_cpu++)
        {
            if(curr_cpu == 0)
                printf("\nTotal CPU usage: %f%%\n", cpu_usage_array[curr_cpu]);
            else
                printf("CPU %d usage: %f%%\n", curr_cpu-1, cpu_usage_array[curr_cpu]);
        }
        pthread_mutex_unlock(&lock_cpu_usage);
        sleep(1);
    }
    return NULL;
}