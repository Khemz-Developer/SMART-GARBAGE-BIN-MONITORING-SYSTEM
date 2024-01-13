#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>



#define lcd_port PORTD
#define lcd_data_dir DDRD
#define rs PD2
#define en PD3

#define US_PORT PORTC
#define US_PIN PINC
#define US_DDR DDRC

#define US_TRIG_BIN1 PC0
#define US_ECHO_BIN1 PC1

#define US_TRIG_BIN2 PC2
#define US_ECHO_BIN2 PC3

#define lcd_bin1_row 1
#define lcd_bin2_row 2

#define US_ERROR -1
#define US_NO_OBSTACLE -2

#define MAX_DISTANCE_CM 100

void HCSR04Init();
void HCSR04Trigger(uint8_t trigger_pin);
uint16_t GetPulseWidth(uint8_t echo_pin);

void lcd_init();
void lcd_command(unsigned char cmnd);
void lcd_clear();
void lcd_print(char *str);
void lcd_setCursor(unsigned char x, unsigned char y);

void UART_Init(unsigned int baud_rate);
void UART_Transmit(unsigned char data);
unsigned char UART_Receive();

void GSM_SendCommand(char* command);
void GSM_SendSMS(char* phoneNumber, char* message);

void initialize()
{
	lcd_data_dir = 0xFF;
	_delay_ms(15);
	lcd_command(0x02);
	lcd_command(0x28);
	lcd_command(0x0C);
	lcd_command(0x06);
	lcd_command(0x01);
	_delay_ms(2);
}

void UART_Init(unsigned int baud_rate)
{
	// Calculate the baud rate value based on the desired baud rate and F_CPU
	unsigned int ubrr = F_CPU / 16 / baud_rate - 1;
	
	// Set the baud rate
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	
	// Enable receiver and transmitter
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	
	// Set frame format: 8 data bits, 1 stop bit, no parity
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	
	
}

void UART_Transmit(unsigned char data)
{
	// Wait for empty transmit buffer
	while (!(UCSR0A & (1 << UDRE0)));
	
	// Put data into buffer, sends the data
	UDR0 = data;
}

unsigned char UART_Receive()
{
	// Wait for data to be received
	while (!(UCSR0A & (1 << RXC0)));
	
	// Get and return received data from buffer
	return UDR0;
}

void GSM_SendCommand(char* command)
{
	// Send an AT command to the GSM module
	while (*command != '\0')
	{
		UART_Transmit(*command);
		command++;
	}
	
	// Send carriage return (CR) and line feed (LF) to end the command
	UART_Transmit('\r');
	UART_Transmit('\n');
	
	_delay_ms(100); // Wait for the GSM module to respond
}

void GSM_SendSMS(char* phoneNumber, char* message)
{
	// Send SMS command with phone number and message
	GSM_SendCommand("AT+CMGF=1"); // Set SMS text mode
	_delay_ms(100);
	GSM_SendCommand("AT+CMGS=\"");
	GSM_SendCommand(phoneNumber);
	GSM_SendCommand("\"");
	_delay_ms(100);
	GSM_SendCommand(message);
	_delay_ms(100);
	UART_Transmit(0x1A); // Send Ctrl+Z to indicate the end of the message
}


void lcd_command(unsigned char cmnd)
{
	lcd_port = (lcd_port & 0x0F) | (cmnd & 0xF0);
	lcd_port &= ~(1 << rs);
	lcd_port |= (1 << en);
	_delay_us(1);
	lcd_port &= ~(1 << en);
	
	_delay_us(200);
	
	lcd_port = (lcd_port & 0x0F) | (cmnd << 4);
	lcd_port |= (1 << en);
	_delay_us(1);
	lcd_port &= ~(1 << en);
	_delay_ms(2);
}

void lcd_clear()
{
	lcd_command(0x01);
	_delay_ms(2);
}

