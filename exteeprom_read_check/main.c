/*
 * exteeprom_read_check.c
 *
 * Created: 03-02-2018 14:49:33
 * Author : hp-pc
 */ 

/*
 * uart_eeprom.c
 *
 * Created: 12-01-2018 15:07:40
 * Author : hp-pc
 */ 

#define F_CPU 3686400UL
//#define F_CPU   8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>


#define BAUD 9600
#define ubrrv (F_CPU/(16UL*BAUD))-1
#define EEPROM_ID 0xA0
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
	 // char ch;
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
	 UART_Transmit(*ptr_string++);
 }
 
////////////////////////////////////////////////////////////////////////////////////////////////
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
 	I2C_Write(0xA0);
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

int main(void)
{
	DDRD=0xff;
	DDRB=0xff;
	
	//PORTD=0x00;
unsigned char eeprom_address = 0x00, write_str[] = "SIDDHARTH PAL    SIDASTRO    SATYAM";
unsigned char read_str[100];
	//unsigned char rdchar=0;
 
  UART_Intial(ubrrv);
  UART_TxString("sidd  ");
 
// UART_Transmit( EEPROM_WriteByte(0x00,'a') );
 //UART_TxString("  write complete  ");
      //UART_Transmit(EEPROM_ReadByte(0x00));
//  	    UART_TxString("WRITE_start");
//  	  EEPROM_WriteString(eeprom_address,write_str);
//  	   	UART_TxString( write_str );
// 			UART_TxString("________________________");
		   UART_TxString("read_start");
		   
		   EEPROM_ReadString(0x01,read_str);
 	UART_TxString(read_str);

 /*
//EEPROM_WriteByte(0,0b11001100);
PORTD=EEPROM_WriteByte(0,0b11001100);
_delay_ms(1000);
PORTD=0x00;
_delay_ms(1000);
PORTD=EEPROM_ReadByte(0);
*/
while(1){

        }
  
    return 0;
}



