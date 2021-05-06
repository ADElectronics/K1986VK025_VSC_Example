#include "MLDR187_lib.h"
#include "MLDR187.h"

#include "MLDR187_bkp.h"
#include "MLDR187_gpio.h"
#include "MLDR187_ssp.h"
#include "MLDR187_adc.h"
#include "MLDR187_it.h"
#include "MLDR187_rst_clk.h"
#include "MLDR187_eeprom.h"

#include "ST7735.h"
#include "riscv_math.h"
#include "riscv_const_structs.h"
#include "window_table.h"

/* Private define ------------------------------------------------------------*/
#define LED_PORT              MDR_PORTB
#define LED_PORT_CLK          RST_CLK_PORTB
#define LED_PIN_0             PORT_Pin_3
#define LED_PIN_1             PORT_Pin_4
#define LED_PIN_2             PORT_Pin_5

#define TEST_LENGTH_SAMPLES   160
#define TEST_FFT_SIZE         128

#define MDR_PLIC_PR8         ((volatile uint32_t*)(MDR_PLIC_PRI_BASE + 4*8))[0]

/* Private variables ---------------------------------------------------------*/
PORT_InitTypeDef PORT_InitStructure;
RST_CLK_CpuSpeed_InitTypeDef RST_CLK_CpuSpeed_InitStructure;
SSP_InitTypeDef SSP_InitStruct;
ADC_InitTypeDef ADC_InitStruct;


static q15_t testInput[TEST_FFT_SIZE * 2];
static q15_t testOutput[TEST_FFT_SIZE];
uint16_t adc_raw[TEST_LENGTH_SAMPLES];
uint16_t adc_current = 0;
uint16_t adc_ready = 0;
static uint8_t lcd_data[ST7735_WIDTH * ST7735_HEIGHT];

/* Private functions ---------------------------------------------------------*/

void update_lcd_fft()
{
   for(uint32_t i = 0; i < TEST_FFT_SIZE; i++)
   {
      // 10 бит = 1024
      // AVREF = 3.3V
      // MIC BIAS = 1.25V
      // MIC BIAS OFFSET ~ 388
      testInput[i * 2 + 0] = (q15_t)((adc_raw[i] - 388) << 8);
      testInput[i * 2 + 1] = 0x00;
   }

   riscv_mult_q15((q15_t *)&testInput, (q15_t *)&window_table, (q15_t *)&testInput, TEST_FFT_SIZE);
   riscv_cfft_q15(&riscv_cfft_sR_q15_len128, (q15_t *)&testInput, 0, 1);
   riscv_cmplx_mag_q15((q15_t *)&testInput, (q15_t *)&testOutput, TEST_FFT_SIZE);

   for(uint32_t h = 0; h < ST7735_HEIGHT; h++)
   {
      for(uint32_t i = 0; i < (TEST_LENGTH_SAMPLES / 2 - 1); i++)
         lcd_data[i + ST7735_WIDTH * h] = ((testOutput[i] / 100 ) >= h) ? 0xff : 0x00;
   }
}

