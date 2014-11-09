#include "ICMIMax7219.h"

/**
 * The number of the Arduino pin that is connected to the data in pin.
 * The data in pin is pin 1 on the MAX7219.
 */
const uint8_t PIN_DATA = 5;

/**
 * The number of the Arduino pin that is connected to the clock pin.
 * The clock pin is pin 13 on the MAX7219.
 */
const uint8_t PIN_CLOCK = 6;

/**
 * The number of the Arduino pin that is connected to the load pin
 * The data in pin is pin 12 on the MAX7219.
 */
const uint8_t PIN_LOAD = 7;

/**
 * The number of LED rows connected to the MAX7219.
 */
const uint8_t ROW_COUNT = 8;


/**
 * Initialize a variable for controlling the MAX7219.
 */
ICMIMax7219 max7219;

/**
 * No setup required.
 */
void setup()
{
    max7219.begin(PIN_DATA, PIN_CLOCK, PIN_LOAD, ROW_COUNT);
}

void loop()
{
    // first enter test mode to check the LED connections
    max7219.testDisplay(true);
    // chip is disabled by default, we need to enable it now
    // after enabling, all LEDs should be lit
    max7219.enable(true);

    delay(3000);

    // disable test mode
    max7219.testDisplay(false);

    // display a led chaser
    ledChaser();

    // show a brightness fader
    intensityFader();

    // flash the display
    flasher();

    // disable the chip again for the next loop iteration
    max7219.enable(false);
    delay(1000);
}

/**
 * This method implements a led chaser across the display matrix.
 * It iterates through the columns 0-7 of each row, starting with row 0.
 * When one row is complete, this row is blanked and the loop jumps to
 * the next row.
 */
void ledChaser()
{
    uint8_t rowValue = 0;

    // make sure display is blank
    max7219.clearDisplay();

    // repeat 5 times
    for (uint8_t i = 0; i < 5; i++)
    {
        // Iterate through each row
        for(uint8_t row = 0; row < ROW_COUNT; row++)
        {
            // Iterate through each column
            for(uint8_t column = 0; column < 8; column++)
            {
                rowValue = 0;
                // set the current columns bit in the value to high (1)
                bitSet(rowValue, column);
                // write the new value to the led driver
                max7219.setRow(row, rowValue);
                // delay a short amount of time
                delay(50);
            }
            // blank the row so that nothing remains in this row before switching to the next
            max7219.setRow(row, 0);
        }
    }
}

/**
 * Display a checkerboard pattern and then
 * changes the display brightness from low to high and back,
 * as a smooth fade (well, as smooth as it can get with
 * only 16 levels).
 */
void intensityFader()
{
    // make sure the display is blank and dim
    max7219.clearDisplay();
    max7219.setIntensity(1);

    // write a checkerboard pattern to the rows
    for(uint8_t row = 0; row < ROW_COUNT; row++)
    {
        if (row == 0 || row % 2 == 0)
        {
            max7219.setRow(row, B01010101);
        }
        else
        {
            max7219.setRow(row, B10101010);
        }

    }

    // repeat 5 times
    for (uint8_t i = 0; i < 5; i++)
    {
        // ramp up the brightness
        for(uint8_t intensity = 1; intensity <= 16; intensity++)
        {
            max7219.setIntensity(intensity);
            delay(150);
        }
        // and down again
        for(uint8_t intensity = 16; intensity >= 1; intensity--)
        {
            max7219.setIntensity(intensity);
            delay(150);
        }
    }

    // set brightness back to maximum
    max7219.setIntensity(16);
}

/**
 * Illustrates how to flash the display by toggling between the enabled
 * and disabled state of the MAX7219.
 */
void flasher()
{
    // light all LEDs
    for(uint8_t row = 0; row < ROW_COUNT; row++)
    {
        max7219.setRow(row, B11111111);
    }

    // repeat 20 times
    for (uint8_t i = 0; i < 20; i++)
    {
        // flash the display be switching between enabled and disabled state
        max7219.enable(false);
        delay(100);
        max7219.enable(true);
        delay(100);
    }
}