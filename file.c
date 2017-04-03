#include "file.h"
#include "i2c-e2prom.h"


#define	MBR					0
#define	FAT_START			1 //page no
#define	FAT_END				15 //page no
#define	FIRST_REC_NO		16
#define LAST_REC_NO			255
#define	DATA_START_SECT		16
#define DATA_END_SECTOR		511

#define TOT_REC_PER_SEC		15

#define OPEN				0
#define CREATE				1

int new_file;

veego_FILE veego_file;
static unsigned char fat_sector[256];

/*####################################################
search_for_file_instance = searching of file presence.
	parameters-->
		name(char):- File name which have to open
		op_cr(int):- its for open or create
	return-->
		(int)it will return record no
			OPEN:- 
				if(file found)=existing file record no.
			CREATE:- 
				if(free record found)=next free record no.
			else
				ERR;
####################################################*/
static unsigned int search_for_file_instance(char *fname, int op_cr);

/*####################################################
extract_record_details = extracting all file data from FAT.
	parameters-->
		veego_FILE pointer
		rec_no(unsigned int):- record number
	return-->
		(int)it will return record no
			OPEN:- 
				if(file found)=existing file record no.
			CREATE:- 
				if(free record found)=next free record no.
			else
				ERR;
####################################################*/
static unsigned char extract_record_details(veego_FILE *vfptr, unsigned int rec_no);

/*####################################################
next_free_file_data_sector = search next free data location.
	parameters-->
		rec_no(int):- record number
	return-->
		(int):- it will return address of next free location
####################################################*/
static unsigned int next_free_file_data_sector(unsigned int rec_no);

/*####################################################
deci_to_str = converting dicimal to 2byte char.
	parameters-->
		deci(int):- number
		*ptr(char):- pointer of string
	return-->
		*(char):- it will return pointer of string
####################################################*/
unsigned char * deci_to_str(int deci, unsigned char* ptr);

/*####################################################
str_to_int = converting 2byte char to dicimal .
	parameters-->
		*ptr(char):- pointer of string
	return-->
		(int):- it will return decimal number
####################################################*/
int str_to_int(unsigned char * ch); //converting byte to decimal



//**********########################**************//

int f_create(char* name, bool mode, int size){
	int rec_no;
	
	rec_no = search_for_file_instance((char *)name, CREATE);
	if(rec_no){
		memset(veego_file.file_name, 0, sizeof(veego_file.file_name));
		memcpy(veego_file.file_name, name, 8);
		veego_file.file_size = size;
		veego_file.mode = mode;
		memset(ram_file_data, 0, sizeof(ram_file_data));
		veego_file.idx_ptr = ram_file_data;
		veego_file.fat_record = rec_no;
		
		veego_file.start_addr = next_free_file_data_sector(rec_no);
		new_file = 1;
		return FR_CREAT; //FILE created
	}else{
		return FR_DENIED; //space not found
	}
}


int f_format(void){
	//delete all FAT data from EEPROM
	int i,j;
	
	for(i=FAT_START; i<FAT_END+1; i++){
		// rd_eeprom_page(i,fat_sector);
		read_nob_from_eeprom_abs(fat_sector,i,sizeof(fat_sector)-16);
		for(j=0;j<TOT_REC_PER_SEC+1;j++){
			memset(&fat_sector[j*16], 0, 1);
		}
		// wr_eeprom_page(i, fat_sector);
		write_nob_to_eeprom_abs(i,fat_sector,TOT_REC_PER_SEC*16); //each record of 16Byte
	}
	return FR_FORMAT;
}


int f_putc(char c){
	int ret;
	if(veego_file.mode == WR || new_file){
		*veego_file.idx_ptr = c;
		veego_file.idx_ptr++;
		ret = FR_OK;
	}else
		ret = ERR;
	return ret;
}


int f_save(void){
	int sector_no;
	int rec_pos;
	int ret;
	
	if(new_file){
		//store DATA************************
		//check for data written correctly
		if(write_nob_to_eeprom_abs(veego_file.start_addr,ram_file_data,veego_file.file_size+1)){
			
			//read FAT ************
			sector_no = veego_file.fat_record/16;
			rec_pos = (veego_file.fat_record%16)*16;
			
			// rd_eeprom_page(sector_no,fat_sector);
			read_nob_from_eeprom_abs(fat_sector,sector_no,sizeof(fat_sector));
			
			memcpy(&fat_sector[rec_pos],veego_file.file_name,8);
			deci_to_str(veego_file.file_size,&fat_sector[rec_pos+9]);
			deci_to_str(veego_file.start_addr,&fat_sector[rec_pos+11]);
			
			fat_sector[rec_pos+15] |= 0x01&veego_file.mode;
			
			//write FAT record
			// wr_eeprom_page(sector_no,fat_sector);
			write_nob_to_eeprom_abs(sector_no,fat_sector,sizeof(fat_sector));
			
			new_file = 0;
			ret = FR_SAVE;
		}else
			ret = ERR;
		
	}else if(veego_file.mode == WR){
		//store DATA*****************
		if(write_nob_to_eeprom_abs(veego_file.start_addr,ram_file_data,veego_file.file_size+1))			
			ret = FR_SAVE;
		else
			ret = ERR;
	}else
		ret = ERR;
	
	return ret;
}

