#include "ili9341.h"
#include "font.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define FONT_SPACE 8

#define TFT_DC_LOW palSetPad(TFT_DC_PORT, TFT_DC_PAD)
#define TFT_DC_HIGH palClearPad(TFT_DC_PORT,TFT_DC_PAD)


ILI9341::ILI9341(SPIDriver * drv, const SPIConfig * cfg)
    : _pSpi(drv), _pSpiCfg((SPIConfig *) cfg)
{
}


/*
Returns
x: if x is between a and b
a: if x is less than a
b: if x is greater than b 
*/
uint16_t ILI9341::constrain(uint16_t x, uint16_t a, uint16_t b)
{
	if (x < a) return a;
	if (x > b) return b;
	return x;
}

void ILI9341::sendCmd(uint8_t cmd)
{
    TFT_DC_LOW;
    spiSelect(_pSpi);
    spiSend(_pSpi, 1, (const void *) &cmd);
    spiUnselect(_pSpi);
}

void ILI9341::sendData8(uint8_t data)
{
    TFT_DC_HIGH;
    spiSelect(_pSpi);
    spiSend(_pSpi, 1, (const void *) &data);
    spiUnselect(_pSpi);
}

void ILI9341::sendData16(uint16_t data)
{
    TFT_DC_HIGH;
    spiSelect(_pSpi);
    spiSend(_pSpi, 2, &data);
    spiUnselect(_pSpi);
}

void ILI9341::sendData(uint16_t *data, size_t size)
{
    //uint8_t count=0;
    TFT_DC_HIGH;
    spiSelect(_pSpi);

    /*
    for(count=0;count<howmany;count++)
    {
        data1 = data[count]>>8;
        data2 = data[count]&0xff;
        TFT_sendData(data1);
        TFT_sendData(data2);
    }
    */
    spiSend(_pSpi, size, data);
    spiUnselect(_pSpi);
}

uint8_t ILI9341::readReg(uint8_t Addr, uint8_t xParameter)
{
    uint8_t data=0;
    sendCmd(0xd9);                             /* ext command                  */
    sendData8(0x10+xParameter);                /* 0x11 is the first Parameter  */
    TFT_DC_LOW;
    spiSelect(_pSpi);
    spiSend(_pSpi, 1, &Addr);
    TFT_DC_HIGH;
    spiReceive(_pSpi, 1, &data);
    spiUnselect(_pSpi);

    return data;
}


uint16_t ILI9341::readID()
{
    uint8_t i=0;
    uint8_t data[3] ;
    uint16_t id = 0x0;

    spiAcquireBus(_pSpi);
    // Setup transfer params again
    spiStart(_pSpi, _pSpiCfg);

    //uint8_t ID[3] = {0x00, 0x93, 0x41};
    for(i=0;i<3;i++)
        data[i]= readReg(0xd3,i+1);
	
    id = (data[1] << 8) | (0xFF & data[2]);

    spiStop(_pSpi);
    spiReleaseBus(_pSpi);
    return id;
}

void ILI9341::setCol(uint16_t StartCol, uint16_t EndCol)
{
    sendCmd(0x2A);         /* Column Command address       */
    sendData16(StartCol);
    sendData16(EndCol);
}

void ILI9341::setPage(uint16_t StartPage,uint16_t EndPage)
{
    sendCmd(0x2B);     /* Column Command address       */
    sendData16(StartPage);
    sendData16(EndPage);
}


void ILI9341::fillScreen(uint16_t XL, uint16_t XR, uint16_t YU, uint16_t YD, uint16_t color)
{
    unsigned long  XY=0;
    unsigned long i=0;

    if(XL > XR)
    {
        XL = XL^XR;
        XR = XL^XR;
        XL = XL^XR;
    }
    if(YU > YD)
    {
        YU = YU^YD;
        YD = YU^YD;
        YU = YU^YD;
    }
    XL = constrain(XL, MIN_X,MAX_X);
    XR = constrain(XR, MIN_X,MAX_X);
    YU = constrain(YU, MIN_Y,MAX_Y);
    YD = constrain(YD, MIN_Y,MAX_Y);

    XY = (XR-XL+1);
    XY = XY*(YD-YU+1);

    /* We need to reaquire the bus and setup 
     parameters again
     */
    spiAcquireBus(_pSpi);
    spiStart(_pSpi, _pSpiCfg);

    setCol(XL,XR);
    setPage(YU, YD);
    sendCmd(0x2c);        /* start to write to display ra */
                          /* m                            */

    TFT_DC_HIGH;
    spiSelect(_pSpi);

    for(i=0; i < XY; i++)
    {
        spiSend(_pSpi, sizeof(color), &color);
    }

    spiUnselect(_pSpi);
    spiStop(_pSpi);
    spiReleaseBus(_pSpi);

}


