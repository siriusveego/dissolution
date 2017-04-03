#include <lpc23xx.h>


#define FAIL			0 
#define PASS			1

#define WRITEI2C		0xa0 
#define READI2C			0xa1 

#define E2PROMR			0xa0
#define E2PROMW			0xa1

#define PAGESIZE		256
#define MAXIMUM_PAGES	512

#define BMISCALE_SIGNATURE	 4321	 

void delay_ms(unsigned long a);

void init_i2c0(void);
void do_i2c0_action(void);
unsigned char read_write_page(unsigned int pageno,unsigned char rd_wr_type);
void enable_i2c_timeout(unsigned long timeout_duration);
void handle_new_eeprom(void);
void delay_ms(unsigned long a);
unsigned char wr_eeprom_page(unsigned char pageno, unsigned char *ptr);
unsigned char rd_eeprom_page(unsigned char pageno, unsigned char *ptr);

unsigned int write_nob_to_eeprom_abs(unsigned int eep_abs_loc, unsigned char *src_sram, unsigned int nob);

unsigned int read_nob_from_eeprom_abs(unsigned char *sram_dst, unsigned int eep_abs_loc, unsigned int nob);

