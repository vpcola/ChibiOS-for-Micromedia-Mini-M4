/* 
Interface with 2.2" TFT LCD 320x240 using ILI9341 controller with SD card socket.

LCD Pinout:

1. SDO (MISO) - RB6
2. LED
3. SCK - RB7
4. SDI (MOSI) - RB8
5. D/C - RA1
6. RESET - 5V
7. CS - RA2
8. GND
9. VCC
|
|
J2

SD Pinout:

1. SD_CS - RA0
2. SD_MOSI -  RP12
3. SD_MISO - RP13
4. SD_SCK - RP14
|
|
J4
*/

#ifndef TFTv2_h
#define TFTv2_h

#include "ch.hpp"
#include "hal.h"

#define RGB565(r,g,b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))   

// toggle
#define TFT_DC_PORT GPIOB
#define TFT_DC_PAD  10

//Basic Colors
#define RED	    0xf800
#define GREEN	0x07e0
#define BLUE	0x001f
#define BLACK	0x0000
#define YELLOW	0xffe0
#define WHITE	0xffff

//Other Colors
#define CYAN		0x07ff	
#define BRIGHT_RED	0xf810	
#define GRAY1		0x8410  
#define GRAY2		0x4208  

//TFT resolution 240*320
#define MIN_X	0
#define MIN_Y	0
#define MAX_X	239
#define MAX_Y	319

class ILI9341
{
    public:

    ILI9341(SPIDriver * drv, const SPIConfig * cfg);

    unsigned char init();
    uint16_t readID();
    void fillScreen(uint16_t XL, uint16_t XR, uint16_t YU, uint16_t YD, uint16_t color);
    void fillRectangle(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width, uint16_t color);
    void clearScreen(void);
    void setXY(uint16_t poX, uint16_t poY);
    void setPixel(uint16_t poX, uint16_t poY,uint16_t color);
    void drawChar( uint8_t ascii, uint16_t poX, uint16_t poY,uint16_t size, uint16_t fgcolor);
    void drawString(char *string,uint16_t poX, uint16_t poY, uint16_t size,uint16_t fgcolor);
    void drawHorizontalLine( uint16_t poX, uint16_t poY, uint16_t length,uint16_t color);
    void drawLine( uint16_t x0,uint16_t y0,uint16_t x1, uint16_t y1,uint16_t color);
    void drawVerticalLine( uint16_t poX, uint16_t poY, uint16_t length,uint16_t color);
    void drawRectangle(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width,uint16_t color);
    void drawCircle(int poX, int poY, int r,uint16_t color);
    void fillCircle(int poX, int poY, int r,uint16_t color);
    void drawTriangle( int poX1, int poY1, int poX2, int poY2, int poX3, int poY3, uint16_t color);

    private:

    SPIDriver * _pSpi;
    SPIConfig * _pSpiCfg;

    uint16_t constrain(uint16_t x, uint16_t a, uint16_t b);
    void sendCmd(uint8_t index);
    void sendData8(uint8_t data);
    void sendData16(uint16_t data);
    void sendData(uint16_t *data, size_t size);
    uint8_t readReg(uint8_t Addr, uint8_t xParameter);
    void setCol(uint16_t StartCol,uint16_t EndCol);
    void setPage(uint16_t StartPage,uint16_t EndPage);

};

#endif