void update_lcd_clear()
{
   for(uint32_t h = 0; h < ST7735_HEIGHT; h++)
   {
      for(uint32_t i = 0; i < ST7735_WIDTH; i++)
         lcd_data[i + ST7735_WIDTH * h] = 0x00;
   }
}
/*
void ADC_IRQHandler(void)
{
   adc_raw[adc_current] = ADC_GetResult();
   adc_current++;

   if(adc_current >= TEST_LENGTH_SAMPLES)
   {
      adc_current = 0;
      adc_ready = 1;
   }
}
*/
int main()
{
   MDR_RST_CLK->CPU_CLOCK &= RST_CLK_CPU_CLOCK_HCLKSEL_Msk; // HCLK = HSI

   RST_CLK_EnablePeripheralClock(RST_CLK_EEPROM, RST_CLK_Div1);
   EEPROM_SetLatency(flashCoreSpeedUpTo60MHz);

   RST_CLK_CpuSpeed_InitStructure.hseState = RST_CLK_HseOscillator;
   RST_CLK_CpuSpeed_InitStructure.hseSpeed = 16000000;
   RST_CLK_CpuSpeed_InitStructure.cpuC1Src = RST_CLK_CpuC1SelHse;
   RST_CLK_CpuSpeed_InitStructure.pllState = RST_CLK_PllFromHse;
   RST_CLK_CpuSpeed_InitStructure.pllMult = 2;
   RST_CLK_CpuSpeed_InitStructure.cpuC2Src = RST_CLK_CpuC2SelPllCpu;
   RST_CLK_CpuSpeed_InitStructure.cpuC3Div = RST_CLK_Div1;
   RST_CLK_CpuSpeed_InitStructure.hclkSrc = RST_CLK_HclkSelCpuC3;
   RST_CLK_SetupCpuSpeed(&RST_CLK_CpuSpeed_InitStructure);
   RST_CLK_SetupHsPeripheralClock(RST_CLK_Clk_PER1_C2, RST_CLK_ClkSrc_PLLCPU);

   RST_CLK_EnablePeripheralClock(LED_PORT_CLK, RST_CLK_Div1);
   RST_CLK_EnablePeripheralClock(RST_CLK_PORTD, RST_CLK_Div1);

   PORT_InitStructure.PORT_OE = PORT_OE_OUT;
   PORT_InitStructure.PORT_FUNC = PORT_FUNC_PORT;
   PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
   PORT_InitStructure.PORT_SPEED = PORT_SPEED_SLOW_4mA;
   PORT_InitStructure.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
   PORT_Init(LED_PORT, (LED_PIN_0 | LED_PIN_1 | LED_PIN_2), &PORT_InitStructure);
   PORT_SetReset(LED_PORT, LED_PIN_0, SET);
   PORT_SetReset(LED_PORT, LED_PIN_1, RESET);
   PORT_SetReset(LED_PORT, LED_PIN_2, RESET);

   PORT_InitStructure.PORT_FUNC = PORT_FUNC_PORT;
   PORT_InitStructure.PORT_SPEED = PORT_SPEED_FAST_2mA;
   PORT_Init(MDR_PORTD, (PORT_Pin_0 | PORT_Pin_4 | PORT_Pin_9), &PORT_InitStructure);

   PORT_InitStructure.PORT_OE = PORT_OE_OUT;
   PORT_InitStructure.PORT_FUNC = PORT_FUNC_MAIN;
   PORT_InitStructure.PORT_SPEED = PORT_SPEED_FAST_2mA;
   PORT_Init(MDR_PORTD, (PORT_Pin_1 | PORT_Pin_3), &PORT_InitStructure);

   PORT_InitStructure.PORT_OE = PORT_OE_IN;
   PORT_Init(MDR_PORTD, (PORT_Pin_2), &PORT_InitStructure);

   SSP_StructInit(&SSP_InitStruct);
   SSP_InitStruct.clkDivisor = RST_CLK_Div1;
   SSP_Init(ST7735_SPI ,&SSP_InitStruct);
   SSP_Cmd(ST7735_SPI, ENABLE);

   ADC_StructInitDefault(&ADC_InitStruct);
   ADC_InitStruct.clkSrc = RST_CLK_ClkSrc_PLLCPU;
   ADC_InitStruct.clkDiv = RST_CLK_Div64;
   ADC_InitStruct.autoCont = ENABLE;
   ADC_InitStruct.chnl = ADC_CHNL_AIN3;
   ADC_Init(&ADC_InitStruct); // не дописаны функции в SPL...
   MDR_ADC->CFG = 0x0000a04c; // ручками выставляем нужные биты
   ADC_Cmd(ENABLE);

   ST7735_Init();
   update_lcd_clear();
   PORT_SetReset(LED_PORT, LED_PIN_0, RESET);
   PORT_SetReset(LED_PORT, LED_PIN_1, SET);

   //MDR_ADC->STATUS |= ADC_STATUS_ECOIF_IE; // ECOIF IRQ
   //MDR_PLIC_PR8 = 0x5;
   //PLIC_EnableIRQ(8);
   ADC_Start();

   while (1)
   {
      if(MDR_ADC->STATUS & (1 << 2)) // EOCIF
      {
         adc_raw[adc_current] = ADC_GetResult();
         adc_current++;

         if(adc_current >= TEST_LENGTH_SAMPLES)
         {
            adc_current = 0;
            adc_ready = 1;
         }
      }

      if(adc_ready)
      {
         adc_ready = 0;
         MDR_ADC->CFG &= ~ADC_CFG_GO; //ADC_Stop();
         PORT_SetReset(LED_PORT, LED_PIN_2, SET);
         update_lcd_fft();
         ST7735_PutImageR2G3B3(0, 0, ST7735_WIDTH, ST7735_HEIGHT, (uint8_t *)&lcd_data);
         PORT_SetReset(LED_PORT, LED_PIN_2, RESET);
         ADC_Start();
      }
   }
}