void ILI9341::fillRectangle(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width, uint16_t color)
{
    fillScreen(poX, poX+length, poY, poY+width, color);
}

void ILI9341::clearScreen(void)
{
	unsigned int i;
	uint16_t data = 0;
    /* We need to reaquire the bus and setup 
       parameters again
    */
    spiAcquireBus(_pSpi);
    spiStart(_pSpi, _pSpiCfg);

    setCol(0, 239);
    setPage(0, 319);
    sendCmd(0x2c);       /* start to write to display ra */
                         /* m                            */

    TFT_DC_HIGH;
    spiSelect(_pSpi);
    for(i=0; i<76800; i++)
    {
        spiSend(_pSpi, 1, (const void *) &data);
    }
    spiUnselect(_pSpi);

    spiStop(_pSpi);
    spiReleaseBus(_pSpi);
}

unsigned char ILI9341::init (void)
{

    spiAcquireBus(_pSpi);
    spiStart(_pSpi, _pSpiCfg);

    chThdSleepMilliseconds(500);
    sendCmd(0x01);
    chThdSleepMilliseconds(200);


    sendCmd(0xCF);
    sendData8(0x00);
    sendData8(0x8B);
    sendData8(0X30);

    sendCmd(0xED);
    sendData8(0x67);
    sendData8(0x03);
    sendData8(0X12);
    sendData8(0X81);

    sendCmd(0xE8);
    sendData8(0x85);
    sendData8(0x10);
    sendData8(0x7A);

    sendCmd(0xCB);
    sendData8(0x39);
    sendData8(0x2C);
    sendData8(0x00);
    sendData8(0x34);
    sendData8(0x02);

    sendCmd(0xF7);
    sendData8(0x20);

    sendCmd(0xEA);
    sendData8(0x00);
    sendData8(0x00);

    sendCmd(0xC0);                                                      /* Power control                */
    sendData8(0x1B);                                                   /* VRH[5:0]                     */

    sendCmd(0xC1);                                                      /* Power control                */
    sendData8(0x10);                                                   /* SAP[2:0];BT[3:0]             */

    sendCmd(0xC5);                                                      /* VCM control                  */
    sendData8(0x3F);
    sendData8(0x3C);

    sendCmd(0xC7);                                                      /* VCM control2                 */
    sendData8(0XB7);

    sendCmd(0x36);                                                      /* Memory Access Control        */
    sendData8(0x08);

    sendCmd(0x3A);
    sendData8(0x55);

    sendCmd(0xB1);
    sendData8(0x00);
    sendData8(0x1B);

    sendCmd(0xB6);                                                      /* Display Function Control     */
    sendData8(0x0A);
    sendData8(0xA2);


    sendCmd(0xF2);                                                      /* 3Gamma Function Disable      */
    sendData8(0x00);

    sendCmd(0x26);                                                      /* Gamma curve selected         */
    sendData8(0x01);

    sendCmd(0xE0);                                                      /* Set Gamma                    */
    sendData8(0x0F);
    sendData8(0x2A);
    sendData8(0x28);
    sendData8(0x08);
    sendData8(0x0E);
    sendData8(0x08);
    sendData8(0x54);
    sendData8(0XA9);
    sendData8(0x43);
    sendData8(0x0A);
    sendData8(0x0F);
    sendData8(0x00);
    sendData8(0x00);
    sendData8(0x00);
    sendData8(0x00);

    sendCmd(0XE1);                                                      /* Set Gamma                    */
    sendData8(0x00);
    sendData8(0x15);
    sendData8(0x17);
    sendData8(0x07);
    sendData8(0x11);
    sendData8(0x06);
    sendData8(0x2B);
    sendData8(0x56);
    sendData8(0x3C);
    sendData8(0x05);
    sendData8(0x10);
    sendData8(0x0F);
    sendData8(0x3F);
    sendData8(0x3F);
    sendData8(0x0F);

    sendCmd(0x11);                                                      /* Exit Sleep                   */
    chThdSleepMilliseconds(120);
    sendCmd(0x29);                                                      /* Display on                   */

    clearScreen();

    spiStop(_pSpi);
    spiReleaseBus(_pSpi);

	return 1;
}

void ILI9341::setXY(uint16_t poX, uint16_t poY)
{
    spiAcquireBus(_pSpi);
    spiStart(_pSpi, _pSpiCfg);

    setCol(poX, poX);
    setPage(poY, poY);
    sendCmd(0x2c);

    spiStop(_pSpi);
    spiReleaseBus(_pSpi);
}

