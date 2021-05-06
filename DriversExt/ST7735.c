#include "ST7735.h"

void ST7735_SendCmd(uint8_t cmd);
void ST7735_SendData(uint8_t data);
void ST7735_SendRAW(uint8_t data);
void ST7735_at(uint8_t startX, uint8_t startY, uint8_t stopX, uint8_t stopY);

__inline void ST7735_SendData(uint8_t data)
{
   ST7735_DC_HIGH;
   SSP_SendData(ST7735_SPI, (uint16_t)data);
   while(SSP_GetFlagStatus(ST7735_SPI, sspFlagBusy) == SET) { };
}

__inline void ST7735_SendCmd(uint8_t cmd)
{
   ST7735_DC_LOW;
   SSP_SendData(ST7735_SPI, (uint16_t)cmd);
   while(SSP_GetFlagStatus(ST7735_SPI, sspFlagBusy) == SET) { };
}

__inline void ST7735_SendRAW(uint8_t data)
{
   SSP_SendData(ST7735_SPI, (uint16_t)data);
   while(SSP_GetFlagStatus(ST7735_SPI, sspFlagBusy) == SET) { };
}

void ST7735_at(uint8_t startX, uint8_t startY, uint8_t stopX, uint8_t stopY)
{
	ST7735_SendCmd(ST7735_CASET);
	ST7735_SendData(0x00);
	ST7735_SendData(startX + ST7735_XSTART);
	ST7735_SendData(0x00);
	ST7735_SendData(stopX + ST7735_XSTART);

	ST7735_SendCmd(ST7735_RASET);
	ST7735_SendData(0x00);
	ST7735_SendData(startY + ST7735_YSTART);
	ST7735_SendData(0x00);
	ST7735_SendData(stopY + ST7735_YSTART);

	ST7735_SendCmd(ST7735_RAMWR);
}

__inline void ST7735_Clear(void)
{
	ST7735_FillRect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, 0);
}

void ST7735_Init()
{
	ST7735_CS_LOW;

#if LCD_SOFT_RESET > 0
	ST7735_SendCmd(ST7735_SWRESET);
	ST7735_DELAY_MS(ST7735_SOFT_RST_DELAY);
#else
	ST7735_RST_LOW;
	ST7735_DELAY_MS(10);
	ST7735_RST_HIGH;
	ST7735_DELAY_MS(ST7735_HRDW_RST_DELAY);
#endif

	ST7735_SendCmd(ST7735_SLPOUT);
	ST7735_DELAY_MS(ST7735_HRDW_RST_DELAY);

	ST7735_SendCmd(ST7735_FRMCTR1); // Frame Rate Control 1
	ST7735_SendData(0x01);
	ST7735_SendData(0x2c);
	ST7735_SendData(0x2d);

	ST7735_SendCmd(ST7735_FRMCTR2); // Frame Rate Control 2
	ST7735_SendData(0x01);
	ST7735_SendData(0x2c);
	ST7735_SendData(0x2d);

	ST7735_SendCmd(ST7735_FRMCTR3); // Frame Rate Control 3
	ST7735_SendData(0x01);
	ST7735_SendData(0x2c);
	ST7735_SendData(0x2d);
	ST7735_SendData(0x01);
	ST7735_SendData(0x2c);
	ST7735_SendData(0x2d);

	ST7735_SendCmd (ST7735_COLMOD); // Режим цвета:
	//ST7735_SendData(0x03); // RGB 4-4-4
	ST7735_SendData(0x05); // RGB 5-6-5
	//ST7735_SendData(0x06); // RGB 6-6-6

	ST7735_SendCmd(ST7735_INVOFF); // Инверсия выкл
	//ST7735_SendCmd(ST7735_INVON); // вкл

	ST7735_SendCmd(ST7735_PWCTR1);
	ST7735_SendData(0xa2);
	ST7735_SendData(0x02);
	ST7735_SendData(0x84);

	ST7735_SendCmd(ST7735_PWCTR2);
	ST7735_SendData(0xc5);

	ST7735_SendCmd(ST7735_PWCTR3);
	ST7735_SendData(0x0a);
	ST7735_SendData(0x00);

	ST7735_SendCmd(ST7735_PWCTR4);
	ST7735_SendData(0x8a);
	ST7735_SendData(0x2a);

	ST7735_SendCmd(ST7735_PWCTR5);
	ST7735_SendData(0x8a);
	ST7735_SendData(0xee);

	ST7735_SendCmd(ST7735_VMCTR1);
	ST7735_SendData(0x0e);

	ST7735_SendCmd(ST7735_CASET);
	ST7735_SendData(0x00);
	ST7735_SendData(0x00);
	ST7735_SendData(0x00);
	ST7735_SendData(0x4f);

	ST7735_SendCmd(ST7735_RASET);
	ST7735_SendData(0x00);
	ST7735_SendData(0x00);
	ST7735_SendData(0x00);
	ST7735_SendData(0x9f);

	ST7735_SendCmd(0x36);
	ST7735_SendData(ST7735_ROTATION);

	ST7735_SendCmd(0x26); // Гамма
	ST7735_SendData(0x01); // (G2.2) или (G1.0)
	//ST7735_SendData(0x02); // (G1.8) или (G2.5)
	//ST7735_SendData(0x04); // (G2.5) или (G2.2)
	//ST7735_SendData(0x08); // (G1.0) или (G1.8)

	ST7735_SendCmd(ST7735_NORON);
	ST7735_CS_HIGH;

	ST7735_Clear();

	ST7735_CS_LOW;
	ST7735_SendCmd(ST7735_DISPON);
	ST7735_CS_HIGH;
}

