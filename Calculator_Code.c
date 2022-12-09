/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"		//https://raspberrypi.github.io/pico-sdk-doxygen/gpio_8h.html



#define PICO_DEFAULT_LED_PIN 25

//Changing I2C default pins to GP14 and GP15
#define PICO_DEFAULT_I2C 1
#define PICO_DEFAULT_I2C_SDA_PIN 14
#define PICO_DEFAULT_I2C_SCL_PIN 15
#define i2c_default i2c1


/* Example code to drive a 16x2 LCD panel via a I2C bridge chip (e.g. PCF8574)

   NOTE: The panel must be capable of being driven at 3.3v NOT 5v. The Pico
   GPIO (and therefor I2C) cannot be used at 5v.

   You will need to use a level shifter on the I2C lines if you want to run the
   board at 5v.

   Connections on Raspberry Pi Pico board, other boards may vary.

   GPIO 4 (pin 6)-> SDA on LCD bridge board
   GPIO 5 (pin 7)-> SCL on LCD bridge board
   3.3v (pin 36) -> VCC on LCD bridge board
   GND (pin 38)  -> GND on LCD bridge board
*/
// commands
const int LCD_CLEARDISPLAY = 0x01;
const int LCD_RETURNHOME = 0x02;
const int LCD_ENTRYMODESET = 0x04;
const int LCD_DISPLAYCONTROL = 0x08;
const int LCD_CURSORSHIFT = 0x10;
const int LCD_FUNCTIONSET = 0x20;
const int LCD_SETCGRAMADDR = 0x40;
const int LCD_SETDDRAMADDR = 0x80;

// flags for display entry mode
const int LCD_ENTRYSHIFTINCREMENT = 0x01;
const int LCD_ENTRYLEFT = 0x02;

// flags for display and cursor control
const int LCD_BLINKON = 0x01;
const int LCD_CURSORON = 0x02;
const int LCD_DISPLAYON = 0x04;

// flags for display and cursor shift
const int LCD_MOVERIGHT = 0x04;
const int LCD_DISPLAYMOVE = 0x08;

// flags for function set
const int LCD_5x10DOTS = 0x04;
const int LCD_2LINE = 0x08;
const int LCD_8BITMODE = 0x10;

// flag for backlight control
const int LCD_BACKLIGHT = 0x08;

const int LCD_ENABLE_BIT = 0x04;

// By default these LCD display drivers are on bus address 0x27
static int addr = 0x27;


//integers for button array
int row1 = 0;
int row2 = 1;
int row3 = 2;
int row4 = 3;
int row5 = 4;

int col1 = 5;
int col2 = 6;
int col3 = 7;
int col4 = 8;
int col5 = 9;

int haveinput;
int currentvalue0 = 0;
int currentvalue1 = 0;
int resultvalue = 0;


char operation = '0';
int valuenumber = 0;
char value0[14];
char value1[14];
char result[16];
// 0 for decimal, 1 for binary, 2 for hex
int numtype = 0;

int numinputs = 0;

int temp; //this is for hex to ascii

int lcdstate = 0; //this is the state of on or off for the lcd

//testing
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
//gpio_init(LED_PIN);
//gpio_set_dir(LED_PIN, GPIO_OUT);


// Modes for lcd_send_byte
#define LCD_CHARACTER  1
#define LCD_COMMAND    0

#define MAX_LINES      2
#define MAX_CHARS      16

/* Quick helper function for single byte transfers */
void i2c_write_byte(uint8_t val) {
#ifdef i2c_default
    i2c_write_blocking(i2c_default, addr, &val, 1, false);
#endif
}

void lcd_toggle_enable(uint8_t val) {
    // Toggle enable pin on LCD display
    // We cannot do this too quickly or things don't work
#define DELAY_US 600
    sleep_us(DELAY_US);
    i2c_write_byte(val | LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
    i2c_write_byte(val & ~LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
}

// The display is sent a byte as two separate nibble transfers
void lcd_send_byte(uint8_t val, int mode) {
    uint8_t high = mode | (val & 0xF0) | LCD_BACKLIGHT;
    uint8_t low = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT;

    i2c_write_byte(high);
    lcd_toggle_enable(high);
    i2c_write_byte(low);
    lcd_toggle_enable(low);
}

void lcd_clear(void) {
    lcd_send_byte(LCD_CLEARDISPLAY, LCD_COMMAND);
}

// go to location on LCD
void lcd_set_cursor(int line, int position) {
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_byte(val, LCD_COMMAND);
}

static void inline lcd_char(char val) {
    lcd_send_byte(val, LCD_CHARACTER);
}

void lcd_string(const char *s) {
    while (*s) {
        lcd_char(*s++);
    }
}

void lcd_init() {
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x02, LCD_COMMAND);

    lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND);
    lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND);
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND);
    lcd_clear();
}

