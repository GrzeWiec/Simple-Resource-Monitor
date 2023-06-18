
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


void *Reader()
{   
    while(1)
    {
    reader_status = OK;
    fptr = fopen("/proc/stat", "r" );

    long page_size = sysconf(_SC_PAGE_SIZE);
    cpu_data_array = (char *)calloc(page_size, sizeof(char));

    while(pthread_mutex_trylock(&lock_cpu_data) != 0);
    fread(cpu_data_array, sizeof(char), page_size, fptr);
    long size = strlen(cpu_data_array);
    data_size = page_size - size;
    // printf("Length of string is : %ld, buffer space is %ld\n\n", size, data_size);
    fread(cpu_data_array, sizeof(char), sizeof(cpu_data_array), fptr);
    pthread_mutex_unlock(&lock_cpu_data);

    fclose(fptr);
    analizer_rdy = true;
    sleep(1);
    }
    return NULL;
}


void *Analizer()
{   
    analizer_status = OK;
    static bool if_initialized = false;
    char * token;
    if(!if_initialized)
    {
        if_initialized = true;
        for(long i = 2; i < data_size; i++)
            if((cpu_data_array[i] == 'u')&&(cpu_data_array[i-1] == 'p')&&(cpu_data_array[i-2] == 'c')) 
                cpu_number++;
    }

    long sum = 0, idle;
    char corrector[] = " \n";
    float singe_cpu_usage = 0.0;
    cpu_usage_array = (float *)calloc(cpu_number, sizeof(float));
    while(1)
    {
        analizer_status = OK;
        if(analizer_rdy)
        {
        analizer_rdy = false;
        // puts(cpu_data_array);
        while((pthread_mutex_trylock(&lock_cpu_usage) != 0) && (pthread_mutex_trylock(&lock_cpu_data) != 0));
        token = strtok(cpu_data_array, corrector);
            for(uint8_t cpu_ctr = 0; cpu_ctr < cpu_number; cpu_ctr++)
            {
                sum = 0;
                for(uint8_t column_ctr = 0; column_ctr < 10; column_ctr++)
                {
                token = strtok(NULL, corrector);
                // printf("cpu: %d,column: %d,value: %s\n",cpu_ctr,column_ctr,token);
                if (token!=NULL)
                    sum += atol(token);
                if (column_ctr == 3)
                    idle = atol(token);
                }
                token = strtok(NULL, corrector);
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
    }
    return NULL;
}


void *Printer()
{
    while(1)   
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
        exit(2);
    }
    while(1)
    {
        printf("\nWATCHDOG STATUS\n");
        printf("Reader status - %d\n", reader_status);
        printf("Analizer status - %d\n", analizer_status);
        printf("Printer status - %d\n", printer_status);

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
}


void *Logger()
{



}


int main()
{   
    pthread_t t_Reader_id;
    pthread_t t_Analizer_id;
    pthread_t t_Printer_id;
    pthread_t t_Watchdog_id;
    
    pthread_mutex_init(&lock_cpu_data, NULL);
    pthread_mutex_init(&lock_cpu_usage, NULL);

    pthread_create(&t_Reader_id, NULL, Reader, NULL);
    pthread_create(&t_Analizer_id, NULL, Analizer, NULL);
    pthread_create(&t_Printer_id, NULL, Printer, NULL);
    pthread_create(&t_Watchdog_id, NULL, Watchdog, NULL);

    pthread_join(t_Reader_id, NULL);
    pthread_join(t_Analizer_id, NULL);
    pthread_join(t_Printer_id, NULL);
    pthread_join(t_Watchdog_id, NULL);

    free(cpu_data_array);
    free(cpu_usage_array);
    return 0;
}