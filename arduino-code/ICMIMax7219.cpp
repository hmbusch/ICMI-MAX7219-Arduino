#include "Arduino.h"
#include "ICMIMax7219.h"

/**
 * Initializes the instance with the values given. If the instance was created
 * using the empty constructor, you *must* call this method before calling any
 * other method.
 * If the instance was created using the constructor with parameters, this method
 * will be called for you by the constructor.
 *
 * This method does not enable the chip, so it will not display anything
 * until enable(true) is called.
 *
 * @param dataPin the number of the Arduino pin that is connected to the data in pin
 *                of the MAX7219 (pin 1, labeled DIN in the datasheet)
 * @param clockPin the number of the Arduino pin that is connected to the clock pin
 *                 of the MAX7219 (pin 13, labeled CLK in the datasheet)
 * @param loadPin the number of the Arduino pin that is connected to the load pin
 *                of the MAX7219 (pin 12, labeled LOAD in the datasheet)
 * @param digitCount the number of digits/rows connected to the chip. Make sure this number
 *                   is correct. A number lower than 1 is treated as on, a number higher
 *                   than 8 will be treated as 8.
 */
void ICMIMax7219::begin(uint8_t dataPin, uint8_t clockPin, uint8_t loadPin, uint8_t digitCount)
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

    // switch to maximum brightness
    setIntensity(16);

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

/**
 * Writes the given value to the register with the given address.
 * Effectively, this result in shifting 16 bits of data to the IC.
 *
 * @param which the registers address (use defined values)
 * @param value the value to write to the register. This value needs
 *              to conform to the rules given in the ICs datasheet.
 */
void ICMIMax7219::writeRegister(uint8_t which, uint8_t value)
{
    digitalWrite(loadPin, LOW);
    delayMicroseconds(5);
    shiftOut(this->dataPin, this->clockPin, MSBFIRST, which);
    shiftOut(this->dataPin, this->clockPin, MSBFIRST, value);
    delayMicroseconds(5);
    digitalWrite(loadPin, HIGH);
}

/**
 * Convenience method to write a boolean value to a register.
 * A value of true renders as a 1, false renders as 0.
 *
 * @param which the registers address (use defined values)
 * @param value the boolean value to set
 */
void ICMIMax7219::writeBoolRegister(uint8_t which, boolean value)
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

/**
 * Enables or disables the IC. When the IC is disabled, the display will
 * automatically be blanked by the IC but all the configuration data
 * (including the digit configurations) will remain intact. You can make
 * the whole display flash its current content by repeatedly disabling
 * and enabling the chip.
 *
 * @param enable if true, the chip will be enabled, otherwise the chip
 *               will be disabled
 */
void ICMIMax7219::enable(boolean enable)
{
    writeBoolRegister(REG_SHUTDOWN, enable);
}

/**
 * Sets the overall brightness of the display. The MAX7219 supports 16 brightness
 * levels, 1 being the darkest, 16 being the brightest. Please note that you
 * can only set the overall brightness. Individual brightness control per digit
 * is not supported by the IC. Also note that you cannot switch of the display
 * by reducing the brightness to a minimum. The lowest brightness level still has
 * a duty cycle of 1/32. To turn the display off, use the enable() method.
 *
 * @param level the desired brightness level, must be between one and sixteen.
 *              Values larger than 16 will be treated as 16, values smaller than
 *              1 are treated as 1.
 *
 */
void ICMIMax7219::setIntensity(uint8_t level)
{
    uint8_t newLevel = 0;

    // The value actually written to the MAX7219 must be between
    // 0 and 15, therefore some calculations are necessary.
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

/**
 * Enables or disables the display test mode of the MAX7219. When in test mode,
 * all digits/rows will be completly lit with the brightest setting. The test
 * mode also ignores the set scan limit and thus always scans through all eight
 * digits, even if they are not connected. Switching to test mode DOES NOT delete
 * the previous configuration. So once test mode is disabled again, the IC will
 * resume the previous operation.
 *
 * @param testDisplay true enables the test mode, false disables it
 */
void ICMIMax7219::testDisplay(boolean testDisplay)
{
    writeBoolRegister(REG_DISPLAY_TEST, testDisplay);
}

/**
 * Sets the register content of the designated digit/row to the specified value.
 * Each of the eight bits in value represents a single segment/column of the
 * digit/row. The least significant bit in the value is segment A, the most
 * significant is segment DP (sometimes called H). Please note: this bit order
 * actually differs from the order sent to the MAX7219. I chose to implement
 * a translation between my format and theirs because mine is easier to process.
 * Note: this method produces unreadable results when Code-B decoding is activated.
 * The bit translation table can be found in the header file of this library.
 *
 * @param which the index of the digit/row to configure. The value must be
 *              between 0 and the value of digitCount (given in the constructor)
 *              minus one.
 * @param value the value to apply to that digit/row (see above for examples)
 */
void ICMIMax7219::setDigitRaw(uint8_t which, uint8_t value)
{
    if (which >= 0 && which < this->digitCount)
    {
        // Apply the bit translation to the value
        uint8_t translatedValue = value;
        bitWrite(translatedValue, 6, bitRead(value, 0));
        bitWrite(translatedValue, 5, bitRead(value, 1));
        bitWrite(translatedValue, 4, bitRead(value, 2));
        bitWrite(translatedValue, 2, bitRead(value, 4));
        bitWrite(translatedValue, 1, bitRead(value, 5));
        bitWrite(translatedValue, 0, bitRead(value, 6));
        // register parameter is between 0 and 7, register address is simply +1
        writeRegister(which + 1, translatedValue);
    }
}

/**
 * This method is an alias for setDigitRaw(uint8_t, uint8_t) that has
 * a less confusing name when working with a matrix instead of a seven
 * segment display.
 * For more documentation, please refer to setDigitRaw().
 *
 * @param which the index of the row to configure. The value must be
 *              between 0 and the value of digitCount (given in the constructor)
 *              minus one.
 * @param value the value to apply to that row
 */
void ICMIMax7219::setRow(uint8_t which, uint8_t value)
{
    setDigitRaw(which, value);
}

/**
 * Clears the register of every digit, thereby blanking the display. This operation
 * cannot be undone. If you want to shut off the display temporarily, consider using
 * enable(false) instead.
 */
void ICMIMax7219::clearDisplay()
{
    for(int i = 1; i < 9; i++)
    {
        writeRegister(i, 0);
    }
}

/**
 * This method returns a version indicator for this library. You can use it to
 * check what version of this library is used, e.g. for issuing a warning when a
 * non compatible version is used.
 *
 * @return the version number as an integer
 */
uint8_t ICMIMax7219::version()
{
    return 2;
}

