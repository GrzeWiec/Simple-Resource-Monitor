#include "Reader.c"
#include "Analizer.c"
#include "Printer.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

char * cpu_data_array;
float * cpu_usage_array;
volatile sig_atomic_t done = false;
pthread_mutex_t lock_cpu_data;
pthread_mutex_t lock_cpu_usage;
 
void term(int signum)
{
    done = true;
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

    pthread_join(t_Reader_id, NULL);
    pthread_join(t_Analizer_id, NULL);
    pthread_join(t_Printer_id, NULL);

    while(!done);
    pthread_mutex_destroy(&lock_cpu_data);
    pthread_mutex_destroy(&lock_cpu_usage);
    free(cpu_data_array);
    free(cpu_usage_array);

    return 0;
}