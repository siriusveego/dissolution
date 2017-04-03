#include <LPC23xx.h>
#include <stdlib.h>
//#include <string.h>	
//#include <stdio.h>
//#include <math.h>
#include "i2c-e2prom.h"
#include "file.h"
//#include "main.h"

unsigned char ram_file_data[1024];
// unsigned char wdata[PAGESIZE];
// unsigned char rdata[PAGESIZE];
char temp_data[512];


int main(void){
	
	char str[24];
	unsigned char read_data[256], *ptr;
	int num, temp;
	
	temp = 1;
	
	read_data[0] = 0x01;
	read_data[1] = 0x02;
	read_data[2] = 0x03;
	read_data[3] = 0x04;
	
	ptr = read_data;
	num = *(unsigned char*)ptr;
	num = *(unsigned short*)ptr;
	num = *(unsigned int*)ptr;

	
	init_i2c0();
	while(1){
		temp = f_format();
//*****************//
		num = f_create("temp1", WR, 25);
		f_putc('t');
		f_putc('e');
		f_putc('u');
		f_putc('j');
		num = f_save();
		f_close();
//*****************//
		
		num = f_create("temp2", RD, 350);
		memset(temp_data, 80, 325);
		for(temp=0; temp<strlen(temp_data); temp++){
			num=f_putc(temp_data[temp]);
		}
		num = f_save();
		num = f_close();
//*****************//
		
		num = f_open("temp1");
		f_close();
		num = f_open("temp2");
		f_close();
		num = f_open("temp3");
		f_close();
		num = num+1;
	}
	return 1;
}


#if 0		
		temp = f_format();
//*****************//
		f_create("temp1", WR, 25);
		
		f_putc('t');
		f_putc('e');
		f_putc('u');
		f_putc('j');
		num = f_save();
		f_close();
//*****************//
		
		num = f_open("temp1");
		sprintf(str,"Hello VEEGO %d", num);
		for(temp=0; temp<strlen(str); temp++){
			num=f_putc(str[temp]);
		}
		num = f_save();
		num = f_close();
//*****************//
		
		num = f_create("temp2", RD, 350);
		memset(temp_data, 80, 325);
		for(temp=0; temp<strlen(temp_data); temp++){
			num=f_putc(temp_data[temp]);
		}
		num = f_save();
		num = f_close();
		
//*****************//
		
		num = f_open("temp1");
		num = f_seek(6);
		sprintf(str,"buddy %d", num);
		for(temp=0; temp<strlen(str)+1; temp++){
			f_putc(str[temp]);
		}
		num = f_save();
		f_close();
//*****************//
		num = f_open("temp2");
		num = f_close();
		//rd_eeprom_page(17,read_data);
		//rd_eeprom_page(18,read_data);
//*****************//
		
		num = f_open("temp3");
		
		rd_eeprom_page(0,read_data);
		for(temp=16; temp<20;temp++){
			rd_eeprom_page(temp,read_data);
			num = temp;
		}
#endif