void ILI9341::setPixel(uint16_t poX, uint16_t poY,uint16_t color)
{
    setXY(poX, poY);

    spiAcquireBus(_pSpi);
    spiStart(_pSpi, _pSpiCfg);
    sendData16(color);
    spiStop(_pSpi);
    spiReleaseBus(_pSpi);
}

// the default font looks ugly, especially with lower case character in size 1. MD modifies here to use MD's font for it to look better.
void ILI9341::drawChar( uint8_t ascii, uint16_t poX, uint16_t poY,uint16_t size, uint16_t fgcolor)
{
	unsigned char i, j;

	unsigned char letter = ascii < 0x52 ? ascii - 0x20 : ascii - 0x52;

	for (i =0; i<5; i++ ) 
	{
		unsigned char line = ascii < 0x52 ? Alpha1[letter*5+i] : Alpha2[letter*5+i];

		for (j = 0; j<8; j++) {
			if (line & 0x1) {
				if (size == 1) // default size
					setPixel(poX+i, poY+j, fgcolor);
				else {  // big size
					fillRectangle(poX+i*size, poY+j*size, size, size, fgcolor);
				} 
			}
			line >>= 1;
		}
	}
}

void ILI9341::drawString(char *string,uint16_t poX, uint16_t poY, uint16_t size,uint16_t fgcolor)
{
    char * temp = string;
    while(*temp)
    {
        drawChar(*temp, poX, poY, size, fgcolor);
        temp++;

        if(poX < MAX_X)
        {
            poX += FONT_SPACE*size;        /* Move cursor right            */
        }
    }
}

void  ILI9341::drawHorizontalLine( uint16_t poX, uint16_t poY,
uint16_t length,uint16_t color)
{
	int i;
    /* We need to reaquire the bus and setup 
       parameters again
     */
    spiAcquireBus(_pSpi);
    spiStart(_pSpi, _pSpiCfg);


    setCol(poX,poX + length);
    setPage(poY,poY);
    sendCmd(0x2c);

    for(i=0; i<length; i++)
        sendData16(color);

    spiStop(_pSpi);
    spiReleaseBus(_pSpi);
}

void ILI9341::drawLine( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{

    int x = x1-x0;
    int y = y1-y0;
    int dx = fabs(x), sx = x0<x1 ? 1 : -1;
    int dy = -fabs(y), sy = y0<y1 ? 1 : -1;
    int err = dx+dy, e2;                                                /* error value e_xy             */
    for (;;){                                                           /* loop                         */
        setPixel(x0,y0,color);
        e2 = 2*err;
        if (e2 >= dy) {                                                 /* e_xy+e_x > 0                 */
            if (x0 == x1) break;
            err += dy; x0 += sx;
        }
        if (e2 <= dx) {                                                 /* e_xy+e_y < 0                 */
            if (y0 == y1) break;
            err += dx; y0 += sy;
        }
    }

}

void ILI9341::drawVerticalLine( uint16_t poX, uint16_t poY, uint16_t length, uint16_t color)
{
	int i;

    /* We need to reaquire the bus and setup 
       parameters again
    */
    spiAcquireBus(_pSpi);
    spiStart(_pSpi, _pSpiCfg);


    setCol(poX,poX);
    setPage(poY,poY+length);
    sendCmd(0x2c);
    for (i=0; i<length; i++)
        sendData16(color);

    spiStop(_pSpi);
    spiReleaseBus(_pSpi);
}

void ILI9341::drawRectangle(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width,uint16_t color)
{
    drawHorizontalLine(poX, poY, length, color);
    drawHorizontalLine(poX, poY+width, length, color);
    drawVerticalLine(poX, poY, width,color);
    drawVerticalLine(poX + length, poY, width,color);
}

void ILI9341::drawCircle(int poX, int poY, int r,uint16_t color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        setPixel(poX-x, poY+y,color);
        setPixel(poX+x, poY+y,color);
        setPixel(poX+x, poY-y,color);
        setPixel(poX-x, poY-y,color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
}

void ILI9341::fillCircle(int poX, int poY, int r,uint16_t color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {

        drawVerticalLine(poX-x, poY-y, 2*y, color);
        drawVerticalLine(poX+x, poY-y, 2*y, color);

        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);

}

void ILI9341::drawTriangle( int poX1, int poY1, int poX2, int poY2, int poX3, int poY3, uint16_t color)
{
    drawLine(poX1, poY1, poX2, poY2,color);
    drawLine(poX1, poY1, poX3, poY3,color);
    drawLine(poX2, poY2, poX3, poY3,color);
}

