#include "Arduino.h"
#include "Max7219.h"

// -------------------------------------------------------------------
// Register addresses
// -------------------------------------------------------------------
#define REG_NO_OP 0x00
#define REG_DIG_0 0x01
#define REG_DIG_1 0x02
#define REG_DIG_2 0x03
#define REG_DIG_3 0x04
#define REG_DIG_4 0x05
#define REG_DIG_5 0x06
#define REG_DIG_6 0x07
#define REG_DIG_7 0x08
#define REG_DECODE_MODE 0x09
#define REG_INTENSITY 0x0A
#define REG_SCAN_LIMIT 0x0B
#define REG_SHUTDOWN 0x0C
#define REG_DISPLAY_TEST 0x0F

// -------------------------------------------------------------------
// Decode modes
// -------------------------------------------------------------------

// no code B decode 
#define DECODE_CODEB_NONE 0
// code b decode for digit 0, no decode for other digits
#define DECODE_CODEB_0 1
// code b decode for digits 0-3, no decode for other digits
#define DECODE_CODEB_0_3 0x0F
// code b decode for all digits
#define DECODE_CODEB_0_7 0xFF

/**
 * Creates a new controller object for controlling the MAX7219.
 * Parameters needed are the three pins (data, clock, load) to which the
 * register is connected and the information how many digits (or rows, 
 * when using a LED matrix instead of seven segment display) are connected
 * to the IC. The minimum value is 1, the maximum value is 8. Any values
 * outside this range will be treatet as the edge of the range, i.e. -10
 * will be used as 1, 99 will be used as 8.
 * This method also initializes the IC by specifying full brightness, the
 * according scan limit.
 * PLEASE NOTE: This constructor does not enable the IC in order to give the
 * user more control over when the display will be activated. The user must
 * explicitly call enable(true) to activate the IC.
 */
Max7219::Max7219(int dataPin, int clockPin, int loadPin, int digitCount)
{
  this->dataPin = dataPin;
  this->clockPin = clockPin;
  this->loadPin = loadPin;
  this->digitCount = digitCount;
  
  pinMode(this->dataPin, OUTPUT);
  pinMode(this->clockPin, OUTPUT);
  pinMode(this->loadPin, OUTPUT);
  
  digitalWrite(loadPin, HIGH);
  
  // enforce normal mode
  testDisplay(false);
  
  // shutdown code b decoding
  writeRegister(REG_DECODE_MODE, DECODE_CODEB_NONE);
  
  // we are scanning at least 4 digits by default due to the warnings in the IC datasheet
  if (digitCount > 4)
  {
    writeRegister(REG_SCAN_LIMIT, digitCount - 1);
  }
  else
  {  
    writeRegister(REG_SCAN_LIMIT, 3);
  }
  
  clearDisplay();
  
  enable(false);
}

void Max7219::writeRegister(int which, int value)
{
  digitalWrite(loadPin, LOW);
  delayMicroseconds(5);
  shiftOut(this->dataPin, this->clockPin, MSBFIRST, which);
  shiftOut(this->dataPin, this->clockPin, MSBFIRST, value);
  delayMicroseconds(5);
  digitalWrite(loadPin, HIGH);
}

void Max7219::writeBoolRegister(int which, boolean value)
{
  if (value)
  {
    writeRegister(which, 1);
  }
  else
  {
    writeRegister(which, 0);
  }
}

void Max7219::enable(boolean enable)
{
  writeBoolRegister(REG_SHUTDOWN, enable);
}

/**
 * Sets the display intensity to one of 16 levels, the higher the brighter.
 * The level must be specified as a numeric value between 0 and 15. Any value
 * above 15 will be treated as 15.
 */
void Max7219::setIntensity(int level)
{
  byte newLevel = 0;
  if (level > 16)
  {
    newLevel = 15;
  }
  else if (level > 0)
  {
    newLevel = level - 1;
  }
  writeRegister(REG_INTENSITY, newLevel);
}

void Max7219::testDisplay(boolean testDisplay)
{
  writeBoolRegister(REG_DISPLAY_TEST, testDisplay);
}

/**
 * Sets the raw value of a digit. The given value is loaded directly to the register
 * for the given digit. The digit number must be between 0 and digitCount-1, so if 
 * you have defined 6 digits, they are adressed 0-5.
 */
void Max7219::setDigitRaw(int which, int value)
{
  if (which >= 0 && which < this->digitCount)
  {
    // register parameter is between 0 and 7, register address is simply +1
    writeRegister(which + 1, value);  
  }
}

void Max7219::clearDisplay()
{
  for(int i = 1; i < 9; i++)
  {
    writeRegister(i, 0);
  }
}

int Max7219::version()
{
  return 1;
}

