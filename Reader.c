
// C program to implement
// the above approach
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 
// Driver code
int main()
{  
   while(1)
   {
   char command[50];
   strcpy(command, "cat /proc/stat");
   system(command);
   sleep(1);
   }
   return 0;
}