#include "Reader.c"
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

volatile sig_atomic_t done = 0;
 
void term(int signum)
{
    done = 1;
}

int main()
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);

    extern char * cpu_data_array;
    pthread_t t_Reader_id;
    pthread_t t_Analizer_id;
    pthread_t t_Printer_id;
    pthread_t t_Watchdog_id;
    pthread_t t_Logger_id;

    pthread_create(&t_Reader_id, NULL, Reader, NULL);
    pthread_join(t_Reader_id, NULL);
    free(cpu_data_array);

    return 0;
}