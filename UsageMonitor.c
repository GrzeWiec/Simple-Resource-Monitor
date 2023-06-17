
// C program to implement
// the above approach
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
long file_size = 1;
char stat_file[1];
FILE * fptr;
char * init_array;
bool analizer_rdy = false;
long data_size = 0;

void *Reader()
{   
    while(1)
    {
    fptr = fopen("/proc/stat", "r" );

    long page_size = sysconf(_SC_PAGE_SIZE);
    init_array = (char *)calloc(page_size, sizeof(char));

    fread(init_array, sizeof(char), page_size, fptr);
    long size = strlen(init_array);
    data_size = page_size - size;

    printf("Length of string is : %ld, buffer space is %ld\n\n", size, data_size);

    fread(init_array,sizeof(char),sizeof(init_array),fptr);
    fclose(fptr);
    analizer_rdy = true;
    sleep(1);
    }
    return NULL;
}

void *Analizer()
{   
    static bool if_initialized = false;
    char * token;
    uint8_t cpu_number = 0;
    if(!if_initialized)
    {
        if_initialized = true;
        for(long i = 2; i < data_size; i++)
            if((init_array[i] == 'u')&&(init_array[i-1] == 'p')&&(init_array[i-2] == 'c')) 
                cpu_number++;
    }

    long sum = 0, idle;
    char corrector[] = " \n";
    float singe_cpu_usage = 0.0;
    float *total_cpu_usage;
    total_cpu_usage = (float *)calloc(cpu_number, sizeof(float));
    while(1)
    {
        if(analizer_rdy)
        {
        analizer_rdy = false;
        // puts(init_array);
        token = strtok(init_array, corrector);
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
                total_cpu_usage[cpu_ctr] = singe_cpu_usage;
                // if(cpu_ctr == 0)
                //     printf("%ld for total cpu\n", sum);
                // else
                //     printf("%ld for %d cpu\n", sum, cpu_ctr-1);
                
            }
            // for(int x = 0;x<21;x++)
            //     printf("cpu number: %d, usage: %f%%\n",x,total_cpu_usage[x]);
        }

    }
    return NULL;
}

int main()
{   
    pthread_t t_Reader_id;
    pthread_t t_Analizer_id;
    pthread_create(&t_Reader_id, NULL, Reader, NULL);
    pthread_create(&t_Analizer_id, NULL, Analizer, NULL);
    pthread_join(t_Reader_id, NULL);
    pthread_join(t_Analizer_id, NULL);
    free(init_array);
    return 0;
}