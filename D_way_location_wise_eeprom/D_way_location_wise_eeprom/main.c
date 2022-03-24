/*
 * D_way_location_wise_eeprom.c
 *
 * Created: 13-02-2018 11:18:41
 * Author : hp-pc
 */ 




#define F_CPU 3686400UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#define EEPROM_ID 0xA0
#define ENTER_PRESS (ch=='\n')||(ch=='\r')

#define BAUD 9600
#define ubrrv (F_CPU/(16UL*BAUD))-1


char ch;

void UART_Intial(unsigned int ubrr)
{
	UBRR0H=(unsigned char)(ubrr>>8);
	UBRR0L=(unsigned char)ubrr;
	UCSR0B=(1<<TXEN0)|(1<<RXEN0);//|(1<<RXCIE0);
	UCSR0C=(1<<USBS0)|(3<<UCSZ00);
}

void UART_Transmit(char input)
{
	while(!(UCSR0A&(1<<UDRE0)));
	UDR0=input;
}

char UART_recieve()
{
	while(!(UCSR0A&(1<<RXC0)));
	return UDR0;
	
}

uint8_t UART_RxString(char *ptr_string)
{
	 char ch;
	uint8_t len = 0;
	while(1)
	{
		ch=UART_recieve();// ch=UART_RxChar();    //Receive a char
		UART_Transmit(ch);     //Echo back the received char

		if((ch=='\r') || (ch=='\n')) //read till enter key is pressed
		{						     //once enter key is pressed null terminate the string
			ptr_string[len]=0;           //and break the loop
			break;
		}
		else if((ch=='\b') && (len!=0))
		{
			len--;    //If backspace is pressed then decrement the index to remove the old char
		}
		else
		{
			ptr_string[len]=ch; //copy the char into string and increment the index
			len++;
		}
	}
	return len;
}

void UART_TxString(char *ptr_string)
{
	while(*ptr_string)
	UART_Transmit(*ptr_string++);//UART_TxChar(*ptr_string++);// Loop through the string and transmit char by char
}
///////////////////////////////////////////////
void I2C_Start()
{
	//DDRB=0x03;//_____________

	//I2C_DATA_HI();
	PORTB=0x02;
	// _delay_ms(1);

	//I2C_CLOCK_HI();
	PORTB|=0x01;
	_delay_ms(1);
	
	//	I2C_DATA_LO();
	PORTB&=0x01;
	//_delay_ms(1);
}

void I2C_Stop(void)
{

	//PORTB=0x00;//SCL = 0;			// Pull SCL low
	//_delay_us(1);

	PORTB=0x00;//SDA = 0;			// Pull SDA  low
	_delay_us(1);

	PORTB|=0x01;//SCL = 1;			// Pull SCL High
	_delay_us(1);

	PORTB|=0x02;//SDA = 1;			// Now Pull SDA High, to generate the Stop Condition
}

void I2C_Ack(void)
{
	DDRB=0xff;
	PORTB=0x00;//SDA = 0;		//Pull SDA low to indicate Positive ACK
	PORTB|=0x01;//I2C_Clock();	//Generate the Clock
	//PORTB=0x02;//SDA = 1;		// Pull SDA back to High(IDLE state)
}

void I2C_NoAck(void)
{
	DDRB=0xff;
	PORTB=0x02;//SDA = 1;		//Pull SDA high to indicate Negative/NO ACK
	//  I2C_Clock();	    // Generate the Clock
	PORTB|=0x01;//SCL = 1;		// Set SCL */
	//return 1;
	}

void I2C_Write(unsigned char dat)
{
	unsigned char x;
	unsigned char b;
	for(x=0; x<8; x++)
	{
		if(dat & 0x80) PORTB=0x02;// SDA = 1;
		
		else PORTB=0x00;	//SDA = 0;
		_delay_ms(1);
		
		PORTB|=0x01;		//SCL = 1;
		_delay_ms(1);
		dat <<= 1;
		PORTB&=0x02;	//SCL = 0;
	}
	
	PORTB&=0x02;//SDA = 1;
	_delay_ms(1);
	//I2C_Ack();
	PORTB=0x00;//SDA = 0;		//Pull SDA low to indicate Positive ACK
	PORTB|=0x01;
	//return dat;
}


unsigned char I2C_Read(void)
{
	unsigned char i,dat=0x00;
	DDRB=0x01;//direction sda_in,scl_out 0b00000001
	//PORTB=0x02;// sda as i/p pullup high
	for (i=0;i<8;i++)
	{
		PORTB=0x02;
		_delay_us(1);
		PORTB|=0x01;//scl high
		_delay_us(1);
		dat=dat<<1;
	
		dat|=(PINB&0x02);
_delay_us(10);
		PORTB=0x02;//scl low
		
	}
	I2C_NoAck();
/*	if(!(dat==0xff))
	{
		_delay_ms(100);
		I2C_NoAck();
	}
	else
	{
		
		// eeprom_Data=I2C_Read();
		I2C_Ack();
	}
	*/
	return dat>>1;
}


unsigned char EEPROM_WriteByte(unsigned char eeprom_Address, unsigned char eeprom_Data)
{

	I2C_Start();               
	I2C_Write(EEPROM_ID);
	//I2C_Ack();
	I2C_Write(eeprom_Address); 
	//I2C_Ack();
	I2C_Write(eeprom_Data);    
	//I2C_Ack();
	I2C_Stop();           	   
	_delay_ms(5);          
	
	return eeprom_Data;
}

 void EEPROM_Erase()
 {
	 unsigned char eeprom_address;

	 for(eeprom_address=0;eeprom_address<255;eeprom_address++)
	 {
		 EEPROM_WriteByte(eeprom_address,0xff); // Write Each memory location with OxFF
	 }
 }
 
