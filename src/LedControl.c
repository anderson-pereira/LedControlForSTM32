/*
  LedControl.cpp - A library for controling Led Digits or 
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

//#include "WProgram.h"
#include "LedControl.h"


//the opcodes the MAX7221 and MAX7219 understand
#define OP_NOOP   0
#define OP_DIGIT0 1
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15


byte status[64] = {0};
/* This is the array we send the data with 2 commandbytes for 8 devices */
byte spidata[16] = {0};
/* Data is shifted out of this pin*/
uint32_t SPI_MOSI = 0;
/* The clock is signaled on this pin */
uint32_t SPI_CLK = 0;
/* This one is driven LOW for chip selectzion */
uint32_t SPI_CS = 0;
/* The maximum number of devices to be used */
GPIO_TypeDef* DATAPORT = {0};
GPIO_TypeDef* CLKPORT = {0};
GPIO_TypeDef* CSPORT = {0};

int maxDevices = 0;

void LedControlInit(uint16_t dataPin, GPIO_TypeDef* dataPort,
					uint16_t clkPin, GPIO_TypeDef* clkPort,
					uint16_t csPin, GPIO_TypeDef* csPort,
					uint8_t numDevices) {

    SPI_MOSI=dataPin;
    SPI_CLK=clkPin;
    SPI_CS=csPin;

    DATAPORT = dataPort;
    CLKPORT = clkPort;
    CSPORT = csPort;

    if(numDevices<=0 || numDevices>8 )
    	numDevices=8;

    maxDevices=numDevices;
//    pinMode(SPI_MOSI,OUTPUT);
//    pinMode(SPI_CLK,OUTPUT);
//    pinMode(SPI_CS,OUTPUT);
    HAL_GPIO_WritePin(csPort, SPI_CS, GPIO_PIN_SET);
//    digitalWrite(SPI_CS,HIGH);
    SPI_MOSI=dataPin;

    for(int i=0;i<64;i++) 
    	status[i]=0x00;

    for(int i=0;i<maxDevices;i++) {
    	spiTransfer(i,OP_DISPLAYTEST,0);
		//scanlimit is set to max on startup
		setScanLimit(i,7);
		//decode is done in source
		spiTransfer(i,OP_DECODEMODE,0);
		clearDisplay(i);
		//we go into shutdown-mode on startup
		shutdown(i,1);
    }
}

int getDeviceCount() {
    return maxDevices;
}

void shutdown(int addr, byte b) {
    if(addr<0 || addr>=maxDevices)
    	return;

    if(b)
    	spiTransfer(addr, OP_SHUTDOWN,0);
    else
    	spiTransfer(addr, OP_SHUTDOWN,1);
}
	
void setScanLimit(int addr, int limit) {
    if(addr<0 || addr>=maxDevices)
    	return;

    if(limit>=0 || limit<8)
    	spiTransfer(addr, OP_SCANLIMIT,limit);
}

void setIntensity(int addr, int intensity) {
    if(addr<0 || addr>=maxDevices)
    	return;

    if(intensity>=0 || intensity<16)	
    	spiTransfer(addr, OP_INTENSITY,intensity);
    
}

void clearDisplay(int addr) {
    int offset;

    if(addr<0 || addr>=maxDevices)
    	return;

    offset=addr*8;

    for(int i=0;i<8;i++) {
		status[offset+i]=0;
		spiTransfer(addr, i+1,status[offset+i]);
    }
}

void setLed(int addr, int row, int column, byte state) {
    int offset;
    byte val=0x00;

    if(addr<0 || addr>=maxDevices)
    	return;

    if(row<0 || row>7 || column<0 || column>7)
    	return;

    offset=addr*8;
    val=0B10000000 >> column;

    if(state)
    	status[offset+row]=status[offset+row]|val;
    else {
		val=~val;
		status[offset+row]=status[offset+row]&val;
    }

    spiTransfer(addr, row+1,status[offset+row]);
}
	
void setRow(int addr, int row, byte value) {
    int offset;

    if(addr<0 || addr>=maxDevices)
    	return;

    if(row<0 || row>7)
    	return;

    offset=addr*8;
    status[offset+row]=value;
    spiTransfer(addr, row+1,status[offset+row]);
}
    
void setColumn(int addr, int col, byte value) {
    byte val;

    if(addr<0 || addr>=maxDevices)
    	return;

    if(col<0 || col>7) 
    	return;

    for(int row=0;row<8;row++) {
		val=value >> (7-row);
		val=val & 0x01;
		setLed(addr,row,col,val);
    }
}

void setDigit(int addr, int digit, byte value, byte dp) {
    int offset;
    byte v;

    if(addr<0 || addr>=maxDevices)
	return;
    if(digit<0 || digit>7 || value>15)
	return;
    offset=addr*8;
    v=charTable[value];
    if(dp)
	v|=0B10000000;
    status[offset+digit]=v;
    spiTransfer(addr, digit+1,v);
    
}

void setChar(int addr, int digit, char value, byte dp) {
    int offset;
    byte index,v;

    if(addr<0 || addr>=maxDevices)
	return;
    if(digit<0 || digit>7)
 	return;
    offset=addr*8;
    index=(byte)value;
    if(index >127) {
	//nothing define we use the space char
	value=32;
    }
    v=charTable[index];
    if(dp)
	v|=0B10000000;
    status[offset+digit]=v;
    spiTransfer(addr, digit+1,v);
}

void shiftOut(uint16_t dataPin, GPIO_TypeDef* dataPort,
		uint16_t clkPin, GPIO_TypeDef* clkPort,
		uint8_t msbfirst, char data) {

		for(size_t i=0;i<8;i++) {
			if(msbfirst)
				HAL_GPIO_WritePin(dataPort, dataPin, (data>>(7-i))&1);
			else
				HAL_GPIO_WritePin(dataPort, dataPin, (data>>(i))&1);
			//HAL_Delay(1);
			HAL_GPIO_WritePin(clkPort, clkPin, GPIO_PIN_SET);
			//HAL_Delay(1);
			HAL_GPIO_WritePin(clkPort, clkPin, GPIO_PIN_RESET);
		}
}


void spiTransfer(int addr, volatile byte opcode, volatile byte data) {
    //Create an array with the data to shift out
    int offset=addr*2;
    int maxbytes=maxDevices*2;

    for(int i=0;i<maxbytes;i++)
    	spidata[i]=(byte)0;
    //put our device data into the array
    spidata[offset+1]=opcode;
    spidata[offset]=data;
    //enable the line 
    //digitalWrite(SPI_CS,LOW);
    HAL_GPIO_WritePin(CSPORT, SPI_CS, GPIO_PIN_RESET);
    //Now shift out the data 
    for(int i=maxbytes;i>0;i--)
    	shiftOut(SPI_MOSI, DATAPORT, SPI_CLK, CLKPORT, 1, spidata[i-1]);
    //latch the data onto the display
    //digitalWrite(SPI_CS,HIGH);
    HAL_GPIO_WritePin(CSPORT, SPI_CS, GPIO_PIN_SET);
}    


