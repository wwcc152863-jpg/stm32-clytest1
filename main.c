/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "DS18B20.h"
#include "delay.h"
#include "OLED.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define WATER_PRESENT           90
#define WATER_ADC_DRY           3850
#define WATER_ADC_WET           4095
#define LIGHT_THRESHOLD         50
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t water_level;

void MX_IWDG_Init(void)
{
	IWDG->KR = 0x5555;
	IWDG->PR = 4;
	IWDG->RLR = 1250;
	IWDG->KR = 0xAAAA;
	IWDG->KR = 0xCCCC;
}

void LED_Set_State(uint8_t x)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, (GPIO_PinState)x);
}

void Buzzer_Set_State(uint8_t x)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, x ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

float temp_value;
uint8_t light, turbidity;
uint16_t water_adc_raw;
char s[100];

void ADC_GetValue(void)
{
	uint32_t adc_sum[3] = {0};
	uint8_t i;
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;

	for (i = 0; i < 10; i++)
	{
		sConfig.Channel = ADC_CHANNEL_0;
		HAL_ADC_ConfigChannel(&hadc1, &sConfig);
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 10);
		adc_sum[0] += HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);

		sConfig.Channel = ADC_CHANNEL_1;
		HAL_ADC_ConfigChannel(&hadc1, &sConfig);
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 10);
		adc_sum[1] += HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);

		sConfig.Channel = ADC_CHANNEL_2;
		HAL_ADC_ConfigChannel(&hadc1, &sConfig);
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 10);
		adc_sum[2] += HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);
	}
	light = 100 - (adc_sum[0] / 10 / 4095.0 * 100);
	{
		int32_t raw = adc_sum[1] / 10;
		water_adc_raw = raw;
		if (raw < WATER_ADC_DRY) raw = WATER_ADC_DRY;
		if (raw > WATER_ADC_WET) raw = WATER_ADC_WET;
		water_level = (raw - WATER_ADC_DRY) * 100 / (WATER_ADC_WET - WATER_ADC_DRY);
	}
	turbidity = 100 - adc_sum[2] / 10 / 4095.0 * 100;
}

void AutoCtrl(void)
{
	(water_level >= WATER_PRESENT) ? Buzzer_Set_State(0) : Buzzer_Set_State(1);
	(light < LIGHT_THRESHOLD) ? LED_Set_State(1) : LED_Set_State(0);
}

void Page(void)
{
	OLED_ShowString(1, 1, "Temp:", 8);
	sprintf(s, "%.1fC ", temp_value);
	OLED_ShowString(40, 1, s, 8);

	OLED_ShowString(1, 17, "Water:", 8);
	OLED_ShowString(40, 17, water_level >= WATER_PRESENT ? "Yes  " : "No   ", 8);

	OLED_ShowString(1, 33, "Light:", 8);
	sprintf(s, "%d%%  ", light);
	OLED_ShowString(40, 33, s, 8);

	OLED_ShowString(1, 49, "Turb:", 8);
	sprintf(s, "%d%%  ", turbidity);
	OLED_ShowString(40, 49, s, 8);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();

  LED_Set_State(0);
  Buzzer_Set_State(0);
  HAL_ADCEx_Calibration_Start(&hadc1);
	

  MX_IWDG_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    ADC_GetValue();
    temp_value = DS18B20_GetTemp();
    AutoCtrl();
    Page();
    OLED_Update();

    IWDG->KR = 0xAAAA;
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
