#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void main(){
    char value_str[7];
    long int value_int = 0;

    FILE* f0 = fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", "r");

    while(1){
            fread(&value_str, 6, 6, f0);
            value_int = strtol(value_str,NULL,0);
            printf("0 %li\n", value_int);
            fflush(stdout);

            usleep(5000);
            rewind(f0);
    }
  }