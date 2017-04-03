#include "i2c-e2prom.h"	

#include <string.h>


//===================  diagnosis mode =======================================

#include <stdio.h>
#include <string.h>
//#include "timers.h"
//#include "lcd.h"

unsigned char tenms_counter=0;
unsigned char i2c_states_sts=0;

extern unsigned char MSEC10F_A;
extern char num[8];
extern unsigned char buzz_on_time;

//===========================================================================	
unsigned char i2c0_timeout_f;

unsigned char sla,cmdwrd1,cmdwrd0,i2c_direction,i2c0rdy_f;
static unsigned char *wptr, *rptr;

unsigned char rdata[PAGESIZE],wdata[PAGESIZE];

/*#####################*/
static unsigned int new_crc(volatile unsigned char *dat, unsigned short length);

static unsigned int append_CRC16(volatile unsigned char *data, unsigned short length);

static unsigned int checkCRC(volatile unsigned char *data, unsigned short length);
/*#####################*/


void init_i2c0(void)
{
 PINSEL1 |= 0x01400000;	      // SDA0 N SCL0 R USED ON P0.27 & P0.28
 I20SCLL = I20SCLH = 300;     // When pclk = 12 MHz this gives i2c speed of approx 20 khz. 
 
 I20CONSET = 0x40;            // Enable i2c interface no 0
}

//void do_i2c0_action(void)
//{
//	static unsigned short n,nob;
//
//    if(I20STAT == 0x08)  // START condition has been transmitted .
//    {
//     I20DAT = sla;
//     I20CONCLR = 0x28;   // Clear START bit and SI( i2c interrupt flag ) bit.
//
//	 i2c_states_sts=1;
//     return;
//    }
//
//    if(I20STAT == 0x18)   // SLA Address has been transmitted & ACK has been received . 
//    {
//     I20DAT = cmdwrd1;
//     I20CONCLR = 0x8;     // clear SI ( i2c interrupt flag ) 
//     n = 0;
//
//	 i2c_states_sts=2;
//     return;
//    }
//   
//    if((I20STAT == 0x28) && (n < 1))	// Data Byte in I2DAT has been transmitted & 
//    {									// ACK has been Received .
//     I20DAT = cmdwrd0;
//     n = 1;
//     nob = 0;
//     I20CONCLR = 0x8;                  // Clear SI bit.
//
//	 i2c_states_sts=3;
//     return;
//    }
//	
//    if((I20STAT == 0x28) && (n == 1) && (i2c_direction == WRITEI2C))
//    {
//        if(nob < PAGESIZE)
//	    {
//          I20DAT = wdata[nob]; 
//	    }
//        else
//        {	 		
//          i2c0rdy_f = 1;
//          // STO is set
//          I20CONSET = 0x10;
//        }
//        
//		nob++;
//        
//		I20CONCLR = 0x8;      //Clear  SI bit.	   
//	    i2c_states_sts=4;	           
//		return;
//    }
//   
//    if((I20STAT == 0x28) && (n == 1) && (i2c_direction == READI2C))
//    {
//     
//     I20CONSET = 0x20;	   // Issue repeat start
//     I20CONCLR = 0x8;     // Clear SI bit.
//
//	 i2c_states_sts=5;
//     return;
//    }
//   
//    if(I20STAT == 0x10)	// Repetated START condition has been transmitted .
//    {
//     I20DAT = sla + 1;
//     I20CONCLR = 0x28;	    // Clear START bit and SI( i2c interrupt flag ) bit.
//
//	 i2c_states_sts=6;
//     return;
//    }
//    
//	if(I20STAT == 0x40)	// SLA + READ is sent & ack is received. 
//    {
//     nob = 0;
//     I20CONSET = 0x04;	    // set ACK bit 
//     I20CONCLR = 0x08;		// clear SI bit
//
//	 i2c_states_sts=7;
//     return;
//    }
//   
//    if(I20STAT == 0x50)	 // Data byte has been received & ACK returned .
//    {
//     rdata[nob] = I20DAT;
//     nob++;
//     
//	 if(nob == (PAGESIZE-1))		
//        I20CONCLR = 0x0C;	 // clear ACK bit & clear SI bit 
//     else
//        I20CONCLR = 0x08;	 // clear SI bit
//
//	 i2c_states_sts=8;
//     return;
//    }
//	
//    if(I20STAT == 0x58)	 // Data byte has been received & NOT ACK returned .
//    {
//     rdata[nob] = I20DAT;
//     i2c0rdy_f  = 1;
//     I20CONSET = 0x10;		 // stop condition flag
//     I20CONCLR = 0x08;		 // clear SI bit	
//
//	 i2c_states_sts=9;
//     return;
//    }  
//	
//	if(I20STAT == 0x00)               // I2C bus error state 
//	{
//	  I20CONSET = 0x10;		       // stop condition flag
//      I20CONCLR = 0x08;			   // clear SI bit
//	  
//	  i2c_states_sts=10;				   
//	  return ;	
//	}
//}
//
//// read_write_page routine takes approx 3.8 Msec to read a page of 32 bytes.
//// and takes aprox 3.6 msec for writing a page of 32 bytes + 12 msec Extra for internal flash writing delay..
//
//unsigned char read_write_page(unsigned int pageno,unsigned char rd_wr_type)
//{
// unsigned char ch;
// unsigned int x;
// 
// i2c0rdy_f = 0;
//
// if(pageno > 255)
//   sla = 0xA2;
// else
//   sla = 0xA0;
// 
// //sla = 0xA0;	  
//  
//
// if(rd_wr_type == READI2C)
// {		  	
//    i2c_direction = READI2C; 
// }
// else
// {	
//    i2c_direction = WRITEI2C; 	
// }
//
// x = pageno * PAGESIZE;
//
// cmdwrd1 = x/256;
// cmdwrd0 = x%256;
//
// i2c_direction = rd_wr_type;            // 1 Read, 0 Write.
// I20CONSET = 0x20;                     // issue STA pulse.and start communication.
//
// tenms_counter = 0;  
//	
//	while(1)
//    {
//        if(i2c0rdy_f)
//	    {
//	      break;
//	    }
//
//        ch = I20CONSET;
//
//        if(ch & 0x08)
//		{
//		 do_i2c0_action();
//		}
//	}
//
//   if(rd_wr_type == WRITEI2C && i2c0rdy_f)
//     delay_ms(10);		     								
//
//   return(i2c0rdy_f);
//}

