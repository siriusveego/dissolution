#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define WR	1
#define RD	0

extern unsigned char ram_file_data[1024]; //this is global

enum RETRUN{
	ERR = 0,
	FR_OK = 1,
	FR_EXIST = 2,
	FR_CREAT = 3,
	FR_SAVE = 4,
	FR_NO_FILE = 5,
	FR_DENIED = 6,
	FR_FORMAT = 7,
};

typedef struct
{
	bool mode; //(file is for read or write)
	char file_name[8]; //(7 character for file name)
	unsigned char *idx_ptr; //(current RAM location of open file.)	
	
	unsigned int file_size; //(total size of file)
	unsigned int start_addr; //(sector starting address of open file.)

	unsigned int fat_record; //(FAT record number)
	
}veego_FILE;


/*####################################################
f_create = create new non existing file.
	parameters-->
		name(char):- File name to be open
		mode(bool):- read/write/create
		size (int):- Size of file in byte
	return-->
		(int)it will return result of fuction execution
####################################################*/
int f_create(char* name, bool mode, int size);

/*####################################################
f_format = deletes FAT from EEPROM..
	parameters-->
		void
	
	return-->
		(int)it will return result of fuction execution
####################################################*/
int f_format(void);

/*####################################################
f_putc = puts a character to the file.
	parameters-->
		c:- A character to be put.
	
	return-->
		When the character was written successfuly, it returns number of characters written.
####################################################*/
int f_putc(char c);


/*####################################################
f_save = copy the opened RAM file to EEPROM.
	parameters-->
		void
	
	return-->
		When file save successfully, it returns FR_OK.
####################################################*/
int f_save(void);

/*####################################################
f_open = file name will be search in eeprom and 
		if present, 
			it will copy to RAM.
		else
			NULL will be return
		
	parameters--> 
		
		name(char):- File name to be open
		mode(bool):- read/write/create

	return-->
		(int)it will return result of fuction execution
####################################################*/
int f_open(const char* name);

/*####################################################
f_close = open file will be close, all ptr parameter get erase.
	parameters-->
		void
		
	return-->
		(int)it will return result of fuction execution
####################################################*/
int f_close(void);

/*####################################################
f_seek = opened file idx_ptr will be shift by offset given
	parameters-->
		ofs:- Byte offset from top of the file to set read/write pointer.
		
	return-->
		(int)it will return result of fuction execution
####################################################*/
int f_seek(unsigned int ofs);

/*####################################################
f_gets = reads a string from the file.
	parameters-->
			void
	return-->
		When the function succeeded, char  will return.
####################################################*/
char f_getc(void);

/*####################################################
f_rewind = open file index pointer will move to starting addr.
	parameters-->
		void
	
	return-->
		starting address of RAM location
####################################################*/
unsigned char* f_rewind(void);