int f_open(const char* name){
	int rec_addr;
	
	rec_addr = search_for_file_instance((char *)name, OPEN);
	if(rec_addr){
		extract_record_details(&veego_file, rec_addr);
		
		memset(ram_file_data,0,sizeof(ram_file_data));
		read_nob_from_eeprom_abs(ram_file_data,veego_file.start_addr,veego_file.file_size);
		veego_file.idx_ptr = ram_file_data; //RAM location
		return FR_OK; //FILE opened
	}else
		return FR_NO_FILE;
}


int f_close(void){
	veego_file.mode = 0; //read/write mode copy to ptr
	veego_file.file_size = 0; //totol file size
	veego_file.start_addr = 0; //starting addr of file located in eeprom
	veego_file.idx_ptr = 0;
	veego_file.fat_record = 0;
	
	return FR_OK;
}

int f_seek(unsigned int ofs){
	
	if(veego_file.file_size > ofs){
		veego_file.idx_ptr = &ram_file_data[ofs];
		return FR_OK;
	}else
		return ERR;
}


char f_getc(void){
	char c;
	// if(*veego_file.idx_ptr < veego_file.file_size){
		c = *veego_file.idx_ptr;
		veego_file.idx_ptr++;
		return c;
	// }else
		// return 0;
}

unsigned char* f_rewind(void){
	//RAM location starting addr need to be store
	veego_file.idx_ptr = ram_file_data;
	return veego_file.idx_ptr;
}



/*####################################################
			utility functions
####################################################*/

unsigned char * deci_to_str(int deci, unsigned char* ptr){
	*ptr = deci%256;
	deci -= *ptr;
	ptr--;
	*ptr = deci/256;
	return ptr;
}

int str_to_int(unsigned char * ch){
    int d;
    d = (256*(*ch));
    ch++;
    d += *ch;
    return d;
}

// Returns 0 for fail or serial number of record.
static unsigned int search_for_file_instance(char *fname, int op_cr)
{
	unsigned char search_start = 1;
	unsigned int ch;
	
	for(ch = FIRST_REC_NO; ch < (LAST_REC_NO + 1); ch++)
	{
		if(search_start || !(ch % 16))
		{
			// move the record sector data in search array;
			// rd_eeprom_page((ch / 16), fat_sector);
			read_nob_from_eeprom_abs(fat_sector,(ch / 16),sizeof(fat_sector)-16);
			search_start = 0;
		}
		
		if(op_cr == OPEN){
			if(strcmp(fname, &fat_sector[(ch % 16) * 16]) == 0)
				return ch; //file present
		}else if(op_cr == CREATE){
			if(strcmp((const char*)fname, &fat_sector[(ch % 16) * 16]) == 0)
				return ERR; //file present
			if(fat_sector[(ch % 16) * 16] == '\0')
				return ch;	// Free record found here
		}
	}
	return(ERR);// No file found.
}

static unsigned char extract_record_details(veego_FILE *vfptr, unsigned int rec_no)
{
	// move the record sector data in search array;
	// rd_eeprom_page((rec_no / 16), fat_sector);
	read_nob_from_eeprom_abs(fat_sector,(rec_no / 16),sizeof(fat_sector));
	strcpy(vfptr->file_name,&fat_sector[(rec_no % 16) * 16]);
	vfptr->file_size = str_to_int(&fat_sector[(rec_no % 16) * 16 + 8]);
	vfptr->start_addr = str_to_int(&fat_sector[(rec_no % 16) * 16 + 10]);
	// vfptr.atbr = fat_sector[(rec_no % 16) * 16 + 7];
	vfptr->mode = 0x01 & fat_sector[((rec_no % 16) * 16) + 15]; //LSB
	
	//return ?????????
}

// Returns 0 for fail or serial number of record.
static unsigned int next_free_file_data_sector(unsigned int rec_no)
{
	unsigned int reqd_sector;
	unsigned int rec_pos;
	unsigned int size, ch;
	
	ch = rec_no;
	if(ch == 0)
	{
		// File system index is full you may return the free sector in data area 
		// if you desire or return 0 otherwise. "TO BE DECIDED"
	}
	else
	{
		if(ch == FIRST_REC_NO)
			return DATA_START_SECT;
		else if(ch%16 == 0){
			rec_pos = 16*14; //last record position
			// rd_eeprom_page((ch/16)-1, fat_sector); //rd previous sector
			read_nob_from_eeprom_abs(fat_sector,(ch / 16)-1,sizeof(fat_sector)-16);
			reqd_sector = str_to_int(&fat_sector[rec_pos + 10]);
			size = str_to_int(&fat_sector[rec_pos + 8]);
			reqd_sector += (size/256);
			if(size%256) reqd_sector++;
			return reqd_sector;
		}else{
			rec_pos = ((ch%16)-1)*16; //previous record position
			reqd_sector = str_to_int(&fat_sector[rec_pos + 10]);
			size = str_to_int(&fat_sector[rec_pos + 8]);
			reqd_sector += (size/256);
			if(size%256) reqd_sector++;
			return reqd_sector;
		}
	}
}

/*####################################################
####################################################*/




