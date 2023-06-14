
// C program to implement
// the above approach
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
long file_size = 1;
char stat_file[1];
FILE * fptr;
char * init_array;

void init()
{
    fptr = fopen("/proc/stat", "r" );
    long page_size = sysconf(_SC_PAGE_SIZE);
    init_array = (char *)calloc(page_size, sizeof(char));
    fread(init_array, sizeof(char), page_size, fptr);
    long size = strlen(init_array);
    printf("Length of string is : %ld, buffer space is %ld\n\n", size, (page_size - size));
}

int main()
{  
    while(1)
    {
    init();
    fread(init_array,sizeof(char),sizeof(init_array),fptr);
    puts(init_array);
    fclose(fptr);
    free(init_array);
    sleep(1);
    }
    return 0;
}