//Calculator code start


void switchIO()
{
	gpio_put(row1, false);
	gpio_put(row2, false);
	gpio_put(row3, false);
	gpio_put(row4, false);
	gpio_put(row5, false);
	
	gpio_set_dir(row1, false);
	gpio_set_dir(row2, false);
	gpio_set_dir(row3, false);
	gpio_set_dir(row4, false);
	gpio_set_dir(row5, false);
	
	
	gpio_set_dir(col1, true);
	gpio_set_dir(col2, true);
	gpio_set_dir(col3, true);
	gpio_set_dir(col4, true);
	gpio_set_dir(col5, true);
	
	gpio_put(col1, true);
	gpio_put(col2, true);
	gpio_put(col3, true);
	gpio_put(col4, true);
	gpio_put(col5, true);
	
	return;
}

void domath()
{
	if(operation == '+')
	{
		resultvalue = currentvalue0 + currentvalue1;
		numinputs = 0;
		currentvalue0 = 0;
		currentvalue1 = 0;
	}
	
	else if(operation == '-')
	{
		resultvalue = currentvalue0 - currentvalue1;
		numinputs = 0;
		currentvalue0 = 0;
		currentvalue1 = 0;
	}
	
	else if(operation == '*')
	{
		resultvalue = currentvalue0 * currentvalue1;
		numinputs = 0;
		currentvalue0 = 0;
		currentvalue1 = 0;
	}
	
	else if(operation == '/')
	{
		resultvalue = currentvalue0 / currentvalue1;
		numinputs = 0;
		currentvalue0 = 0;
		currentvalue1 = 0;	
	}
	
	valuenumber = 0;
	
	return;
}

void writetoLCD()
{
	if(numtype == 0)
	{
		itoa(resultvalue, result, 10);
		for(int a = 0; a < numinputs; a++)
		{

			lcd_send_byte(result[a], LCD_ENTRYSHIFTINCREMENT);
			sleep_ms(5);

		}
	}
	
	else if(numtype == 1)
	{
		itoa(resultvalue, result, 2);
		for(int a = 0; a < numinputs; a++)
		{
			lcd_send_byte(result[a], LCD_ENTRYSHIFTINCREMENT);
			sleep_ms(5);
		}
	}	
	
	else if(numtype == 2)
	{
		for(int b = numinputs; b < 0; b++)
		{
			temp = resultvalue / (16 ^ b);
			if(temp == 10)
			{
				result[b] = 'A';
			}
			
			else if(temp == 11)
			{
				result[b] = 'B';
			}
			
			else if(temp == 12)
			{
				result[b] = 'C';
			}
			
			else if(temp == 13)
			{
				result[b] = 'D';
			}
			
			else if(temp == 14)
			{
				result[b] = 'E';
			}
			
			else if(temp == 15)
			{
				result[b] = 'F';
			}
		}
			
		for(int a = 0; a < numinputs; a++)
		{
			sleep_ms(5);
			lcd_send_byte(result[a], LCD_ENTRYSHIFTINCREMENT);
		}
	}
	
	numinputs = 0;
	
}
		

void writevalue(char input)
{
	if(valuenumber == 0)
	{
		// if decimal mode and a decimal number
		if(numtype == 0 && input >= 47 && input <= 57)
		{
			currentvalue0 += atoi(input) * (10 ^ numinputs);
			value0[numinputs] = input;
			numinputs++;
		}
		
		// if binary and binary number
		else if(numtype == 1 && input != '0' && input != '1')
		{
			currentvalue0 += atoi(input) * (2 ^ numinputs);
			value0[numinputs] = input;
			numinputs++;
		}
		
		// if hex and hex number
		else if(numtype == 0 && ((input >= 47 && input <= 57) || (input >= 65 && input <= 70 )))
		{
			if(input == 'A')
			{
				currentvalue0 = 10 * (16 ^ numinputs);
			}
			else if(input == 'B')
			{
				currentvalue0 = 11 * (16 ^ numinputs);
			}
			else if(input == 'C')
			{
				currentvalue0 = 12 * (16 ^ numinputs);
			}
			else if(input == 'D')
			{
				currentvalue0 = 13 * (16 ^ numinputs);
			}
			else if(input == 'E')
			{
				currentvalue0 = 14 * (16 ^ numinputs);
			}
			else if(input == 'F')
			{
				currentvalue0 = 15 * (16 ^ numinputs);
			}
			value0[numinputs] = input;
			numinputs++;
			
		}
		

	}
	else if(valuenumber == 1)
	{
		// if decimal mode and a decimal number
		if(numtype == 0 && input >= 47 && input <= 57)
		{
			currentvalue1 += atoi(input) * (10 ^ numinputs);
			value1[numinputs] = input;
			numinputs++;
		}
		
		// if binary and binary number
		else if(numtype == 1 && input != '0' && input != '1')
		{
			currentvalue1 += atoi(input) * (2 ^ numinputs);
			value1[numinputs] = input;
			numinputs++;
		}
		
		// if hex and hex number
		else if(numtype == 0 && ((input >= 47 && input <= 57) || (input >= 65 && input <= 70 )))
		{
			if(input == 'A')
			{
				currentvalue1 = 10 * (16 ^ numinputs);
			}
			else if(input == 'B')
			{
				currentvalue1 = 11 * (16 ^ numinputs);
			}
			else if(input == 'C')
			{
				currentvalue1 = 12 * (16 ^ numinputs);
			}
			else if(input == 'D')
			{
				currentvalue1 = 13 * (16 ^ numinputs);
			}
			else if(input == 'E')
			{
				currentvalue1 = 14 * (16 ^ numinputs);
			}
			else if(input == 'F')
			{
				currentvalue1 = 15 * (16 ^ numinputs);
			}
			value1[numinputs] = input;
			numinputs++;
			
		}
		
		
	}
	writetoLCD();
	
	return;
}