void ST7735_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color)
{
	if((x >= ST7735_WIDTH) || (y >= ST7735_HEIGHT))
		return;
	if((x + w - 1) >= ST7735_WIDTH)
		w = ST7735_WIDTH - x;
	if((y + h - 1) >= ST7735_HEIGHT)
		h = ST7735_HEIGHT - y;

	ST7735_CS_LOW;
	ST7735_at(x, y, (x + w - 1), (y + h - 1));
	ST7735_DC_HIGH;

	for(y = h; y > 0; y--)
	{
		for(x = w; x > 0; x--)
		{
			uint8_t data = (uint8_t)(color >> 8);
			ST7735_SendRAW(data);
			data = (uint8_t)(color & 0xFF);
			ST7735_SendRAW(data);
		}
	}
	
	ST7735_CS_HIGH;
}

void ST7735_PutPix(uint8_t x, uint8_t y, uint16_t Color) 
{
	ST7735_CS_LOW;
	ST7735_at(x, y, x + 1, y + 1);
	ST7735_SendData((uint8_t)(Color >> 8));
	ST7735_SendData((uint8_t)(Color & 0xFF));
	ST7735_CS_HIGH;
}

void ST7735_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color)
{
	ST7735_CS_LOW;

	if(x1 == x2)
	{
		ST7735_at(x1, y1, x1, y2);
		ST7735_DC_HIGH;
		for(; y1 < y2; y1++)
		{
			uint8_t data = (uint8_t)(color >> 8);
			ST7735_SendRAW(data);
			data = (uint8_t)(color & 0xFF);
			ST7735_SendRAW(data);
		}
	}
	else if(y1 == y2)
	{
		ST7735_at(x1, y1, x2, y1);
		ST7735_DC_HIGH;
		for(; x1 < x2; x1++)
		{
			uint8_t data = (uint8_t)(color >> 8);
			ST7735_SendRAW(data);
			data = (uint8_t)(color & 0xFF);
			ST7735_SendRAW(data);
		}
	}
	else
	{
		/* code */
	}

	ST7735_CS_HIGH;
}

// печать десятичного числа
void ST7735_PutDec(int16_t numb, uint8_t dcount, uint8_t x, uint8_t y, FontDef font, uint16_t fntColor, uint16_t bkgColor)
{
	uint16_t divid = 10000;
	uint8_t i, res;
	
	if(numb < 0)
	{
		numb = -numb;
		ST7735_PutChar(x, y, '-', font, fntColor, bkgColor);
		x += font.width;
	}
	else
	{
		ST7735_PutChar(x, y, '+', font, fntColor, bkgColor);
		x += font.width;
	}
	
	for (i=5; i!=0; i--)
	{
		res = (numb / divid) + 0x30;

		if (i <= dcount)
		{
			ST7735_PutChar(x, y, res, font, fntColor, bkgColor);
			x += font.width;
		}

		numb %= divid;
		divid /= 10;
	}
}

