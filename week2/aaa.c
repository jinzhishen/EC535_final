#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

void main() { 
    char value_str[7];
    char value_str_1[7];
    long int value_int = 0;
    long int value_volt = 0;
    char button;


   FILE* f4 = fopen("/sys/class/gpio/gpio67/direction", "w");
    if (f4 != NULL) {
    fputs("out", f4);
    fclose(f4);
    } else {
    // Handle error opening the file
   }

   FILE* f5= fopen("/sys/class/gpio/gpio67/value", "w");
       fprintf(f5, "1"); 
    fclose(f5);
 
}