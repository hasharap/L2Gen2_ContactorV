/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "OTA.h"
#include "DataFrames.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

uint16_t application_size = 0;
uint16_t application_write_idx = 0;
uint8_t rec_crc[16];
uint32_t RxVal[2];
uint32_t Data[2];
uint8_t Rx_Dataframe[23];
uint16_t index1 = 0;
uint32_t NACK_counter = 0;
OTA_STATE_ ota_state = OTA_STATE_IDLE;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_IWDG_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
static void goto_application(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  //  MX_IWDG_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
    memset(Rx_Dataframe, 0, 23);
 	HAL_UART_Receive_IT(&huart1, Rx_Dataframe, 1);

 	//comment
// 	RxVal[0] = 0x03 ;
//	Write_Data(CONFIGURATION_LOCATION, RxVal, 1, 1);
 	//comment

  	Read_Data(CONFIGURATION_LOCATION, RxVal, 1);



  	if (RxVal[0] == 0x10)
  	{
  		send_response(OTA_ACK);
  		ota_state = OTA_STATE_START;
  		MX_IWDG_Init();
  		while (1);
  	}
  	else if (RxVal[0] == 0x03)
  	{
  		ota_state = OTA_STATE_HANDSHAKE;
  		MX_IWDG_Init();
  		while (1);
  	}
  	else if (RxVal[0] == 0x01)
  	{
  		goto_application();
  	}
  	else
  	{
  		ota_state = OTA_STATE_HANDSHAKE;
  	}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Reload = 780;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PC10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure peripheral I/O remapping */
  __HAL_AFIO_REMAP_USART3_PARTIAL();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	uint16_t data_len = 0;
	uint16_t temp = 0;
 	memset(rec_crc, 0, 16);
	if (huart == &huart1) { // check which UART peripheral generated the interrupt
		HAL_IWDG_Refresh(&hiwdg);
		if (Rx_Dataframe[0] == 0x23 || Rx_Dataframe[0] == 0xA1) {
			HAL_IWDG_Refresh(&hiwdg);
			Rx_Dataframe[index1++] = huart->Instance->DR; // store the received byte in the buffer
			if (Rx_Dataframe[index1 - 1] == 0x0A
					&& (((index1 == 20 || index1 == 9)
							&& ota_state != OTA_STATE_DATA) || index1 == 23)) { // check if the end of a line has been reached
				HAL_IWDG_Refresh(&hiwdg);
				if (ota_state != OTA_STATE_HANDSHAKE) {
					temp = Rx_Dataframe[2];
					Rx_Dataframe[2] = Rx_Dataframe[3];
					Rx_Dataframe[3] = temp;
					data_len = Rx_Dataframe[3] << 8 | Rx_Dataframe[2];
					for (int i = 0; i < data_len; i++) {
						rec_crc[i] = Rx_Dataframe[4 + i];
					}
				}
				frame_Handle(Rx_Dataframe);
				memset(Rx_Dataframe, 0, 23);
				memset(rec_crc, 0, 16);
				index1 = 0;
				HAL_IWDG_Refresh(&hiwdg);
			}
		} else {
			memset(Rx_Dataframe, 0, 23);
			memset(rec_crc, 0, 16);
			index1 = 0;
			HAL_IWDG_Refresh(&hiwdg);
		}
		HAL_UART_Receive_IT(&huart1, &Rx_Dataframe[index1], 1); // enable reception of the next byte
	}
}
void frame_Handle(uint8_t *Frame) {
	switch (ota_state) {
	case OTA_STATE_IDLE:

		break;
	case OTA_STATE_HANDSHAKE:
		OTA_SOM_ *som = (OTA_SOM_*) Frame;
		if (som->sof == OTA_HANDSHAKE && som->crc == 0xC8AA) {
			send_response(0x03);
			ota_state = OTA_STATE_START;
		} else {
			NACK_counter += 1;
			send_response(OTA_NACK);
			ota_state = OTA_STATE_HANDSHAKE;
		}


		break;
	case OTA_STATE_START:
		Data[0] = 0x03;
		Write_Data(CONFIGURATION_LOCATION, Data, 1, 1);
		OTA_COMMAND_ *cmd = (OTA_COMMAND_*) Frame;
		if (cmd->packet_type == OTA_START) {
			if (cmd->crc == calc_CRC(rec_crc, cmd->data_len)) {
				send_response(OTA_ACK);
				ota_state = OTA_STATE_HEADER;
			} else {
				NACK_counter += 1;
				send_response(OTA_NACK);
				ota_state = OTA_STATE_START;
			}
		}
		break;
	case OTA_STATE_HEADER:
		OTA_HEADER_ *header = (OTA_HEADER_*) Frame;
		if (header->packet_type == OTA_APP_SIZE) {
			reverseArray(rec_crc, header->data_len);
			if (header->crc == calc_CRC(rec_crc, header->data_len)) {
				application_size = rec_crc[1] << 8 | rec_crc[0];
				Erase_APP_Slot();
				send_response(OTA_ACK);
				ota_state = OTA_STATE_DATA;
			} else {
				NACK_counter += 1;
				send_response(OTA_NACK);
				ota_state = OTA_STATE_HEADER;
			}
		}
		break;
	case OTA_STATE_DATA:
		OTA_DATA_ *data = (OTA_DATA_*) Frame;
		if (data->packet_type == OTA_DATA) {
			reverseArray(rec_crc, data->data_len);
			if (data->crc == calc_CRC(rec_crc, data->data_len)) {
				Write_Data(
				APPLICATION_LOCATION + application_write_idx, data->data, 4, 0);
				application_write_idx += 16;
				send_response(OTA_ACK);
				if (application_write_idx >= application_size) {
					ota_state = OTA_STATE_END;
				} else {
					ota_state = OTA_STATE_DATA;
				}
			} else {
				NACK_counter += 1;
				send_response(OTA_NACK);
				ota_state = OTA_STATE_DATA;
			}
		}
		break;
	case OTA_STATE_END:
		OTA_COMMAND_ *end = (OTA_COMMAND_*) Frame;
		if (end->packet_type == OTA_END) {
			if (end->crc == calc_CRC(rec_crc, end->data_len)) {
				send_response(OTA_ACK);
				Data[0] = 0x01;
				Write_Data(CONFIGURATION_LOCATION, Data, 1, 1);
				ota_state = OTA_STATE_IDLE;
			} else {
				NACK_counter += 1;
				send_response(OTA_NACK);
				ota_state = OTA_STATE_END;
			}
		}
		break;
	default:
		break;
	}
	if (NACK_counter >= 3) {
		while (1)
			;
	}
}

void send_response(uint8_t is_success) {

	memset(rec_crc, 0, 16);
	rec_crc[0] = is_success;
	OTA_RESP_ response = { .sof = OTA_SOF, .packet_type = OTA_RESPONSE,
			.data_len = 1, .status = is_success, .crc = calc_CRC(rec_crc, 1),
			.eof = OTA_EOF

	};

	HAL_UART_Transmit(&huart1, &response, 8, HAL_MAX_DELAY);

}

static void goto_application(void) {
	void (*app_reset_handler)(
			void) = (void*)(*((volatile uint32_t*)(APPLICATION_LOCATION + 4U)));
	HAL_RCC_DeInit();
	HAL_DeInit();
	__set_MSP(*(volatile uint32_t*) APPLICATION_LOCATION);
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
	app_reset_handler();
}

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
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
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
