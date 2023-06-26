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
FILE * fptr;
char * cpu_data_array;

void *Reader()
{   
    uint32_t data_size = 0;
    uint32_t page_size = sysconf(_SC_PAGE_SIZE);
    cpu_data_array = (char *)calloc(page_size, sizeof(char));
    while(1)
    {
        long read_size = 0;
        long read_idx = 0;
        
        fptr = fopen("/proc/stat", "r" );
        read_idx = 0;
        do
        {   
            fflush(fptr);
            read_size = fread(&cpu_data_array[read_idx], sizeof(char), page_size, fptr);
            read_idx += read_size;
        }while(read_size);
        fclose(fptr);
        puts(cpu_data_array);
        sleep(1);
    }
    return NULL;
}