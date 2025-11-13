#include "OTA.h"
#include <stdbool.h>
#include <stdio.h>

uint32_t crc_val = 400;

static uint32_t GetPage(uint32_t Address) {
	for (int indx = 0; indx < 128; indx++) {
		if ((Address < (0x08000000 + (FLASH_PAGE_SIZE * (indx + 1))))
				&& (Address >= (0x08000000 + FLASH_PAGE_SIZE * indx))) {
			return (0x08000000 + FLASH_PAGE_SIZE * indx);
		}
	}

	return 0;
}

/**
 * @brief Write data into Flash memory.
 * @param StartAddress: Starting address in Flash memory where data will be written.
 * @param Data: Pointer to the data array to be written into Flash memory.
 * @param WordSize: Number of words (32-bit) to be written.
 * @param is_first_word: Indicates whether it's the first word being written, used for Flash erase operation.
 * @retval HAL_FLASH status/error code.
 */
uint32_t Write_Data(uint32_t StartAddress, uint32_t *Data, uint16_t WordSize,
bool is_first_word) {
	static FLASH_EraseInitTypeDef EraseInitStruct; // Static structure for Flash erase configuration
	uint32_t PAGEError; // Error code for Flash erase operation
	int sofar = 0; // Counter for the number of words programmed so far

	/* Unlock the Flash to enable the flash control register access */
	HAL_FLASH_Unlock();

	/* Erase the user Flash area if it's the first word being written */
	if (is_first_word) {
		printf("Erasing the Flash memory...\r\n");

		// Calculate start and end page addresses for erase operation
		uint32_t StartPage = GetPage(StartAddress);
		uint32_t EndPageAddress = StartAddress + WordSize * 4;
		uint32_t EndPage = GetPage(EndPageAddress);

		// Fill erase configuration structure
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		EraseInitStruct.PageAddress = StartPage;
		EraseInitStruct.NbPages = ((EndPage - StartPage) / FLASH_PAGE_SIZE) + 1;

		// Erase Flash pages
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
			/* Error occurred while page erase */
			return HAL_FLASH_GetError();
		}

		printf("Erased the Flash memory...\r\n");
	}

	/* Program the user Flash area word by word */
	while (sofar < WordSize) {
		// Program a word into Flash memory
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, StartAddress, Data[sofar])
				== HAL_OK) {
			StartAddress += 4; // Advance address by word size (4 bytes)
			sofar++; // Increment the count of words programmed
		} else {
			/* Error occurred while writing data in Flash memory */
			return HAL_FLASH_GetError();
		}
	}

	/* Lock the Flash to disable the flash control register access */
	HAL_FLASH_Lock();

	return 0; // Success
}

/**
 * @brief Read data from Flash memory.
 * @param StartAddress: Starting address in Flash memory from where data will be read.
 * @param RxBuf: Pointer to the buffer where the read data will be stored.
 * @param WordSize: Number of words (32-bit) to be read.
 */
void Read_Data(uint32_t StartAddress, uint32_t *RxBuf, uint16_t WordSize) {
	// Continuously read data until WordSize becomes 0
	while (1) {
		// Read a word from Flash memory and store it in the buffer
		*RxBuf = *(__IO uint32_t*) StartAddress;
		StartAddress += 4; // Move to the next word address
		RxBuf++; // Move to the next location in the buffer

		// Decrement WordSize and check if it reaches 0
		if (!(WordSize--))
			break; // If WordSize becomes 0, exit the loop
	}
}

void Erase_APP_Slot() {

	HAL_FLASH_Unlock();
	static FLASH_EraseInitTypeDef EraseInitStruct; // Static structure for Flash erase configuration
	uint32_t PAGEError;

	// Fill erase configuration structure
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = APPLICATION_LOCATION;
	EraseInitStruct.NbPages = 43;

	// Erase Flash pages
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
		/* Error occurred while page erase */
		return HAL_FLASH_GetError();
	}
	HAL_FLASH_Lock();
}

