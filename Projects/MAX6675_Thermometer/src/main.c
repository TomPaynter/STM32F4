#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"

SPI_HandleTypeDef SpiHandle;
UART_HandleTypeDef UartHandle;

struct MAX6675_s {	uint8_t input;
				 	float reading;
	 	 	 	 	};

#define MAX6675_CS_PIN LED2_PIN
#define MAX6675_CS_PORT LED2_GPIO_PORT

uint16_t cheeky_pow(uint16_t base, uint8_t power) {
    uint8_t i;
    uint16_t result = base;

    if (power == 0) {
        return 1;
    }

    for (i=0; i<(power - 1); i++) {
        result = result * base;
    }

    return result;
}

void character_binaryString(uint8_t *mastring, uint8_t machar) {

	uint8_t i;

	for(i = 0; i < 8; i++) {
		if ((machar & cheeky_pow(2, 7-i)) > 0) {
			mastring[i] = 1+'0';
		}

		else {
			mastring[i] = 0+'0';
		}
	}
	return;
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



uint8_t float_to_string(uint8_t *output, float value, uint8_t dp)
{
    uint8_t pos = 0,
            digits = 1,
            i=0,
            current_digit;

    if (value < 0) {
        output[0] = (int) "-";
        pos = 1;

        value = value * -1;
    }

    while(value > cheeky_pow(10, digits)){
        digits = digits + 1;
    }

    while(digits > 0) {
        current_digit = (int) value / cheeky_pow(10, (digits - 1));
        output[pos] = '0' + current_digit;
        value = value - current_digit * cheeky_pow(10, (digits - 1));
        digits = digits - 1;
        pos = pos + 1;
    }

    if (dp <= 0) {

        if ((value > 0.5) && (output[pos-1] == '9')) {
                    output[pos-2] = output[pos-2] + 1;
                    output[pos-1] = '0';
                }

        else if (value > 0.5) {
            output[pos-1] = output[pos-1] + 1;
        }

        return pos + 1;
    }

    else {
        output[pos] = '.';
        pos = pos + 1;

        digits = dp;

        value = value * pow(10, dp);

        while(digits > 0) {
            current_digit = (int) value / cheeky_pow(10, (digits - 1));
            output[pos] = '0' + current_digit;
            value = value - current_digit * cheeky_pow(10, (digits - 1));
            digits = digits - 1;
            pos = pos + 1;
        }

        if ((value > 0.5) && (output[pos-1] == '9')) {
            output[pos-2] = output[pos-2] + 1;
            output[pos-1] = '0';
        }

        else if (value > 0.5) {
            output[pos-1] = output[pos-1] + 1;
        }

    }

return pos + 1;
}

void MAX6675_Refresh(struct MAX6675_s *max) {

	uint8_t recieve[2], i;

	HAL_GPIO_WritePin(MAX6675_CS_PORT, MAX6675_CS_PIN, 0);

	HAL_SPI_Receive (&SpiHandle, recieve, 2, 1000);

	HAL_GPIO_WritePin(MAX6675_CS_PORT, MAX6675_CS_PIN, 1);

	uint8_t bob1[] = "Reading 0: ";
	uint8_t bob2[] = "   Reading 1: ";
	uint8_t enda[] = "\r\n";

    HAL_UART_Transmit(&UartHandle, bob1, 11, 100);


	 uint8_t binary[8] = {0,0,0,0,0,0,0,0};

	character_binaryString(binary, recieve[0]);
	HAL_UART_Transmit(&UartHandle, binary, 8, 100);

    HAL_UART_Transmit(&UartHandle, bob2, 14, 100);

	character_binaryString(binary, recieve[1]);
	HAL_UART_Transmit(&UartHandle, binary, 8, 100);



	max->input = ((recieve[1] & 0b10) == 0b10);

	max->reading = ((uint16_t) (recieve[0] & 0b00111111) * 256 + (recieve[1] & 0b11111100));
	max->reading = max->reading / 16;

	uint8_t tempfloat[10], m, bob3[] = "  Temp: ";

	 m = float_to_string(tempfloat, max->reading, 3);
		HAL_UART_Transmit(&UartHandle, bob3, 8, 100);

		HAL_UART_Transmit(&UartHandle, tempfloat, m, 100);

	HAL_UART_Transmit(&UartHandle, enda, 2, 100);

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


 __GPIOA_CLK_ENABLE();

 GPIO_InitTypeDef GPIO_InitStructUART;
 GPIO_InitStructUART.Pin = GPIO_PIN_2 | GPIO_PIN_3;
 GPIO_InitStructUART.Mode = GPIO_MODE_AF_PP;
     //GPIO_InitStruct.Pull = GPIO_NOPULL;
 GPIO_InitStructUART.Speed = GPIO_SPEED_FAST;
 GPIO_InitStructUART.Alternate = GPIO_AF7_USART2;
 HAL_GPIO_Init(GPIOA, &GPIO_InitStructUART);




 __USART2_CLK_ENABLE();

     UartHandle.Instance = USART2;
     UartHandle.Init.BaudRate = 115200;
     UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
     UartHandle.Init.StopBits = UART_STOPBITS_1;
     UartHandle.Init.Parity = UART_PARITY_NONE;
     UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
     UartHandle.Init.Mode = UART_MODE_TX_RX;

 HAL_UART_Init(&UartHandle);



 HAL_SPI_Init(&SpiHandle);

 struct MAX6675_s Temp;


 while (1) {
	 MAX6675_Refresh(&Temp);

	 Temp.reading = 0;

     uint16_t i;
     for (i = 0; i < 60000; i++);
     for (i = 0; i < 60000; i++);
     for (i = 0; i < 60000; i++);
     for (i = 0; i < 60000; i++);
 }
}
