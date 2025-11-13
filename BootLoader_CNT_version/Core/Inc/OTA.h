/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : OTA.h
 * @brief          : Header for OTA.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/

#include "stm32f1xx_hal.h"
#include <stdbool.h>

/* Memory Locations */
#define BOOTLOADER_LOCATION 	0x08000000
#define CONFIGURATION_LOCATION 	0x08002C00
#define APPLICATION_LOCATION 	0x08003800

extern uint32_t crc_val;

void Erase_APP_Slot();

uint32_t Write_Data(uint32_t StartAddress, uint32_t *Data, uint16_t WordSize,
bool is_first_word);

void Read_Data(uint32_t StartAddress, uint32_t *RxBuff, uint16_t WordSize);

