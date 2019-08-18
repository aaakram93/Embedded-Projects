 /******************************************************************************
 *
 * Module: LCD
 *
 * File Name: lcd.c
 *
 * Description: Source file for the LCD driver
 *
 * Author: Ahmed Akram
 *
 *******************************************************************************/

#include "LCD.h"
#include "PLL.h"


/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/
 void PortD_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000008;     // 1) D clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTD_LOCK_R = 0x4C4F434B;   // 2) unlock PortD PF0  
  GPIO_PORTD_CR_R = 0x0F;           // allow changes to PD3-0       
  GPIO_PORTD_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTD_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTD_DIR_R = 0x0F;          // 5) PD3-0 output   
  GPIO_PORTD_AFSEL_R = 0x00;        // 6) no alternate function     
  GPIO_PORTD_DEN_R = 0x0F;          // 7) enable digital pins PD3-0       
}
 
 void PortA_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000001;     // 1) A clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTA_LOCK_R = 0x4C4F434B;   // 2) unlock PortA PA2-4  
 // GPIO_PORTC_CR_R |= 0x1C;           // allow changes to PA2-4       
  GPIO_PORTA_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTA_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTA_DIR_R |= 0x1C;          // 5) PA2-4  output   
  GPIO_PORTA_AFSEL_R = 0x00;        // 6) no alternate function     
  GPIO_PORTA_DEN_R = 0x1C;          // 7) enable digital pins PA2-4         
}
 
void Delay(void){unsigned long volatile time;
  time = 16129; // 1 ms; 
  while(time){
   time--;
  }
}

void LCD_init(void)
{
	LCD_CTRL_PORT_DIR |= (1<<E) | (1<<RS) | (1<<RW); /* Configure the control pins(E,RS,RW) as output pins */
	
	#if (DATA_BITS_MODE == 4)
		#ifdef UPPER_PORT_PINS
			LCD_DATA_PORT_DIR |= 0xF0; /* Configure the highest 4 bits of the data port as output pins */
		#else
			LCD_DATA_PORT_DIR |= 0x0F; /* Configure the lowest 4 bits of the data port as output pins */
		#endif		 
		LCD_sendCommand(FOUR_BITS_DATA_MODE); /* initialize LCD in 4-bit mode */
		LCD_sendCommand(TWO_LINE_LCD_Four_BIT_MODE); /* use 2-line lcd + 4-bit Data Mode + 5*7 dot display Mode */
	#elif (DATA_BITS_MODE == 8)
		LCD_DATA_PORT_DIR = 0xFF; /* Configure the data port as output port */ 
		LCD_sendCommand(TWO_LINE_LCD_Eight_BIT_MODE); /* use 2-line lcd + 8-bit Data Mode + 5*7 dot display Mode */
	#endif
	
	LCD_sendCommand(CURSOR_OFF); /* cursor off */
	LCD_sendCommand(CLEAR_COMMAND); /* clear LCD at the beginning */
}

void LCD_sendCommand(uint8 command)
{
	CLEAR_BIT(LCD_CTRL_PORT,RS); /* Instruction Mode RS=0 */
	CLEAR_BIT(LCD_CTRL_PORT,RW); /* write data to LCD so RW=0 */
	Delay(); /* delay for processing Tas = 50ns */
	SET_BIT(LCD_CTRL_PORT,E); /* Enable LCD E=1 */
	Delay(); /* delay for processing Tpw - Tdws = 190ns */
#if (DATA_BITS_MODE == 4)
	/* out the highest 4 bits of the required command to the data bus D4 --> D7 */
#ifdef UPPER_PORT_PINS
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (command & 0xF0);
#else
	LCD_DATA_PORT = (LCD_DATA_PORT & 0xF0) | ((command & 0xF0) >> 4);
#endif

	Delay(); /* delay for processing Tdsw = 100ns */
	CLEAR_BIT(LCD_CTRL_PORT,E); /* disable LCD E=0 */
	Delay(); /* delay for processing Th = 13ns */
	SET_BIT(LCD_CTRL_PORT,E); /* Enable LCD E=1 */
	Delay(); /* delay for processing Tpw - Tdws = 190ns */

	/* out the lowest 4 bits of the required command to the data bus D4 --> D7 */
#ifdef UPPER_PORT_PINS
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | ((command & 0x0F) << 4);
#else
	LCD_DATA_PORT = (LCD_DATA_PORT & 0xF0) | (command & 0x0F);
#endif

	Delay(); /* delay for processing Tdsw = 100ns */
	CLEAR_BIT(LCD_CTRL_PORT,E); /* disable LCD E=0 */
	Delay(); /* delay for processing Th = 13ns */
#elif (DATA_BITS_MODE == 8)
	LCD_DATA_PORT = command; /* out the required command to the data bus D0 --> D7 */
	Delay(); /* delay for processing Tdsw = 100ns */
	CLEAR_BIT(LCD_CTRL_PORT,E); /* disable LCD E=0 */
	Delay(); /* delay for processing Th = 13ns */
#endif
}