void do_i2c0_action(void)
{
//    static unsigned char n,nob;
	static unsigned short n,nob;

    // Start bit STA is sent

    if(I20STAT == 0x08)
    {
    I20DAT = sla;
    I20CONCLR = 0x28;  //Clear STA bit and SI bit.
    return;
    }
   
    // Slave address + WR is sent and also acknowlege recd.

    if(I20STAT == 0x18)
    {
    I20DAT = cmdwrd1;
    I20CONCLR = 0x8;  //Clear STA bit and SI bit.
    n = 0;
    return;
    }
   
    if((I20STAT == 0x28) && (n < 1))
    {
    I20DAT = cmdwrd0;
    n = 1;
    nob = 0;
    I20CONCLR = 0x8;  //Clear STA bit and SI bit.
    return;
    }
	
    if((I20STAT == 0x28) && (n == 1) && (i2c_direction == WRITEI2C))
    {
    if(nob < PAGESIZE)
	  {
        //I2C0DAT = wdata[nob]; 
		I20DAT = *wptr;
		wptr++;
	  }
    else
        {
		
        i2c0rdy_f = 1;
        // STO is set
        I20CONSET = 0x10;
        }
    nob++;
	//wptr++;
    I20CONCLR = 0x8;  //Clear STA bit and SI bit.
    return;
    }
   
    if((I20STAT == 0x28) && (n == 1) && (i2c_direction == READI2C))
    {
    // Issue repeat start
    I20CONSET = 0x20;
    I20CONCLR = 0x8;  //Clear STA bit and SI bit.
    return;
    }
   
    if(I20STAT == 0x10)
    {
    I20DAT = sla + 1;
    I20CONCLR = 0x28;
    return;
    }
  
    // SLA + READ is sent ack is received. 
    
	if(I20STAT == 0x40)
    {
    nob = 0;
    I20CONSET = 0x04;
    I20CONCLR = 0x08;
    return;
    }
   
    if(I20STAT == 0x50)
    {
    //rdata[nob] = I2C0DAT;
	*rptr = I20DAT;
    nob++;
	rptr++;
    if(nob == (PAGESIZE-1))		// 31 
        I20CONCLR = 0x0C;
    else
        I20CONCLR = 0x08;
    return;
    }
	
    if(I20STAT == 0x58)
    {
    //rdata[nob] = I2C0DAT;
	*rptr = I20DAT;
    i2c0rdy_f = 1;
    I20CONSET = 0x10;
    I20CONCLR = 0x08;
    return;
    } 
	
	if(I20STAT == 0x00)               // I2C bus error state 
	{
	  I20CONSET = 0x10;		       // stop condition flag
      I20CONCLR = 0x08;			   // clear SI bit				   
	  return ;	
	} 
}



