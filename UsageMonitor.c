
// C program to implement
// the above approach
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#define OK true
#define NOT_OK false
long file_size = 1;
char stat_file[1];
FILE * fptr;
char * cpu_data_array;
float * cpu_usage_array;
long data_size = 0;
uint8_t cpu_number = 0;
pthread_mutex_t lock_cpu_data;
pthread_mutex_t lock_cpu_usage;
bool analizer_rdy = false;
bool reader_status = false;
bool analizer_status = false;
bool printer_status = false;
volatile sig_atomic_t done = 0;
 
void term(int signum)
{
    done = 1;
}

void *Reader()
{   
    long page_size = sysconf(_SC_PAGE_SIZE);
    long read_size = 0;
    long read_idx = 0;
    cpu_data_array = (char *)calloc(page_size, sizeof(char));
    while(!done)
    {
    reader_status = OK;
    fptr = fopen("/proc/stat", "r" );

    while(pthread_mutex_trylock(&lock_cpu_data) != 0);

    read_idx = 0;
    do
    {   
        fflush(fptr);
        read_size = fread(&cpu_data_array[read_idx], sizeof(char), page_size, fptr);
        read_idx += read_size;
    }while(read_size);
    data_size = strlen(cpu_data_array);
    fclose(fptr);
    pthread_mutex_unlock(&lock_cpu_data);
    analizer_rdy = true;
    sleep(0.1);
    }
    return NULL;
}


void *Analizer()
{   
    while(!analizer_rdy);
    analizer_status = OK;
    
    while(!analizer_rdy);
    for(long i = 2; i < data_size; i++)
        if((cpu_data_array[i] == 'u')&&(cpu_data_array[i-1] == 'p')&&(cpu_data_array[i-2] == 'c')) 
            cpu_number++;

    long sum = 0, idle;
    char corrector[] = " \n\t";
    char * token;
    char * ptr_token;
    char * strtol_ptr = 0;
    float singe_cpu_usage = 0.0;
    cpu_usage_array = (float *)calloc(cpu_number, sizeof(float));
    while(!done)
    {
        ptr_token = NULL;
        analizer_status = OK;
        if(analizer_rdy)
        {
        while((pthread_mutex_trylock(&lock_cpu_usage) != 0) && (pthread_mutex_trylock(&lock_cpu_data) != 0));
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
                //     printf("%ld for total cpu\n", sum);
                // else
                //     printf("%ld for %d cpu\n", sum, cpu_ctr-1);
                
            }
        pthread_mutex_unlock(&lock_cpu_usage);
        pthread_mutex_unlock(&lock_cpu_data);
        }
    analizer_rdy = false;
    sleep(0.1);
    }
    return NULL;
}


void *Printer()
{
    while(!done)   
    {   
        printer_status = OK;
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


void *Watchdog()
{   
    sleep(1);
    if(!reader_status || !analizer_status || !printer_status)
    {   
        printf("Reader status - %d\n", reader_status);
        printf("Analizer status - %d\n", analizer_status);
        printf("Printer status - %d\n", printer_status);
        printf("Setup failed - exiting the program...\n");
        fflush(stdout);
        exit(2);
    }
    while(!done)
    {
        printf("\nWATCHDOG STATUS\n");
        printf("Reader status - %d\n", reader_status);
        printf("Analizer status - %d\n", analizer_status);
        printf("Printer status - %d\n", printer_status);
        fflush(stdout);

        if(!reader_status + !analizer_status + !printer_status > 0)
        {
            printf("Exiting the program...");
            exit(1);
        }
        reader_status = NOT_OK;
        analizer_status = NOT_OK;
        printer_status = NOT_OK;
        sleep(2);
    }
    return NULL;
}


void *Logger()
{
    return NULL;
}


int main()
{   
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);

    pthread_t t_Reader_id;
    pthread_t t_Analizer_id;
    pthread_t t_Printer_id;
    pthread_t t_Watchdog_id;
    pthread_t t_Logger_id;
    
    pthread_mutex_init(&lock_cpu_data, NULL);
    pthread_mutex_init(&lock_cpu_usage, NULL);

    pthread_create(&t_Reader_id, NULL, Reader, NULL);
    pthread_create(&t_Analizer_id, NULL, Analizer, NULL);
    pthread_create(&t_Printer_id, NULL, Printer, NULL);
    pthread_create(&t_Watchdog_id, NULL, Watchdog, NULL);
    pthread_create(&t_Logger_id, NULL, Logger, NULL);

    pthread_join(t_Reader_id, NULL);
    pthread_join(t_Analizer_id, NULL);
    pthread_join(t_Printer_id, NULL);
    pthread_join(t_Watchdog_id, NULL);
    pthread_join(t_Logger_id, NULL);

    while(!done);
    pthread_mutex_destroy(&lock_cpu_data);
    pthread_mutex_destroy(&lock_cpu_usage);
    free(cpu_data_array);
    free(cpu_usage_array);
    return 0;
}