void ST7735_PutChar(uint8_t x, uint8_t y, uint8_t ch, FontDef font, uint16_t charColor, uint16_t bkgColor)
{
	uint32_t i, b, j;

	ST7735_CS_LOW;
	ST7735_at(x, y, (x + font.width - 1), (y + font.height - 1));
	ST7735_DC_HIGH;
	for(i = 0; i < font.height; i++)
	{
		b = font.data[(ch - 32) * font.height + i];
		for(j = 0; j < font.width; j++)
		{
			if((b << j) & 0x8000)
			{
				uint8_t data = (uint8_t)(charColor >> 8);
				ST7735_SendRAW(data);
				data = (uint8_t)(charColor & 0xFF);
				ST7735_SendRAW(data);
			}
			else
			{
				uint8_t data = (uint8_t)(bkgColor >> 8);
				ST7735_SendRAW(data);
				data = (uint8_t)(bkgColor & 0xFF);
				ST7735_SendRAW(data);
			}
		}
	}
	ST7735_CS_HIGH;
}

void ST7735_PutString(uint8_t x, uint8_t y, const uint8_t str[], FontDef font, uint16_t charColor, uint16_t bkgColor)
{
	while(*str)
	{
		if(x + font.width >= ST7735_WIDTH)
		{
			x = 0;
			y += font.height;
			if(y + font.height >= ST7735_HEIGHT)
			{
				break;
			}
/*
			if(*str == ' ')
			{
				str++;
				continue;
			}*/
		}

		ST7735_PutChar(x, y, *str, font, charColor, bkgColor);
		x += font.width;
		str++;
	}
}

void ST7735_PutImage(uint8_t x, uint8_t y, uint16_t w, uint16_t h, const uint16_t *img)
{
	const uint8_t *data = (uint8_t *)img;
	if((x >= ST7735_WIDTH) || (y >= ST7735_HEIGHT))
		return;
	if((x + w - 1) >= ST7735_WIDTH)
		return;
	if((y + h - 1) >= ST7735_HEIGHT)
		return;

	ST7735_CS_LOW;
	ST7735_at(x, y, (x + w - 1), (y + h - 1));
	ST7735_DC_HIGH;
	for(uint32_t i = 0; i < (sizeof(uint16_t) * w * h); i++)
	{
		ST7735_SendRAW(data[i]);
	}
	ST7735_CS_HIGH;
}

void ST7735_PutImageR2G3B3(uint8_t x, uint8_t y, uint16_t w, uint16_t h, const uint8_t *img)
{
	if((x >= ST7735_WIDTH) || (y >= ST7735_HEIGHT))
		return;
	if((x + w - 1) >= ST7735_WIDTH)
		return;
	if((y + h - 1) >= ST7735_HEIGHT)
		return;

	ST7735_CS_LOW;
	ST7735_at(x, y, (x + w - 1), (y + h - 1));
	ST7735_DC_HIGH;
	for(uint32_t i = 0; i < (w * h); i++)
	{
		// очень грубое, но простое преобразование R2G3B3 в R5G6B5
		uint8_t data = ((img[i] >> 6) * 10) << 3; // R
		//uint8_t data_g = (((img[i] >> 3) & 0x7) * 9); // G
		data |= ((img[i] >> 3) & 0x7);// G
		//data |= (data_g >> 3);
		ST7735_SendRAW(data);
		data = /*((data_g & 0x7) << 5) | */(img[i] & 0x7) * 4; // B
		ST7735_SendRAW(data);
	}
	ST7735_CS_HIGH;
}

void ST7735_InvertColors(uint8_t enable)
{
	ST7735_CS_LOW;
	ST7735_SendCmd(enable ? 0x21 : 0x20);
	ST7735_CS_HIGH;
}
