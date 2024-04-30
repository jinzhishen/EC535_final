#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

#define I2C_BUS        "/dev/i2c-2" // I2C bus device on a Raspberry Pi 3
#define I2C_ADDR       0x27         // I2C slave address for the LCD module
#define BINARY_FORMAT  " %c  %c  %c  %c  %c  %c  %c  %c\n"
#define BYTE_TO_BINARY(byte) \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 
  
int lcd_backlight;
int debug;
char address; 
int i2cFile;
void reset_screen(); 
void display_number_to_lcd(int number);
void display_value(int value);


void i2c_start() {
   if((i2cFile = open(I2C_BUS, O_RDWR)) < 0) {
      printf("Error failed to open I2C bus [%s].\n", I2C_BUS);
      exit(-1);
   }
   // set the I2C slave address for all subsequent I2C device transfers
   if (ioctl(i2cFile, I2C_SLAVE, I2C_ADDR) < 0) {
      printf("Error failed to set I2C address [%s].\n", I2C_ADDR);
      exit(-1);
   }
}

void i2c_stop() { close(i2cFile); }

void i2c_send_byte(unsigned char data) {
   unsigned char byte[1];
   byte[0] = data;
   if(debug) printf(BINARY_FORMAT, BYTE_TO_BINARY(byte[0]));
   write(i2cFile, byte, sizeof(byte)); 
   /* -------------------------------------------------------------------- *
    * Below wait creates 1msec delay, needed by display to catch commands  *
    * -------------------------------------------------------------------- */
   usleep(1000);
}

void main() { 
    char value_str[7];
    char value_str_1[7];
    long int value_int = 0;
    long int value_volt = 0;

    FILE* f0 = fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", "r");
    FILE* f1 = fopen("/sys/bus/iio/devices/iio:device0/in_voltage1_raw", "r");

   i2c_start(); 
   debug=1;

   fread(&value_str, 6, 6, f0);
   fread(&value_str_1, 6, 6, f1);
   value_int = strtol(value_str,NULL,0) - strtol(value_str_1,NULL,0);
   reset_screen();

   /* -------------------------------------------------------------------- *
    * Start writing anything
    * -------------------------------------------------------------------- */
   value_volt = value_int * 1.8/4096*1000;
   display_number_to_lcd(value_volt);

   if(debug) printf("Finished writing to display.\n");
   i2c_stop(); 
}

void reset_screen() {
    /* -------------------------------------------------------------------- *
    * Initialize the display, using the 4-bit mode initialization sequence *
    * -------------------------------------------------------------------- */
   if(debug) printf("Init Start:\n");
   if(debug) printf("D7 D6 D5 D4 BL EN RW RS\n");

   usleep(15000);             // wait 15msec
   i2c_send_byte(0b00110100); // D7=0, D6=0, D5=1, D4=1, RS,RW=0 EN=1
   i2c_send_byte(0b00110000); // D7=0, D6=0, D5=1, D4=1, RS,RW=0 EN=0
   usleep(4100);              // wait 4.1msec
   i2c_send_byte(0b00110100); // 
   i2c_send_byte(0b00110000); // same
   usleep(100);               // wait 100usec
   i2c_send_byte(0b00110100); //
   i2c_send_byte(0b00110000); // 8-bit mode init complete
   usleep(4100);              // wait 4.1msec
   i2c_send_byte(0b00100100); //
   i2c_send_byte(0b00100000); // switched now to 4-bit mode

   /* -------------------------------------------------------------------- *
    * 4-bit mode initialization complete. Now configuring the function set *
    * -------------------------------------------------------------------- */
   usleep(40);                // wait 40usec
   i2c_send_byte(0b00100100); //
   i2c_send_byte(0b00100000); // keep 4-bit mode
   i2c_send_byte(0b10000100); //
   i2c_send_byte(0b10000000); // D3=2lines, D2=char5x8

   /* -------------------------------------------------------------------- *
    * Next turn display off                                                *
    * -------------------------------------------------------------------- */
   usleep(40);                // wait 40usec
   i2c_send_byte(0b00000100); //
   i2c_send_byte(0b00000000); // D7-D4=0
   i2c_send_byte(0b10000100); //
   i2c_send_byte(0b10000000); // D3=1 D2=display_off, D1=cursor_off, D0=cursor_blink

   /* -------------------------------------------------------------------- *
    * Display clear, cursor home                                           *
    * -------------------------------------------------------------------- */
   usleep(40);                // wait 40usec
   i2c_send_byte(0b00000100); //
   i2c_send_byte(0b00000000); // D7-D4=0
   i2c_send_byte(0b00010100); //
   i2c_send_byte(0b00010000); // D0=display_clear

   /* -------------------------------------------------------------------- *
    * Set cursor direction                                                 *
    * -------------------------------------------------------------------- */
   usleep(40);                // wait 40usec
   i2c_send_byte(0b00000100); //
   i2c_send_byte(0b00000000); // D7-D4=0
   i2c_send_byte(0b01100100); //
   i2c_send_byte(0b01100000); // print left to right

   /* -------------------------------------------------------------------- *
    * Turn on the display                                                  *
    * -------------------------------------------------------------------- */
   usleep(40);                // wait 40usec
   i2c_send_byte(0b00000100); //
   i2c_send_byte(0b00000000); // D7-D4=0
   i2c_send_byte(0b11100100); //
   i2c_send_byte(0b11100000); // D3=1 D2=display_on, D1=cursor_on, D0=cursor_blink

   if(debug) printf("Init End.\n");
   sleep(1);

   if(debug) printf("Writing HELLO to display\n");
   if(debug) printf("D7 D6 D5 D4 BL EN RW RS\n");
}

