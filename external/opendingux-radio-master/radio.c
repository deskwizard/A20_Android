/* 
 * OpenDingux Radio Utility
 *
 * Copyright (c) 2011 Jérôme VERES
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <limits.h>
 
static int i2c_fd;

static int i2c_open(void)
{
  i2c_fd = open("/dev/i2c-0", O_RDWR);
  if (!i2c_fd) {
    perror("Unable to open i2c dev file");
    return -1;
  }

  if (ioctl(i2c_fd, I2C_SLAVE, 0x10) < 0) {
    perror("Unable to set slave address");
    return -2;
  }
  return 0;
}

static int i2c_close(void)
{
  if (close(i2c_fd) < 0) {
    perror("Unable to close i2c dev file");
    return -1;
  }
  return 0;
}

static int i2c_read(unsigned char buffer[], int length)
{
  if (read(i2c_fd, buffer, length) != length) {
    perror("Unable to read in i2c");
    return -1;
  }
  return 0;
}

static int i2c_write(unsigned char buffer[], int length)
{
  if (write(i2c_fd, buffer, length) != length ) {
    perror("Unable to write in i2c");
    return -1;
  }
  return 0;
}

char *char2binary(char *dst, unsigned char value)
{
   unsigned char bit;
   char *start = dst;
   for ( bit = 1 << (CHAR_BIT - 1); bit; bit >>= 1 )
   {
      *dst++ = value & bit ? '1' : '0';
   }
   *dst = '\0';
   return start;
}

static int print_buf(unsigned char buffer[], int startaddress, int length)
{
  int i, j;
  char b1 [ CHAR_BIT * sizeof(unsigned char) + 1 ];
  char b2 [ CHAR_BIT * sizeof(unsigned char) + 1 ];
  j = startaddress;
  for(i=0;i<length;i+=2)
  {
    printf(" %2X %2X %2X %s %s\n", j, buffer[i], buffer[i+1], char2binary(b1, buffer[i]), char2binary(b2, buffer[i+1]));
    j++;
    if (j > 0x3F)
    j = 0x00;
  }
  return 0;
}

int RadioShowRegisters(void)
{
  static unsigned char buffer[128]; 

  i2c_open();
  i2c_read(buffer, 128);
  i2c_close();
  print_buf(&buffer[108], 0x00, 12);
  print_buf(buffer,       0x0A, 4);
  print_buf(&buffer[12],  0x10, 2);
  return 0;
}

int RadioShowAllRegisters(void)
{
  static unsigned char buffer[128]; 
  i2c_open();
  i2c_read(buffer, 128);
  i2c_close();
  //print_buf(buffer, 0x0A, 128); 
  print_buf(&buffer[108], 0x00, 20);
  print_buf(buffer, 0x0A, 108);
  return 0;
}

int RadioShowHelp(void)
{
  printf("radio --start       : enable FM chip\n");
  printf("radio --stop        : disable FM chip\n");
  printf("radio --seekup      : seek up\n");
  printf("radio --seekdown    : seek down\n");
  printf("radio --status      : show FM chip status\n");
  printf("radio --register    : show registers of FM chip\n");
  printf("radio --registerall : show all registers of FM chip\n");
  printf("radio --help        : show this help\n");
  return 0;
}


int RadioStart(void)
{
  static unsigned char buffer[]=
  {
    0b11000000, 0b00000001,	//0x02
    0b00000000, 0b00000000,     //0x03
    0b00000000, 0b00000000,     //0x04
    0b00000100, 0b01110111,     //0x05
  };
  i2c_open();
  i2c_write(buffer, 8);
  i2c_close();
  return 0;
}

int RadioStop(void)
{
  static unsigned char buffer[]=
  {
    0b11000000, 0b00000000	//0x02
  };
  i2c_open();
  i2c_write(buffer, 2);
  i2c_close();
  return 0;
}

int RadioSeekUp(void)
{
  static unsigned char buffer[]=
  {
    0b11000011, 0b00000001	//0x02
  };
  i2c_open();
  i2c_write(buffer, 2);
  i2c_close();
  return 0;
}

int RadioSeekDown(void)
{
  static unsigned char buffer[]=
  {
    0b11000001, 0b00000001	//0x02
  };
  i2c_open();
  i2c_write(buffer, 2);
  i2c_close();
  return 0;
}

int RadioFrequency(char freqtext[], unsigned char buffer[])
{
  int freq = 875  + 1 * (int)(buffer[1]);
  sprintf(freqtext, "%d.%d MHz", freq / 10, freq % 10);
  return freq;
}

int RadioStatus(void)
{
  static unsigned char buffer[2]; 
  static char freqtext[10]; 

  i2c_open();
  i2c_read(buffer, 2); // read register 0x0A
  i2c_close();
 
  RadioFrequency(freqtext, buffer); 

  if (buffer[0] & 0b01000000)
  {
    if (buffer[0] & 0b00100000)
    {
      printf("Seeking failed ! ( %s )\n", freqtext);       	
    } else {
      printf("Seeking OK ! ( %s )\n", freqtext);   
    }
  } else {
    printf("Seeking ... ( %s )\n", freqtext);   
  }
  return 0;
}

int main(int argc, char **argv)
{
  if (argc == 1) {
    RadioShowHelp();
    return(0);
  }
  for (argc--, argv++; argc > 0; argc--, argv++) {
    if (strcmp(argv[0], "--help") == 0) {
      RadioShowHelp();
      continue;
    }
    if (strcmp(argv[0], "--register") == 0) {
      RadioShowRegisters();
      continue;
    }
    if (strcmp(argv[0], "--registerall") == 0) {
      RadioShowAllRegisters();
      continue;
    }
    if (strcmp(argv[0], "--start") == 0) {
      RadioStart();
      continue;
    }
    if (strcmp(argv[0], "--stop") == 0) {
      RadioStop();
      continue;
    }
    if (strcmp(argv[0], "--seekup") == 0) {
      RadioSeekUp();
      continue;
    }
    if (strcmp(argv[0], "--seekdown") == 0) {
      RadioSeekDown();
      continue;
    }
    if (strcmp(argv[0], "--status") == 0) {
      RadioStatus();
      continue;
    }
    //if (strcmp(argv[0], "--chan") == 0) {
      //channel = atoi(argv[1]);
      //argc--;
      //argv++;
      //continue;
    //}
    printf("unknown option %s\n", argv[0]);
    return(1);
  }
  return 0;
}