void LCD_displayCharacter(uint8 data)
{
	SET_BIT(LCD_CTRL_PORT,RS); /* Data Mode RS=1 */
	CLEAR_BIT(LCD_CTRL_PORT,RW); /* write data to LCD so RW=0 */
	Delay(); /* delay for processing Tas = 50ns */
	SET_BIT(LCD_CTRL_PORT,E); /* Enable LCD E=1 */
	Delay(); /* delay for processing Tpw - Tdws = 190ns */
#if (DATA_BITS_MODE == 4)
	/* out the highest 4 bits of the required data to the data bus D4 --> D7 */
#ifdef UPPER_PORT_PINS
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (data & 0xF0);
#else
	LCD_DATA_PORT = (LCD_DATA_PORT & 0xF0) | ((data & 0xF0) >> 4);
#endif

	Delay(); /* delay for processing Tdsw = 100ns */
	CLEAR_BIT(LCD_CTRL_PORT,E); /* disable LCD E=0 */
	Delay(); /* delay for processing Th = 13ns */
	SET_BIT(LCD_CTRL_PORT,E); /* Enable LCD E=1 */
	Delay(); /* delay for processing Tpw - Tdws = 190ns */

	/* out the lowest 4 bits of the required data to the data bus D4 --> D7 */
#ifdef UPPER_PORT_PINS
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | ((data & 0x0F) << 4);
#else
	LCD_DATA_PORT = (LCD_DATA_PORT & 0xF0) | (data & 0x0F);
#endif

	Delay(); /* delay for processing Tdsw = 100ns */
	CLEAR_BIT(LCD_CTRL_PORT,E); /* disable LCD E=0 */
	Delay(); /* delay for processing Th = 13ns */
#elif (DATA_BITS_MODE == 8)
	LCD_DATA_PORT = command; /* out the required command to the data bus D0 --> D7 */
	Delay(); /* delay for processing Tdsw = 100ns */
	CLEAR_BIT(LCD_CTRL_PORT,E); /* disable LCD E=0 */
	Delay(); /* delay for processing Th = 13ns */
#endif
}

void LCD_displayString(const char *Str)
{
	uint8 i = 0;
	while(Str[i] != '\0')
	{
		LCD_displayCharacter(Str[i]);
		i++;
	}
	/***************** Another Method ***********************
	while((*Str) != '\0')
	{
		LCD_displayCharacter(*Str);
		Str++;
	}		
	*********************************************************/
}

void LCD_goToRowColumn(uint8 row,uint8 col)
{
	uint8 Address;
	
	/* first of all calculate the required address */
	switch(row)
	{
		case 0:
				Address=col;
				break;
		case 1:
				Address=col+0x40;
				break;
		case 2:
				Address=col+0x10;
				break;
		case 3:
				Address=col+0x50;
				break;
	}					
	/* to write to a specific address in the LCD 
	 * we need to apply the corresponding command 0b10000000+Address */
	LCD_sendCommand(Address | SET_CURSOR_LOCATION); 
}

void LCD_displayStringRowColumn(uint8 row,uint8 col,const char *Str)
{
	LCD_goToRowColumn(row,col); /* go to to the required LCD position */
	LCD_displayString(Str); /* display the string */
}

void LCD_intgerToString(int data)
{
   char buff[16]; /* String to hold the ascii result */
	 char temp;
	 char ascii = 48;
	 char count = 1;
	 char count2 = 0;
	while(data)
	{
	 temp = data % 10;
	 ascii += (temp * count);
	 data = data /10;
	 buff[count2] = ascii;
	 count2 ++;	
	 count *= 10;
	}
    
   LCD_displayString(buff);
}



void LCD_clearScreen(void)
{
	LCD_sendCommand(CLEAR_COMMAND); //clear display 
}