void display_value(int value)
{
    switch(value) {
        case 0:
            i2c_send_byte(0b00111101); //
            i2c_send_byte(0b00111001); // send 0011=3
            i2c_send_byte(0b00001101); //
            i2c_send_byte(0b00001001); // send 0000=0 = 0x30 ='0'
            break;
        case 1:
            i2c_send_byte(0b00111101); //
            i2c_send_byte(0b00111001); // send 0011=3
            i2c_send_byte(0b00011101); //
            i2c_send_byte(0b00011001); // send 0001=1 = 0x31 ='1'
            break;
        case 2:
            i2c_send_byte(0b00111101); //
            i2c_send_byte(0b00111001); // send 0011=3
            i2c_send_byte(0b00101101); //
            i2c_send_byte(0b00101001); // send 0010=2 = 0x32 ='2'
            break;
        case 3:
            i2c_send_byte(0b00111101); //
            i2c_send_byte(0b00111001); // send 0011=3
            i2c_send_byte(0b00111101); //
            i2c_send_byte(0b00111001); // send 0011=3 = 0x33 ='3'
            break;
        case 4:
            i2c_send_byte(0b00111101); //
            i2c_send_byte(0b00111001); // send 0011=3
            i2c_send_byte(0b01001101); //
            i2c_send_byte(0b01001001); // send 0100=4 = 0x34 ='4'
            break;
        case 5:
            i2c_send_byte(0b00111101); //
            i2c_send_byte(0b00111001); // send 0011=3
            i2c_send_byte(0b01011101); //
            i2c_send_byte(0b01011001); // send 0101=5 = 0x35 ='5'
            break;
        case 6:
            i2c_send_byte(0b00111101); //
            i2c_send_byte(0b00111001); // send 0011=3
            i2c_send_byte(0b01101101); //
            i2c_send_byte(0b01101001); // send 0110=6 = 0x36 ='6'
            break;
        case 7:
            i2c_send_byte(0b00111101); //
            i2c_send_byte(0b00111001); // send 0011=3
            i2c_send_byte(0b01111101); //
            i2c_send_byte(0b01111001); // send 0111=7 = 0x37 ='7'
            break;
        case 8:
            i2c_send_byte(0b00111101); //
            i2c_send_byte(0b00111001); // send 0011=3
            i2c_send_byte(0b10001101); //
            i2c_send_byte(0b10001001); // send 1000=8 = 0x38 ='8'
            break;
        case 9:
            i2c_send_byte(0b00111101); //
            i2c_send_byte(0b00111001); // send 0011=3
            i2c_send_byte(0b10011101); //
            i2c_send_byte(0b10011001); // send 1001=9 = 0x39 ='9'
            break;
    }
}

void display_number_to_lcd(int number) {
    // Convert the number to a string
    char buffer[20]; 
    sprintf(buffer, "%d", number);
    
    // Loop through each character in the string
    for (int i = 0; buffer[i] != '\0'; i++) {
        // Convert the character to its digit value
        int digit = buffer[i] - '0';
        
        // Display the digit on LCD
        display_value(digit);
    }

    i2c_send_byte(0b01011101); //
    i2c_send_byte(0b01011001); // send 0101 = 5
    i2c_send_byte(0b01101101); //
    i2c_send_byte(0b01101001); // send 0110 = 6
}