void lcd_print(char *str)
{
	int i;
	for (i = 0; str[i] != 0; i++)
	{
		lcd_port = (lcd_port & 0x0F) | (str[i] & 0xF0);
		lcd_port |= (1 << rs);
		lcd_port |= (1 << en);
		_delay_us(1);
		lcd_port &= ~(1 << en);
		_delay_us(200);
		lcd_port = (lcd_port & 0x0F) | (str[i] << 4);
		lcd_port |= (1 << en);
		_delay_us(1);
		lcd_port &= ~(1 << en);
		_delay_ms(2);
	}
}

void lcd_setCursor(unsigned char x, unsigned char y)
{
	unsigned char adr[] = {0x80, 0xC0};
	lcd_command(adr[y - 1] + x - 1);
	_delay_us(100);
}

void HCSR04Init()
{
	US_DDR |= (1 << US_TRIG_BIN1) | (1 << US_TRIG_BIN2);
}

void HCSR04Trigger(uint8_t trigger_pin)
{
	if (trigger_pin == US_TRIG_BIN1)
	US_PORT |= (1 << US_TRIG_BIN1);
	else if (trigger_pin == US_TRIG_BIN2)
	US_PORT |= (1 << US_TRIG_BIN2);
	
	_delay_us(15);
	
	if (trigger_pin == US_TRIG_BIN1)
	US_PORT &= ~(1 << US_TRIG_BIN1);
	else if (trigger_pin == US_TRIG_BIN2)
	US_PORT &= ~(1 <<US_TRIG_BIN2);
}

uint16_t GetPulseWidth(uint8_t echo_pin)
{
	uint32_t i, result;
	
	for (i = 0; i < 600000; i++)
	{
		if (!(US_PIN & (1 << echo_pin)))
		continue;
		else
		break;
	}
	
	if (i == 600000)
	return US_ERROR;
	
	TCCR1A = 0X00;
	TCCR1B = (1 << CS11);
	TCNT1 = 0x00;
	
	for (i = 0; i < 600000; i++)
	{
		if (US_PIN & (1 << echo_pin))
		{
			if (TCNT1 > 60000)
			break;
			else
			continue;
		}
		else
		break;
	}
	
	if (i == 600000)
	return US_NO_OBSTACLE;
	
	result = TCNT1;
	
	TCCR1B = 0x00;
	
	if (result > 60000)
	return US_NO_OBSTACLE;
	else
	return (result >> 1);
}

int main()
{
	initialize();
	
	HCSR04Init();
	
	UART_Init(9600);
	
	char numberString[4];
	
	uint16_t distance_bin1, distance_bin2;
	uint16_t previous_distance_bin1 = 0, previous_distance_bin2 = 0;
	
	while (1)
	{
		// Trigger and measure for Bin 1
		HCSR04Trigger(US_TRIG_BIN1);
		distance_bin1 = GetPulseWidth(US_ECHO_BIN1);
		distance_bin1 = (distance_bin1 * 0.034 / 2.0);
		
		// Trigger and measure for Bin 2
		HCSR04Trigger(US_TRIG_BIN2);
		distance_bin2 = GetPulseWidth(US_ECHO_BIN2);
		distance_bin2 = (distance_bin2 * 0.034 / 2.0);
		
		if (distance_bin1 >= 80 && distance_bin1 != previous_distance_bin1)
		{
			GSM_SendSMS("+94771659163", "Bin 1 is 80% full."); // Replace "+1234567890" with the recipient's phone number
			_delay_ms(1000);
		}
		
		if (distance_bin2 >= 80 && distance_bin2 != previous_distance_bin2)
		{
			GSM_SendSMS("+94771659163", "Bin 2 is 80% full."); // Replace "+9876543210" with the recipient's phone number
			_delay_ms(1000);
		}
		
		lcd_clear();
		lcd_setCursor(1, lcd_bin1_row);
		lcd_print("Bin 1: ");
		itoa(distance_bin1, numberString, 10);
		lcd_print(numberString);
		lcd_print(" %");
		
		lcd_setCursor(1, lcd_bin2_row);
		lcd_print("Bin 2: ");
		itoa(distance_bin2, numberString, 10);
		lcd_print(numberString);
		lcd_print(" %");
		
		_delay_ms(200);
	}
}