void LED_FUNC(int x,int y){

	gpio_put(LED_PIN, 1);
	sleep_ms(x);
	gpio_put(LED_PIN, 0);
	sleep_ms(y);
}


int main() {
#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c/lcd_1602_i2c example requires a board with I2C pins
#else

    char msg[] = "pico";
    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    LED_FUNC(2000,400);
    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
	gpio_init(row1);
	gpio_init(row2);
	gpio_init(row3);
	gpio_init(row4);
	gpio_init(row5);
	
	gpio_init(col1);
	gpio_init(col2);
	gpio_init(col3);
	gpio_init(col4);
	gpio_init(col5);
	
	gpio_put(row1, true);
	gpio_put(row2, true);
	gpio_put(row3, true);
	gpio_put(row4, true);
	gpio_put(row5, true);
	
	gpio_set_dir(row1, true);
	gpio_set_dir(row2, true);
	gpio_set_dir(row3, true);
	gpio_set_dir(row4, true);
	gpio_set_dir(row5, true);
	
	gpio_set_dir(col1, false);
	gpio_set_dir(col2, false);
	gpio_set_dir(col3, false);
	gpio_set_dir(col4, false);
	gpio_set_dir(col5, false);
	
	
	gpio_put(col1, false);
	gpio_put(col2, false);
	gpio_put(col3, false);
	gpio_put(col4, false);
	gpio_put(col5, false);

    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    lcd_init(); //made it to here
    LED_FUNC(1000,400);
    static char *message[] =
            {
                    "RP2040 by", "Raspberry Pi",
                    "A brand new", "microcontroller",
                    "Twin core M0", "Full C SDK",
                    "More power in", "your product",
                    "More beans", "than Heinz! Lets add some shit"
            };

    while (1) {
        /*while(gpio_get(col1) != 0 || gpio_get(col2) != 0 || gpio_get(col3) != 0 || gpio_get(col4) != 0 || gpio_get(col5) != 0)
		{
			sleep_ms(5);
            //LED_FUNC(200,200);
		}
        */

        //mew code
        sleep_ms(5);
		lcd_set_cursor(0,0);
		sleep_ms(5);
		lcd_send_byte(0b0000001100,LCD_COMMAND);
		sleep_ms(5);
		
  		while(haveinput == 0)
  		{
    			if(gpio_get(col1) != 0)
    			{
	      //change ROWS and COLUMNS 
				switchIO();
			      //Serial.print(digitalRead(col1));
			     
			      if(gpio_get(row1) != 0)
			      {
                    
                   // LED_FUNC(100,100);
			       //Serial.print("row 1");
				/*Wire.beginTransmission(LCD_address);
				if(screen_on = 0)
				{
				  Wire.write(0b1000001100);
				}
				if(screen_on = 1)
				{
				  Wire.write(0b1000001000);
				}
				Wire.endTransmission();*/
			      }
			      else if(gpio_get(row2) != 0)
			      {
				//Serial.print("row 2");
				writevalue('7');
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }

				else if(gpio_get(row3) != 0)
			      {
				//Serial.print("row 2");
				writevalue('4');
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row4) != 0)
			      {
				//Serial.print("row 2");
				writevalue('1');
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row5) != 0)
			      {
				//Serial.print("row 2");
				
				writevalue('0');
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }
			      haveinput = 1;
			      break;
			    }
			
			
			    if(gpio_get(col2) != 0)
			    {
			      //change ROWS and COLUMNS 
			      switchIO();
			      //Serial.print("Column 2\n");
			      sleep_ms(5);
			      if(gpio_get(row1) != 0)
			      {
				//Serial.print("row 1");
				//writevalue(/*fill this with something */);
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110010);
			//        Wire.endTransmission();
			      }
			      else if(gpio_get(row2) != 0)
			      {
				//Serial.print("row 2");
				writevalue('8');
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110011);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row3) != 0)
			      {
				//Serial.print("row 2");
				writevalue('5');
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row4) != 0)
			      {
				//Serial.print("row 2");
				writevalue('2');
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row5) != 0)
			      {
				//Serial.print("row 2");
				//writevalue(/*fill this with something */);
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }

			      haveinput = 1;
			      break;
			    }


			    if(gpio_get(col3) != 0)
			    {
			      //change ROWS and COLUMNS 
			      switchIO();
			      //Serial.print("Column 2\n");
			      sleep_ms(5);
			      if(gpio_get(row1) != 0)
			      {
				//Serial.print("row 1");
				//writevalue(/*fill this with something */);
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110010);
			//        Wire.endTransmission();
			      }
			      else if(gpio_get(row2) != 0)
			      {
				//Serial.print("row 2");
				writevalue('9');
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110011);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row3) != 0)
			      {
				//Serial.print("row 2");
				writevalue('6');
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row4) != 0)
			      {
				//Serial.print("row 2");
				writevalue('3');
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row5) != 0)
			      {
                    sleep_ms(5);
                    LED_FUNC(2000,2400);
                    lcd_string(msg);
				//Serial.print("row 2");
				//writevalue(/*fill this with something */);
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }

			      haveinput = 1;
			      break;
			    }


			    if(gpio_get(col4) != 0)
			    {
			      //change ROWS and COLUMNS 
			      switchIO();
			      //Serial.print("Column 2\n");
			      sleep_ms(5);
			      if(gpio_get(row1) != 0)
			      {
				//Serial.print("row 1");
				//writevalue(/*fill this with something */);
				      lcd_clear();
				      valuenumber = 0;
				      currentvalue0 = 0;
				      currentvalue1 = 0;
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110010);
			//        Wire.endTransmission();
			      }
			      else if(gpio_get(row2) != 0)
			      {
				//Serial.print("row 2");
				//writevalue(/*fill this with something */);
				      operation = '+';
				      valuenumber = 1;
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110011);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row3) != 0)
			      {
				//Serial.print("row 2");
				//writevalue(/*fill this with something */);
					operation = '*';
					valuenumber = 1;
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row4) != 0)
			      {
				//Serial.print("row 2");
				//writevalue(/*fill this with something */);
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row5) != 0)
			      {
				//Serial.print("row 2");
				//writevalue(/*fill this with something */);
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }

			      haveinput = 1;
			      break;
			    }




			    if(gpio_get(col5) != 0)
			    {
			      //change ROWS and COLUMNS 
			      switchIO();
			      //Serial.print("Column 2\n");
			      sleep_ms(5);
			      if(gpio_get(row1) != 0)
			      {
				//Serial.print("row 1");
				      
				//turn on/off LCD
				      if(lcdstate == 0)
				      {
				      		lcd_send_byte(0b0000001000, LCD_COMMAND);
				      }
				      
				      if(lcdstate == 1)
				      {
					      lcd_send_byte(0b0000001100, LCD_COMMAND);
					}
				    	
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110010);
			//        Wire.endTransmission();
			      }
			      else if(gpio_get(row2) != 0)
			      {
				//Serial.print("row 2");
				//writevalue(/*fill this with something */);
				      operation = '-';
				      valuenumber = 1;
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110011);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row3) != 0)
			      {
				//Serial.print("row 2");
				//writevalue(/*fill this with something */);
					operation = '/';
					valuenumber = 1;
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row4) != 0)
			      {
				//Serial.print("row 2");
				//writevalue(/*fill this with something */);
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }
				else if(gpio_get(row5) != 0)
			      {
				//Serial.print("row 2");
				domath();
				valuenumber = 0;
					currentvalue0 = 0;
					currentvalue1 = 0;
			//        Wire.beginTransmission(LCD_address);
			//        Wire.write(0b1000110001);
			//        Wire.endTransmission();
			      }

			      haveinput = 1;
			      break;
			}
			    sleep_ms(10);

		}
	



/*
        for (int m = 0; m < sizeof(message) / sizeof(message[0]); m += MAX_LINES) {
            for (int line = 0; line < MAX_LINES; line++) {
                lcd_set_cursor(0,0);
                //lcd_set_cursor(line, (MAX_CHARS / 2) - strlen(message[m + line]) / 2);
                lcd_string(message[m + line]);
            }
            sleep_ms(2000);
            lcd_clear();
        }
        */
    }

    return 0;
#endif
}
