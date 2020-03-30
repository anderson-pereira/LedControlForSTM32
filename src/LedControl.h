/*
  LedControl.h - A library for controling Led Digits or 
  Led arrays with a MAX7219/MAX7221
  Copyright (c) 2007 Eberhard Fahle

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef LedControl_h
#define LedControl_h
#include "stm32f3xx_hal.h"

/*
 * Here are the segments to be switched on for characters and digits on
 * 7-Segment Displays
 */

typedef unsigned char byte;

const static byte charTable[128] = {
    0B01111110,0B00110000,0B01101101,0B01111001,0B00110011,0B01011011,0B01011111,0B01110000,
    0B01111111,0B01111011,0B01110111,0B00011111,0B00001101,0B00111101,0B01001111,0B01000111,
    0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,
    0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,
    0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,
    0B00000000,0B00000000,0B00000000,0B00000000,0B10000000,0B00000001,0B10000000,0B00000000,
    0B01111110,0B00110000,0B01101101,0B01111001,0B00110011,0B01011011,0B01011111,0B01110000,
    0B01111111,0B01111011,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,
    0B00000000,0B01110111,0B00011111,0B00001101,0B00111101,0B01001111,0B01000111,0B00000000,
    0B00110111,0B00000000,0B00000000,0B00000000,0B00001110,0B00000000,0B00000000,0B00000000,
    0B01100111,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,
    0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00001000,
    0B00000000,0B01110111,0B00011111,0B00001101,0B00111101,0B01001111,0B01000111,0B00000000,
    0B00110111,0B00000000,0B00000000,0B00000000,0B00001110,0B00000000,0B00000000,0B00000000,
    0B01100111,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,
    0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000
};


void shiftOut(uint16_t dataPin, GPIO_TypeDef* dataPort,
		uint16_t clkPin, GPIO_TypeDef* clkPort,
		uint8_t msbfirst, char data);

    /* Send out a single command to the device */
    void spiTransfer(int addr, byte opcode, byte data);
    
    /* 
     * Create a new controler 
     * Params :
     * int dataPin	The pin on the Arduino where data gets shifted out
     * int clockPin	The pin for the clock
     * int csPin	The pin for selecting the device when data is to be sent
     * int numDevices	The maximum number of devices that can be controled
     */
    void LedControlInit(uint16_t dataPin, GPIO_TypeDef* dataPort,
    					uint16_t clkPin, GPIO_TypeDef* clkPort,
    					uint16_t csPin, GPIO_TypeDef* csPort,
    					uint8_t numDevices);

    /*
     * Gets the maximum number of devices attached to
     * this LedControl.
     * Returns :
     * int the number of devices attached to this LedControl
     */
    int getDeviceCount();

    /* 
     * Set the shutdown (power saving) mode for the device
     * Params :
     * int addr		The address of the display to control
     * boolean b	If true the device goes into power-down mode. If false
     *			device goes into normal operation
     */
    void shutdown(int addr, byte b);

    /* 
     * Set the number of digits (or rows) to be displayed.
     * See datasheet for sideeffects of the scanlimit on the brightness
     * of the display.
     * Params :
     * int addr		The address of the display to control
     * int limit The number of digits to be displayed
     * Only values between 0 (only 1 digit) and 7 (all digits) are valid.
     */
    void setScanLimit(int addr, int limit);

    /* 
     * Set the brightness of the display.
     * Params:
     * int addr		The address of the display to control
     * int intensity the brightness of the display. 
     * Only values between 0(darkest) and 15(brightest) are valid.
     */
    void setIntensity(int addr, int intensity);

    /* 
     * Switch all Leds on the display off. 
     * Params:
     * int addr		The address of the display to control
     */
    void clearDisplay(int addr);

    /* 
     * Set the status for a specific Led.
     * Params :
     * int addr		The address of the display to control
     * int row		the row in which the led is located 
     *			Only values between 0 and 7 are valid.
     * int col		the column in which the led is located
     *			Only values between 0 and 7 are valid.
     * boolean state	If true the led is switched on, if false it is switched off
     */
    void setLed(int addr, int row, int col, byte state);

    /* 
     * Set the 8 Led's in a row to a new state
     * Params:
     * int addr		The address of the display to control
     * int row		The row on which the led's are to be set
     *			Only values between 0 and 7 are valid.
     * byte value	A bit set to 1 in this value will light up the
     *			corresponding led.
     */
    void setRow(int addr, int row, byte value);

    /* 
     * Set the 8 Led's in a column to a new state
     * Params:
     * int addr		The address of the display to control
     * int col		The column on which the led's are to be set
     *			Only values between 0 and 7 are valid.
     * byte value	A bit set to 1 in this value will light up the
     *			corresponding led.
     */
    void setColumn(int addr, int col, byte value);

    /* 
     * Display a hexadecimal digit on a 7-Segment Display
     * Params:
     * int addr		the address of the display to control
     * int digit	the position of the digit on the display
     * byte value	the value to be displayed. 
     *			Only values between 0x00 and 0x0F are valid.
     * boolean dp	If true also switches on the decimal point.
     */
    void setDigit(int addr, int digit, byte value, byte dp);

    /* 
     * Display a character on a 7-Segment display.
     * The char that can be acutally displayed are obviously quite limited.
     * Here is the whole set :	
     *	'0','1','2','3','4','5','6','7','8','9','0',
     *  'A','b','c','d','E','F','H','L','P',
     *  '.','-','_',' ' 
     * Params:
     * int addr		the address of the display to control
     * int digit	the position of the character on the display
     * char value	the character to be displayed. (See the limited set above!) 
     * boolean dp	If true also switches on the decimal point.
     */
    void setChar(int addr, int digit, char value, byte dp);

#endif	//LedControl.h