// read_write_page routine takes approx 3.8 Msec to read a page of 32 bytes.
// and takes aprox 3.6 msec for writing a page of 32 bytes + 12 msec Extra for internal flash writing delay..

unsigned char read_write_page(unsigned int pageno,unsigned char rd_wr_type)
{
 unsigned char ch;
 unsigned int x;

 i2c0rdy_f = 0;

 if(pageno > 255)
   sla = 0xA2;
 else
   sla = 0xA0;	  
  

 if(rd_wr_type == READI2C)
 {		  	
    i2c_direction = READI2C; 
 }
 else
 {	
    i2c_direction = WRITEI2C; 	
 }

 x = pageno * PAGESIZE;
 cmdwrd1 = x/256;
 cmdwrd0 = x%256;

 i2c_direction = rd_wr_type;            // 1 Read, 0 Write.
 I20CONSET = 0x20;                     // issue STA pulse.and start communication.

    while(1)
    {
      if(i2c0rdy_f)
	  {
	    break;
	  }

        ch = I20CONSET;

        if(ch & 0x08)
		{
		 do_i2c0_action();
		}
	}

   if(rd_wr_type == WRITEI2C && i2c0rdy_f)
     delay_ms(12);	

   return(i2c0rdy_f);
}

//void enable_i2c_timeout(unsigned long timeout_duration)
//{
// i2c0_timeout_f = 0;
// // load timer1 channel 0 with current counts + timeout_duration
// T1MR0 = timeout_duration * 10;
// T1MR0 += T1TC;
// // Enable timer1 channel 0 interupt
// T1MCR |= 0x01;      //0000 00x 00x 00x 00x  Enable int for match1 channel x only
//}



unsigned char wr_eeprom_page(unsigned char pageno, unsigned char *ptr)
{
unsigned char ch;
unsigned int x;
wptr = ptr;


 i2c0rdy_f = 0;

 if(pageno > 255)
   sla = 0xA2;
 else
   sla = 0xA0;
   	  
i2c_direction = WRITEI2C; 	

 x = pageno * PAGESIZE;
 cmdwrd1 = x/256;
 cmdwrd0 = x%256;

 I20CONSET = 0x20;                     // issue STA pulse.and start communication.

    while(1)
    {
      if(i2c0rdy_f)
	  {
	    break;
	  }

        ch = I20CONSET;

        if(ch & 0x08)
		{
		 do_i2c0_action();
		}
	}

   if(i2c_direction == WRITEI2C && i2c0rdy_f)
     delay_ms(12);	

   return(i2c0rdy_f);
}


unsigned char rd_eeprom_page(unsigned char pageno, unsigned char *ptr)
{
unsigned char ch;
unsigned int x;

rptr = ptr;
i2c0rdy_f = 0;

 if(pageno > 255)
   sla = 0xA2;
 else
   sla = 0xA0;
   	  
i2c_direction = READI2C; 	

 x = pageno * PAGESIZE;
 cmdwrd1 = x/256;
 cmdwrd0 = x%256;

 I20CONSET = 0x20;                     // issue STA pulse.and start communication.

    while(1)
    {
      if(i2c0rdy_f)
	  {
	    break;
	  }

        ch = I20CONSET;

        if(ch & 0x08)
		{
		 do_i2c0_action();
		}
	}

   if(i2c_direction == WRITEI2C && i2c0rdy_f)
     delay_ms(12);	

   return(i2c0rdy_f);
}




