#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"

SPI_HandleTypeDef SpiHandle;

struct MAX6675_s {	uint8_t input;
				 	float reading;
	 	 	 	 	};

#define MAX6675_CS_PIN LED2_PIN
#define MAX6675_CS_PORT LED2_GPIO_PORT

void MAX6675_Refresh(struct MAX6675_s *max) {

	uint8_t recieve[2];

	HAL_GPIO_WritePin(MAX6675_CS_PORT, MAX6675_CS_PIN, 0);

	HAL_SPI_Receive (&SpiHandle, recieve, 2, 1000);

	HAL_GPIO_WritePin(MAX6675_CS_PORT, MAX6675_CS_PIN, 1);

	max->input = ((recieve[1] & 0b10) == 0b10);

	max->reading = ((uint16_t) (recieve[0] & 0b00111111) * 256 + (recieve[1] & 0b11111100));
	max->reading = max->reading / 16;
}

static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}


int main(void) {
 HAL_Init();
// SystemClock_Config();

 // LED clock initialization
 LED2_GPIO_CLK_ENABLE();

 GPIO_InitTypeDef GPIO_InitStruct;
 GPIO_InitStruct.Pin = LED2_PIN;
 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
 GPIO_InitStruct.Pull = GPIO_PULLUP;
 GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
 HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);

 HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, 1);


 __GPIOB_CLK_ENABLE();

 GPIO_InitTypeDef GPIO_InitStructB;
	 GPIO_InitStructB.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
	 GPIO_InitStructB.Mode = GPIO_MODE_AF_PP;
	 GPIO_InitStructB.Pull = GPIO_NOPULL;
	 GPIO_InitStructB.Speed = GPIO_SPEED_FAST;
	 GPIO_InitStructB.Alternate = GPIO_AF5_SPI1;
 HAL_GPIO_Init(GPIOB, &GPIO_InitStructB);

 __SPI1_CLK_ENABLE();

 SpiHandle.Instance               = SPI1;

 SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
 SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
 SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
 SpiHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
 SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
 SpiHandle.Init.CRCPolynomial     = 7;
 SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
 SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
 SpiHandle.Init.NSS               = SPI_NSS_SOFT;
 SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;

 SpiHandle.Init.Mode = SPI_MODE_MASTER;

 HAL_SPI_Init(&SpiHandle);

 struct MAX6675_s Temp;

 HAL_StatusTypeDef bob;

 uint16_t i;

 while (1) {
	 MAX6675_Refresh(&Temp);

 }
}
