//Written by Angelo Maldonado-Liu
//10/28/2022
//PSU ECE 411
//
//
//
/* To do:
      assign values to each row/column pair
      for the equals sign, add in an LCD clear and then output the answer.
      Hex to Binary to Decimal converter.
      need to decide if i have say, 10 in the buffer, when I hit the convert button, does it become 0b1010, and then become 0xA, because if so, then I need to change the size of acceptable values, so that I can fit a D, B, H symbol to signify what type of values we are getting.
      */



#include <Wire.h>
#include <string.h>
//#include <LiquidCrystal_I2C.h>

int row1 = 0;
int row2 = 1;

int col1 = 2;
int col2 = 3;


int LCD_address = 0x27;
//LiquidCrystal_I2C lcd(LCD_address,20,4);  // set the LCD address to blank for a 20 chars and 4 line display (need to determine for our LCD)


char charOutput;
//variables needed for the math, max of 20 digits
char operation = '0';
int valnum = 0;
char value0[14];
char value1[14];
char result[16];
int numType = 0;

int testint = 0;

char testing[] = "ABC";

int haveInput = 0;

int currentvalue0 = 0;
int currentvalue1 = 0;
int resultvalue = 0;

int numinputs = 0;

int screen_on = 0;

int val = 0;
int numCount;

//trash variable to hold things use with strtoul https://www.tutorialspoint.com/c_standard_library/c_function_strtoul.htm
char** garbage;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:


  /*Wire.begin();
  Wire.beginTransmission(LCD_address);
  Wire.write(0b0000000111);
  Wire.endTransmission();*/



  //variables

}

void loop() {
  // put your main code here, to run repeatedly:
  
  //LCD gpio ports
  testint = strtoul(testing, garbage, 16);
  Serial.write(testint);


  /*
   * if(valnum == 0 && numtype == 0)
   * {
   *  currentvalue0 = atoi(value0)
   * }
   * 
   * if(valnum == 0 && numtype == 1)
   * {
   *  currentvalue0 += atoi(value0[numinputs]) * 2 ^ numinputs;
   * }
   * 
   * if(valnum == 0 numtype == 2)
   * {
   *  currentvalue0 += atoi(value0[numinputs]) * 16 ^ numinputs;
   * }
   * 
   * if(valnum == 1 && numtype == 0)
   * {
   *  currentvalue1 = atoi(value1)
   * }
   * 
   * if(valnum == 1 && numtype == 1)
   * {
   *  currentvalue1 += atoi(value0[numinputs]) * 2 ^ numinputs;
   * }
   * 
   * if(valnum == 1 numtype == 2)
   * {
   *  currentvalue1 += atoi(value0[numinputs]) * 16 ^ numinputs;
   * }
   * 
   */
  
  pinMode(row1, OUTPUT);
  pinMode(row2, OUTPUT);

  pinMode(col1, INPUT);
  pinMode(col2, INPUT);
  //Serial.print("start of loop");
  digitalWrite(row1, HIGH);
  digitalWrite(row2, HIGH);
  while(digitalRead(col1) == 1 || digitalRead(col2) == 1)
  {
    delay(5);
  }
  haveInput = 0;
  //wait(50);
  delay(50);
/*  if(val# == 1)
  {
    lcd.setCursor(2,1);
  }
  else
  {
    lcd.setCursor(1,0);
  }
*/  //check which button
  while(haveInput == 0)
  {
    if(digitalRead(col1) == 1)
    {
      digitalWrite(row1, LOW);
      digitalWrite(row2, LOW);
      //change ROWS and COLUMNS 
      switchIO();
      Serial.print(digitalRead(col1));
      delay(50);
      if(digitalRead(row1) == 1)
      {
       Serial.print("row 1");
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
      else if(digitalRead(row2) == 1)
      {
        Serial.print("row 2");
        charOutput = '1';
//        Wire.beginTransmission(LCD_address);
//        Wire.write(0b1000110001);
//        Wire.endTransmission();
      }
      haveInput = 1;
      break;
    }
    if(digitalRead(col2) == 1)
    {
      digitalWrite(row1, LOW);
      digitalWrite(row2, LOW);
      //change ROWS and COLUMNS 
      switchIO();
      Serial.print("Column 2\n");
      delay(50);
      if(digitalRead(row1) == 1)
      {
        Serial.print("row 1");
        charOutput = '2';
//        Wire.beginTransmission(LCD_address);
//        Wire.write(0b1000110010);
//        Wire.endTransmission();
      }
      else if(digitalRead(row2) == 1)
      {
        Serial.print("row 2");
        charOutput = '3';
//        Wire.beginTransmission(LCD_address);
//        Wire.write(0b1000110011);
//        Wire.endTransmission();
      }

      haveInput = 1;
      break;
    }
    delay(10);

  }

  //write into the saved number values
/*  if(operation != '0')
  {
    val# = 1;
    return;
  }

  if(numType == 0)
  {
    if(val# == 0)
    {
      value0[numCount] = charOutput;
      numCount++;
    }
    else if(val# == 1)
    {
      value1[numCount] = charOutput;
      numCount++;
    }
  }
  else if(numType == 1)
  {
    if(val# == 0)
    {
      value0[numCount] = charOutput;
      numCount++;
    }
    else if(val# == 1)
    {
      value1[numCount] = charOutput;
      numCount++;
    }
  }

  else if(numType == 2)
  {
    if(val# == 0)
    {
      value0[numCount] = charOutput;
      numCount++;
    }
    else if(val# == 1)
    {
      value1[numCount] = charOutput;
      numCount++;
    }
  }



*/  
}

void switchIO() {
  pinMode(row1, INPUT);
  pinMode(row2, INPUT);

  pinMode(col1, OUTPUT);
  pinMode(col2, OUTPUT);
  digitalWrite(col1, HIGH);
  digitalWrite(col2, HIGH);

  return;
}
