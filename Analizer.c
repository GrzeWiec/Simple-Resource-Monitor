#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

void *Analizer()
{   
    extern pthread_mutex_t lock_cpu_data;
    extern pthread_mutex_t lock_cpu_usage;
    extern volatile sig_atomic_t done;
    extern char * cpu_data_array;
    extern float * cpu_usage_array;

    uint8_t cpu_number = 0;
    uint32_t data_size = strlen(cpu_data_array);

    sleep(1);

    for(uint32_t i = 2; i < data_size; i++)
        if((cpu_data_array[i] == 'u')&&(cpu_data_array[i-1] == 'p')&&(cpu_data_array[i-2] == 'c')) 
            cpu_number++;

    uint32_t sum = 0, idle;
    char corrector[] = " \n\t";
    char * token; 
    char * ptr_token;
    char * strtol_ptr = 0;
    float singe_cpu_usage = 0.0;
    cpu_usage_array = (float *)calloc(cpu_number, sizeof(float));

    while(!done)
    {
        ptr_token = NULL;
        while((pthread_mutex_trylock(&lock_cpu_usage) != 0) && (pthread_mutex_trylock(&lock_cpu_data) != 0)); //mutex lock
        token = strtok_r(cpu_data_array, corrector, &ptr_token);
            for(uint8_t cpu_ctr = 0; cpu_ctr < cpu_number; cpu_ctr++)
            {
                sum = 0;
                for(uint8_t column_ctr = 0; column_ctr < 10; column_ctr++)
                {
                token = strtok_r(NULL, corrector, &ptr_token);
                // printf("cpu: %d,column: %d,value: %s\n", cpu_ctr, column_ctr, token);
                fflush(stdout);
                if (token!=NULL)
                    {
                    sum += strtol(token, &strtol_ptr, 10);
                    if (column_ctr == 3)
                        idle = strtol(token, &strtol_ptr, 10);
                    }
                }
                token = strtok_r(NULL, corrector, &ptr_token);
                singe_cpu_usage = (1 - ((float)idle / (float)sum)) * 100;
                cpu_usage_array[cpu_ctr] = singe_cpu_usage;
                // if(cpu_ctr == 0)
                //     printf("%d for total cpu\n", sum);
                // else
                //     printf("%d for %d cpu\n", sum, cpu_ctr-1);
                
        pthread_mutex_unlock(&lock_cpu_usage);  
        pthread_mutex_unlock(&lock_cpu_data);
        }
    }
    return NULL;
}