void delay_ms(unsigned long a)      // Delay of 1.000 Mili.Sec per count passed in parameter.
{
  unsigned long n;

  n = a * 9600 ;	                
  while(n-- != 0);
}

void enable_i2c_timeout(unsigned long timeout_duration)
{
 i2c0_timeout_f = 0;
 // load timer1 channel 0 with current counts + timeout_duration
 T1MR0 = timeout_duration * 10;
 T1MR0 += T1TC;
 // Enable timer1 channel 0 interupt
 T1MCR |= 0x01;      //0000 00x 00x 00x 00x  Enable int for match1 channel x only
}


//****Write no. of bytes to eeprom****
unsigned int write_nob_to_eeprom_abs(unsigned int eep_abs_loc, unsigned char *src_sram, unsigned int nob)
{
	unsigned char start_page, last_page, cur_page;
	unsigned int ch, sts;
	
	append_CRC16(src_sram, nob);
	nob = nob+2; //CRC is added
	
	start_page = eep_abs_loc; //this would be page no
	last_page = start_page + (nob / 256);
	if(nob % 256) last_page++;
	
	for(cur_page = start_page; cur_page < last_page; cur_page++){
		memset(wdata, 0, sizeof(wdata));
		for(ch = 0; ch < 256; ch++){
			if(*src_sram  == 0) break;
			wdata[ch] = *src_sram;
			src_sram++;
			nob--;
			if(!nob)
				break;
			if(ch == 256)
				break;
		}

		wr_eeprom_page(cur_page,wdata);
		
		rd_eeprom_page(cur_page,rdata);

		sts = PASS;
		for(ch = 0; ch < 256; ch++)
		{
			if(rdata[ch] != wdata[ch])
			{
				sts = FAIL;
				break;
			}
		}
		if(sts == FAIL)
		{
			// sprintf(error_trap_str,"wr fl pg %03d",cur_page);
			break;
		}
		if(!nob)
			break;
	}
	
	return(sts);
}


//****Read no. of bytes from eeprom****
unsigned int read_nob_from_eeprom_abs(unsigned char *sram_dst, unsigned int eep_abs_loc, unsigned int nob)
{
	unsigned char start_page, last_page, cur_page;
	unsigned int ch, sts;
	
	nob = nob+2; //for CRC
	
	start_page = eep_abs_loc; //page no
	last_page = start_page + (nob/256);
	if(nob % 256) last_page++;
	
	for(cur_page = start_page; cur_page < last_page; cur_page++)
	{
		rd_eeprom_page(cur_page,rdata);
		
		for(ch = 0; ch < 256; ch++)
		{
			*sram_dst = rdata[ch];
			sram_dst++;
			nob--;
			if(!nob)
				return(PASS);
			if(ch == 256)
				break;
		}
	}
	
	if(checkCRC(sram_dst-nob, nob-2) == FAIL)
		return(FAIL);
	
return(sts);
}

static unsigned int checkCRC(volatile unsigned char *data, unsigned short length)
{
	unsigned int temp1, temp2;
	
	temp1 = new_crc(data, length);
	temp2 = ((unsigned int)data[length + 1] << 8);
	temp2 += data[length];
	if(temp1 == temp2)
		return(1);
	else
		return(0);
}

static unsigned int append_CRC16(volatile unsigned char *data, unsigned short length)
{
	unsigned int crc_now;
	
	crc_now = new_crc(data, length);
	data[length] = (unsigned char)(crc_now & 0x00FF);
	data[length + 1] = (unsigned char)((crc_now & 0xFF00) >> 8);
	return(crc_now);
}


static unsigned int new_crc(volatile unsigned char *dat, unsigned short length)
{
	const unsigned int CRC_POLY = 0xA001;
	int j;
	unsigned int reg_crc=0xffff;
	// printf("\nlen %d",length);
	while(length--)
		{
			reg_crc = reg_crc ^ (*dat);
			dat++;
			for(j=0;j<8;j++)
				{
				if(reg_crc & 0x01)		//check if lsb=1
					reg_crc=(reg_crc >> 1) ^ CRC_POLY;
				else
					reg_crc=reg_crc >> 1;
				}
	   }
	return(reg_crc);
}




