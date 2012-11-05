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
#ifndef ICMI_Max7219_h
#define ICMI_Max7219_h

// In order to use the well known Arduino commands, we need to include
// the main Arduino header file. This will only work with Arduino 1.0
// or greater, because the header file was named differently before the
// 1.0 release.
#include "Arduino.h"

// -------------------------------------------------------------------
// Register addresses
// -------------------------------------------------------------------
#define REG_NO_OP 0x00             // no-op register
#define REG_DIG_0 0x01             // digit 0
#define REG_DIG_1 0x02             // digit 1
#define REG_DIG_2 0x03             // digit 2
#define REG_DIG_3 0x04             // digit 3
#define REG_DIG_4 0x05             // digit 4
#define REG_DIG_5 0x06             // digit 5
#define REG_DIG_6 0x07             // digit 6
#define REG_DIG_7 0x08             // digit 7
#define REG_DECODE_MODE 0x09       // decode mode register
#define REG_INTENSITY 0x0A         // display intensity register
#define REG_SCAN_LIMIT 0x0B        // scan limit register
#define REG_SHUTDOWN 0x0C          // shutdown register
#define REG_DISPLAY_TEST 0x0F      // display test register

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
 * This class defines a controller class for the MAX7219 that wraps all
 * the communications with the chip in easy to use method calls. As the
 * MAX7219 primary purpose is to drive seven-segment displays, you will
 * often read the term digit. When controlling a LED matrix, digit is the
 * same as row.
 */
class ICMIMax7219
{
public:

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
    void begin(uint8_t dataPin, uint8_t clockPin, uint8_t loadPin, uint8_t digitCount);

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
    void setRow(uint8_t which, uint8_t value);

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

    /**
     * the number of the Arduino pin that is connected to the data in pin
     */
    uint8_t dataPin;

    /**
     * the number of the Arduino pin that is connected to the clock pin
     */
    uint8_t clockPin;

    /**
     * the number of the Arduino pin that is connected to the load pin
     */
    uint8_t loadPin;

    /**
     * The number of digits/rows connected to the chip.
     */
    uint8_t digitCount;

    /**
     * Writes the given value to the register with the given address.
     * Effectively, this result in shifting 16 bits of data to the IC.
     *
     * @param which the registers address (use defined values)
     * @param value the value to write to the register. This value needs
     *              to conform to the rules given in the ICs datasheet.
     */
    void writeRegister(uint8_t which, uint8_t value);

    /**
     * Convenience method to write a boolean value to a register.
     * A value of true renders as a 1, false renders as 0.
     *
     * @param which the registers address (use defined values)
     * @param value the boolean value to set
     */
    void writeBoolRegister(uint8_t which, boolean value);
};

#endif