unsigned char EEPROM_ReadByte(unsigned char eeprom_Address)
{
	unsigned char eeprom_Data;

	I2C_Start();   
	//_delay_ms(100);          
 	I2C_Write(EEPROM_ID);
// 	//I2C_Ack();
	//_delay_ms(100);

 	I2C_Write(eeprom_Address); 

 	I2C_Start();	
	       
 	I2C_Write(0xA1);
	
	eeprom_Data = I2C_Read();  // Read the data from specified address

	I2C_Stop();		         
	_delay_us(10);
	return eeprom_Data;        

}

void EEPROM_WriteString(unsigned char eeprom_address, unsigned char * source_address)
{
	do
	{
		EEPROM_WriteByte(eeprom_address,*source_address);
		source_address++;
		eeprom_address++;
	}while(*(source_address-1) !=0);
}


void EEPROM_ReadString(unsigned char eeprom_address, unsigned char * destination_address)
{
	char eeprom_data;

	do
	{
		eeprom_data = EEPROM_ReadByte(eeprom_address);
		*destination_address = eeprom_data;
		destination_address++;
		eeprom_address++;
	}while(eeprom_data!=0);
}
/*
"0x00_data"


TX> memory addr for wrt:
   RX>TX> show addr
   
   TX> wrt data @ this loc
   RX>TX> "    data     " 
   
   TX> memory addr for read:
   RX>TX> show addr
   
   TX> read data @this loc
   TX> "  data  "

*/

//For Printf fxn
static int uart_putchar(char c, FILE *stream);
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,_FDEV_SETUP_WRITE);
static int uart_putchar(char c, FILE *stream)
{

	if (c == '\n')
	uart_putchar('\r', stream);
	while(!(UCSR0A&(1<<UDRE0)));
	UDR0 = c;
	return 0;
}
/*Function definition*/
int  getSubString(char *source, char *target, int from, int to)
{
	int length=0;
	int i=0,j=0;
	
	//get length
	while(source[i++]!='\0')
	length++;
	
	if(from<=0 || from>length){
		printf("Invalid \'from\' index\n");
		return 1;
	}
	if(to>length){
		printf("Invalid \'to\' index\n");
		return 1;
	}
	
	for(i=from,j=0;i<=to;i++,j++){
		target[j]=source[i];
	}
	
	//assign NULL at the end of string
	target[j]='\0';
	
	return 0;
}

int main(void)
{
	DDRB=0xff;
	UART_Intial(ubrrv);
	stdout = &mystdout;
	
//////////////////////
char text [100],len=0;
char text1[50] ;
int from,to;

	unsigned char len2=0,str,ch=0,s;
	int i,l;
	//unsigned char str_a_r[50],str_a_w[50];
	struct exteeprom
	{
		unsigned char str_a_w[50];
		unsigned char str_a_r[50];
		unsigned char strdata[50];
		unsigned char des[50];
		int m_address_w;
		int m_address_r;
	};
	struct exteeprom E[25];

	while (1)
	{
		for (int i=0;i<255;i++)
		{		
			UART_TxString("\n\r write..\n\r");
			UART_TxString("W_memory address+data\n\r");
			do
			{len=UART_RxString(text);
				UART_TxString(len);
			} while (ENTER_PRESS);//loop break after condition false i.e enter release after pressed // EXIT
			if (!ENTER_PRESS)//true when enter released
			{
				UART_TxString("\n\renter last index\n\r");
				unsigned int last[50];
				do
				{s=UART_RxString(last);
					UART_TxString(s);
				} while (ENTER_PRESS);//loop break after condition false i.e enter release after pressed // EXIT
				if (!ENTER_PRESS)//true when enter released
					 l = (int)strtol(last, NULL, 0);
   printf("%d",l);
    UART_TxString("\n\r");
				getSubString(text,text1,5,l);
				UART_TxString("\n\r write address\n\r");
				E[i].m_address_w = (int)strtol(text, NULL, 16);
				printf("%d\n",E[i].m_address_w);
			}
			//_______________________________________________________________
			UART_TxString("wrt data\n\r");
// 			do
// 			{str=UART_RxString(E[i].strdata);
// 				UART_TxString(str);
// 				
// 			} while (ENTER_PRESS);//loop break after condition false i.e enter release after pressed // EXIT
// 			if (!ENTER_PRESS)
// 			{
				EEPROM_WriteString(E[i].m_address_w,text1);
				UART_TxString(text1);
			//}
			
			/////////////////////////////////////////////////////
			UART_TxString("\n\r read..");
			UART_TxString("\n\rR_memory_address\n\r");
			do
			{len2=UART_RxString(E[i].str_a_r);
				UART_TxString(len2);

			} while (ENTER_PRESS);//loop break after condition false i.e enter release after pressed // EXIT
			if (!ENTER_PRESS)
			{
				E[i].m_address_r = (int)strtol(E[i].str_a_r, NULL, 16);
				printf("\n%d\n",E[i].m_address_r);
				UART_TxString("\n\r read_data\n\r");
				EEPROM_ReadString(E[i].m_address_r,E[i].des);
				UART_TxString(E[i].des);
				UART_TxString("\n\r");
			}
			UART_TxString("\n\rdo u want write again...");
			
		}
		
	}
}


	


