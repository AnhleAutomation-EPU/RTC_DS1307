#include <avr/io.h>
void uart_init()
{
	UCSRB = (1<<TXEN); // enable UART ACTIVE 
	UCSRC = (1<<UCSZ1)|(1<<UCSZ0)|(1<<URSEL);
	UBRRL = 0X33; // BAUDRATE 9600 
}
void uart_sendata(unsigned char data)
{
	while(!(UCSRA & (1<<UDRE))); // waiting here when UDRE == 0
	UDR = data;
}
void uart_sendString(char*s)
{
	unsigned int i = 0;
	while(s[i] !=0)
	{
		uart_sendata(s[i]);
		i = i + 1;
	}
}
void uart_bcd_packed(unsigned char data)
{
	uart_sendata('0' + (data>>4));  
	uart_sendata('0' + (data & 0x0f));
}
void I2C_INIT()
{
	TWSR = 0x00;
	TWBR = 0X47;
	TWCR = 0X04;
}
void I2C_START()
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTA);
	while(!(TWCR &(1<<TWINT))); // waiting when TWINT set
}
void I2C_STOP()
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	for(int k = 0; k<=100;k++); // short delay
}
void I2C_WRITE(unsigned char data)
{
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while(!(TWCR &(1<<TWINT)));
}
unsigned char I2C_READ(unsigned char ackVal)
{
	TWCR =(1<<TWINT)|(1<<TWEN)|(ackVal<<TWEA);
	/*
		TWEA == 1; that mean want to receaving ACK TWEA = 1
	*/
	while(!(TWCR &(1<<TWINT)));
	return TWDR;
}
void RTC_INIT()
{
	I2C_INIT();
	I2C_START();
	I2C_WRITE(0XD0);
	I2C_WRITE(0X07);
	I2C_WRITE(0X00); 
/*
	set cursor of DS1307 in 0x07 and load valued 0x00 to location 0x07
*/
	I2C_STOP();
}
void RTC_SET_TIME(unsigned char hour, unsigned char minutes, unsigned char second)
{
	I2C_START();
	I2C_WRITE(0xD0);
	I2C_WRITE(0);
	I2C_WRITE(hour);
	I2C_WRITE(minutes);
	I2C_WRITE(second);
	I2C_STOP();
}
void RTC_SET_DATE(unsigned char date, unsigned char month, unsigned char year)
{
	I2C_START();
	I2C_WRITE(0xD0);
	I2C_WRITE(0x04);
	I2C_WRITE(date);
	I2C_WRITE(month);
	I2C_WRITE(year);
	I2C_STOP();
}
void RTC_GET_TIME(unsigned char *h,unsigned char * m,unsigned char *s)
{
	I2C_START();
	I2C_WRITE(0xD0);
	I2C_WRITE(0);
	I2C_STOP();
	
	I2C_START();
	I2C_WRITE(0XD1);
	*s = I2C_READ(1);
	*m = I2C_READ(1);
	*h = I2C_READ(0);
	I2C_STOP();
}
void RTC_GET_DATE(unsigned char *d,unsigned char *m, unsigned char *y)
{
	I2C_START();
	I2C_WRITE(0xD0);
	I2C_WRITE(0x4);
	I2C_STOP();
	
	I2C_START();
	I2C_WRITE(0XD1);
	*d = I2C_READ(1);
	*m = I2C_READ(1);
	*y = I2C_READ(0);
	I2C_STOP();
}
int main(void)
{
	unsigned char x,y,z;
	RTC_INIT();
	RTC_SET_TIME(0x50,0x59,0x23); 
	/*
		Second, minute, hour
	*/
	RTC_SET_DATE(0X25,0X8,0X24);
	
	uart_init();
    while(1)
	{
		RTC_GET_TIME(&x,&y,&z);
		uart_sendString("Time: ");
		uart_bcd_packed(x);
		uart_sendata(':');
		uart_bcd_packed(y);
		uart_sendata(':');
		uart_bcd_packed(z);
		
		RTC_GET_DATE(&x,&y,&z);
		uart_sendString(" DATE: ");
		uart_bcd_packed(x);
		uart_sendata(':');
		uart_bcd_packed(y);
		uart_sendata(':');
		uart_bcd_packed(z);
		uart_sendata('\r');
		
	 for(volatile long i =0;i<200000;i++);
	}
	return 0;
}