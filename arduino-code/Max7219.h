/*
 * ====================================================================
 * ICMI MAX7219 Arduino Library
 * ====================================================================
 * Created by Hendrik Busch in 2012
 * 
 * Please visit https://github.com/hmbusch/ICMI-MAX7219-Arduino
 * for the latest version.
 * 
 * For more information, please visit my project site at:
 * 
 * http://www.icanmakeit.de/category/make/projects/icmi-max7219-arduino-lib/
 */

// Pre-processor directive to ensure that this header file is only
// included once, no matter how often it is used in an include statement.
#ifndef Max7219_h
#define Max7219_h

// In order to use the well known Arduino commands, we need to include 
// the main Arduino header file. This will only work with Arduino 1.0 
// or greater, because the header file was named differently before the
// 1.0 release.
#include "Arduino.h"

/**
 * This class defines a controller class for the MAX7219 that wraps all
 * the communications with the chip in easy to use method calls. As the
 * MAX7219 primary purpose is to drive seven-segment displays, you will
 * often read the term digit. When controlling a LED matrix, digit is the
 * same as row.
 */
class Max7219 
{
  public:
    
	/**
	 * Creates a new controller instance. You need to pass the pin configuration
	 * to the constructor (data in, clock, load) and specify how many digits/rows
	 * you have connected to the MAX7219. This number is used to set the scan limit 
	 * of the IC. If you connect less than 8 digits, it makes sense not to scan
	 * through all 8 possible digits for better brightness. In addition, this 
	 * number is used to validate the digit number when calling any digit manipulation
	 * method.
	 *
	 * @param dataPin the number of the Arduino pin that is connected to the data in pin
	 *                of the MAX7219 (labeled DIN in the datasheet)
	 * @param clockPin the number of the Arduino pin that is connected to the clock pin
	 *                 of the MAX7219 (labeled CLK in the datasheet)
	 * @param loadPin the number of the Arduino pin that is connected to the load pin
	 *                of the MAX7219 (labeled LOAD in the datasheet)
     * @param digitCount the number of digits/rows connected to the chip. Make sure this number
	 *                   is correct.
	 */
    Max7219(uint8_t dataPin, uint8_t clockPin, uint8_t loadPin, uint8_t digitCount);
    
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
    void enable(boolean enable);
    
    /**
	 * Sets the register content of the designated digit/row to the specified value.
	 * Each of the eight bits in value represents a single segment/column of the 
	 * digit/row. The least significant bit in the value is segment A, the most
	 * significant is segment DP (sometimes called H). Please note: this bit order
	 * actually differs from the order sent to the MAX7219. I chose to implement
	 * a translation between my format and theirs because mine is easier to process.
	 * Note: this method produces unreadable results when Code-B decoding is activated.
	 *
	 * This is the translation table:
	 * 
	 *    bit In | bit sent | semgent
	 *    -------+----------+---------
	 *       0   |    6     |    A
	 *       1   |    5     |    B
	 *       2   |    4     |    C
     *       3   |    3     |    D
     *       4   |    2     |    E
     *       5   |    1     |    F
     *       6   |    0     |    G
     *       7   |    7     |    DP
     *
     * Example: the value to display a 5 on the display (segments A, F, G, C, D) would be
     *          B01101101. This is the value passed to this method. The translated value
	 *          sent to the IC would be B01011011.
	 * 
	 * @param which the index of the digit/row to configure. The value must be 
	 *              between 0 and the value of digitCount (given in the constructor)
	 *              minus one.
	 * @param value the value to apply to that digit/row (see above for examples)
	 */
    void setDigitRaw(uint8_t which, uint8_t value);
    
	/**
	 * Sets the overall brightness of the display. The MAX7219 supports 16 brightness
	 * levels, 1 being the darkest, 16 being the brightest. Please note that you 
	 * can only set the overall brightness. Individual brightness control per digit
	 * is not supported by the IC. Also note that you cannot switch of the display
	 * by reducing the brightness to a minimum. The lowest brightness level still has
	 * a duty cycle of 1/32. To turn the display off, use the enable() method.
	 * 
	 * @param level the desired brightness level, must be between one and sixteen, 
	 *              any other will be ignored.
	 *           
	 */
    void setIntensity(uint8_t level);
    
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
    void testDisplay(boolean testDisplay);
    
    /**
	 * This method returns a version indicator for this library. You can use it to
	 * check what version of this library is used, e.g. for issuing a warning when a
	 * non compatible version is used.
	 * 
	 * @return the version number as an integer
	 */
    uint8_t version();
    
	/**
	 * Clears the register of every digit, thereby blanking the display. This operation
	 * cannot be undone. If you want to shut off the display temporarily, consider using
	 * enable(false) instead.
	 */
    void clearDisplay();
    
  private:

    int dataPin;
    int clockPin;
    int loadPin;
    
    int digitCount;
	
	void writeRegister(int which, int value);
    
    void writeBoolRegister(int which, boolean value);
       
};

#endif
