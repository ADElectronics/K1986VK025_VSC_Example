#ifndef _LCD_ST7735_
#define _LCD_ST7735_

#include "MLDR187_lib.h"
#include "MLDR187.h"
#include "MLDR187_gpio.h"
#include "MLDR187_ssp.h"
#include "fonts.h"

#define ST7735_CS_PORT			MDR_PORTD
#define ST7735_CS_PIN			PORT_Pin_0

#define ST7735_DC_PORT			MDR_PORTD
#define ST7735_DC_PIN			PORT_Pin_9

#define ST7735_RST_PORT			MDR_PORTD
#define ST7735_RST_PIN			PORT_Pin_4

#define ST7735_SPI				MDR_SSP2

// Использовать ли софтовый сброс заместо хардварного ?
#define LCD_SOFT_RESET			0

// Задержки в мс
#define ST7735_SOFT_RST_DELAY	350
#define ST7735_HRDW_RST_DELAY 	350

#define ST7735_MADCTL_MY  		0x80
#define ST7735_MADCTL_MX  		0x40
#define ST7735_MADCTL_MV  		0x20
#define ST7735_MADCTL_ML  		0x10
#define ST7735_MADCTL_RGB		0x00
#define ST7735_MADCTL_BGR 		0x08
#define ST7735_MADCTL_MH  		0x04

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST7735_WIDTH			160
#define ST7735_HEIGHT			80
#define ST7735_XSTART			0//1
#define ST7735_YSTART			24//26
#define ST7735_ROTATION			(ST7735_MADCTL_MV | ST7735_MADCTL_BGR) /* ST7735_MADCTL_MY | ST7735_MADCTL_MX | */

#define	ST7735_R5R6R5_BLACK		0x0000
#define	ST7735_R5R6R5_BLUE		0x001F
#define	ST7735_R5R6R5_RED		0xF800
#define	ST7735_R5R6R5_GREEN		0x07E0
#define ST7735_R5R6R5_CYAN		0x07FF
#define ST7735_R5R6R5_MAGENTA	0xF81F
#define ST7735_R5R6R5_YELLOW	0xFFE0
#define ST7735_R5R6R5_WHITE		0xFFFF
#define ST7735_R8G8B8_TO_R5R6R5(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

void ST7735_Init();
void ST7735_Clear(void);
void ST7735_FillRect(uint8_t startX, uint8_t startY, uint8_t stopX, uint8_t stopY, uint16_t color);
void ST7735_PutPix(uint8_t x, uint8_t y, uint16_t Color);
void ST7735_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
void ST7735_PutChar(uint8_t x, uint8_t y, uint8_t chr, FontDef font, uint16_t charColor, uint16_t bkgColor);
void ST7735_PutString(uint8_t x, uint8_t y, const uint8_t str[], FontDef font, uint16_t charColor, uint16_t bkgColor);
void ST7735_PutDec(int16_t num, uint8_t dcount, uint8_t x, uint8_t y, FontDef font, uint16_t fntColor, uint16_t bkgColor);
void ST7735_PutImage(uint8_t x, uint8_t y, uint16_t w, uint16_t h, const uint16_t *img);
void ST7735_PutImageR2G3B3(uint8_t x, uint8_t y, uint16_t w, uint16_t h, const uint8_t *img);

#define ST7735_CS_LOW 			PORT_SetReset(ST7735_CS_PORT, ST7735_CS_PIN, RESET);
#define ST7735_CS_HIGH 			PORT_SetReset(ST7735_CS_PORT, ST7735_CS_PIN, SET);

#define ST7735_DC_LOW 			PORT_SetReset(ST7735_DC_PORT, ST7735_DC_PIN, RESET);
#define ST7735_DC_HIGH 			PORT_SetReset(ST7735_DC_PORT, ST7735_DC_PIN, SET);

#define ST7735_RST_LOW 			PORT_SetReset(ST7735_RST_PORT, ST7735_RST_PIN, RESET);
#define ST7735_RST_HIGH 		PORT_SetReset(ST7735_RST_PORT, ST7735_RST_PIN, SET);

#define ST7735_DELAY_MS(ms)			{ \
   uint32_t DelayCnt = 0; \
   while (DelayCnt++ < (0x0001000 * ms)) {}; }

#endif // _LCD_ST7735_

