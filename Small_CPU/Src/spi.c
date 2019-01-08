/**
 ******************************************************************************
 * @file    spi.c
 * @author  heinrichs weikamp gmbh
 * @version V0.0.1
 * @date    16-Sept-2014
 * @brief   Source code for spi control
 *
 @verbatim
 ==============================================================================
 ##### How to use #####
 ==============================================================================
 @endverbatim
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2014 heinrichs weikamp</center></h2>
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include "dma.h"
//#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "scheduler.h"

#ifdef DEBUG_GPIO
extern void GPIO_new_DEBUG_LOW(void);
extern void GPIO_new_DEBUG_HIGH(void);
#endif

uint8_t data_error = 0;
uint32_t data_error_time = 0;

static void SPI_Error_Handler(void);

/* USER CODE END 0 */

static uint8_t SPI_check_header_and_footer_ok(void);

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;

DMA_HandleTypeDef hdma_tx;
DMA_HandleTypeDef hdma_rx;

// SPI3 init function
void MX_SPI3_Init(void) {
	hspi3.Instance = SPI3;
	hspi3.Init.Mode = SPI_MODE_MASTER;
	hspi3.Init.Direction = SPI_DIRECTION_2LINES;
	hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi3.Init.CLKPolarity = SPI_POLARITY_HIGH;
	hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi3.Init.NSS = SPI_NSS_SOFT;
	hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi3.Init.TIMode = SPI_TIMODE_DISABLED;
	hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
	hspi3.Init.CRCPolynomial = 7;
	HAL_SPI_Init(&hspi3);
}

void MX_SPI3_DeInit(void) {
	HAL_SPI_DeInit(&hspi3);
}

uint8_t SPI3_ButtonAdjust(uint8_t *arrayInput, uint8_t *arrayOutput) {
	HAL_StatusTypeDef status;
	uint8_t answer[10];
	uint8_t rework[10];

	rework[0] = 0xFF;
	for (int i = 0; i < 3; i++) {
		// limiter
		if (arrayInput[i] == 0xFF)
			arrayInput[i] = 0xFE;
		if (arrayInput[i] >= 15) {
			// copy - ausl�se-schwelle
			rework[i + 1] = arrayInput[i];
			// wieder-scharf-schalte-schwelle
			rework[i + 3 + 1] = arrayInput[i] - 10;
		} else if (arrayInput[i] >= 10) {
			// copy - ausl�se-schwelle
			rework[i + 1] = arrayInput[i];
			// wieder-scharf-schalte-schwelle
			rework[i + 3 + 1] = arrayInput[i] - 5;
		} else {
			// copy - ausl�se-schwelle
			rework[i + 1] = 7;
			// wieder-scharf-schalte-schwelle
			rework[i + 3 + 1] = 6;
		}
	}

	status = HAL_OK; /* = 0 */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	for (int i = 0; i < 7; i++) {
		HAL_Delay(10);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
		HAL_Delay(10);
		status += HAL_SPI_TransmitReceive(&hspi3, &rework[i], &answer[i], 1,
				20);
		HAL_Delay(10);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	}

	if (status == HAL_OK) {
		for (int i = 0; i < 3; i++) {
			arrayOutput[i] = answer[i + 2]; // first not, return of 0xFF not
		}
		return 1;
	} else

		return 0;
}

// SPI5 init function
void MX_SPI1_Init(void) {
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_SLAVE;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_HARD_INPUT; //SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED; //_DISABLED; _ENABLED;
	hspi1.Init.CRCPolynomial = 7;
	HAL_SPI_Init(&hspi1);
}

void MX_SPI_DeInit(void) {
	HAL_SPI_DeInit(&hspi1);
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {

	GPIO_InitTypeDef GPIO_InitStruct;

	if (hspi->Instance == SPI1) {
		// Peripheral clock enable
		__SPI1_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();
		//SPI1 GPIO Configuration  
		//PA4   ------> SPI1_CS 
		//PA5   ------> SPI1_SCK
		//PA6   ------> SPI1_MISO 
		//PA7   ------> SPI1_MOSI 

		GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
//    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
		GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		//##-3- Configure the DMA streams ##########################################
		// Configure the DMA handler for Transmission process 
		hdma_tx.Instance = DMA2_Stream3;
		hdma_tx.Init.Channel = DMA_CHANNEL_3;
		hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_tx.Init.Mode = DMA_NORMAL;
		hdma_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
		hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		hdma_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
		hdma_tx.Init.MemBurst = DMA_MBURST_INC4;
		hdma_tx.Init.PeriphBurst = DMA_PBURST_INC4;

		HAL_DMA_Init(&hdma_tx);

		// Associate the initialized DMA handle to the the SPI handle
		__HAL_LINKDMA(hspi, hdmatx, hdma_tx);

		// Configure the DMA handler for Transmission process
		hdma_rx.Instance = DMA2_Stream0;
		hdma_rx.Init.Channel = DMA_CHANNEL_3;
		hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_rx.Init.Mode = DMA_NORMAL;
		hdma_rx.Init.Priority = DMA_PRIORITY_HIGH;
		hdma_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		hdma_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
		hdma_rx.Init.MemBurst = DMA_MBURST_INC4;
		hdma_rx.Init.PeriphBurst = DMA_PBURST_INC4;

		HAL_DMA_Init(&hdma_rx);

		// Associate the initialized DMA handle to the the SPI handle
		__HAL_LINKDMA(hspi, hdmarx, hdma_rx);

		//##-4- Configure the NVIC for DMA #########################################
		//NVIC configuration for DMA transfer complete interrupt (SPI3_RX)
		HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

		// NVIC configuration for DMA transfer complete interrupt (SPI1_TX)
		HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 1, 1);
		HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
	} else if (hspi->Instance == SPI3) {
		__GPIOC_CLK_ENABLE();
		__SPI3_CLK_ENABLE();

		//SPI1 GPIO Configuration  
		//PC10   ------> SPI3_SCK
		//PC11   ------> SPI3_MISO 
		//PC12   ------> SPI3_MOSI 
		//PA15   ------> SPI3_NSS (official)
		//PC9    ------> SPI3_NSS (hw)

		GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
	}
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
	if (hspi->Instance == SPI1) {
		__SPI1_FORCE_RESET();
		__SPI1_RELEASE_RESET();

		//SPI1 GPIO Configuration  
		//PA5   ------> SPI1_SCK
		//PA6   ------> SPI1_MISO 
		//PA7   ------> SPI1_MOSI 

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

		HAL_DMA_DeInit(&hdma_tx);
		HAL_DMA_DeInit(&hdma_rx);

		HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);
		HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
	} else if (hspi->Instance == SPI3) {
		__SPI3_FORCE_RESET();
		__SPI3_RELEASE_RESET();

		//SPI1 GPIO Configuration  
		//PC10   ------> SPI3_SCK
		//PC11   ------> SPI3_MISO 
		//PC12   ------> SPI3_MOSI 
		//PA15   ------> SPI3_NSS (official)
		//PC9    ------> SPI3_NSS (hw)
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
	}
}

void SPI_synchronize_with_Master(void) {
//	GPIO_InitTypeDef GPIO_InitStruct;
//
//	__GPIOA_CLK_ENABLE();
//	/**SPI1 GPIO Configuration
//	 PA5   ------> SPI1_SCK
//	 */
//	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
//	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//	GPIO_InitStruct.Pull = GPIO_PULLUP;
//	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
//	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//	HAL_Delay(10);
//	while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == 0);
//	HAL_Delay(10);
//	while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == 1);
//	HAL_Delay(20);
}

void SPI_Start_single_TxRx_with_Master(void) {
	uint8_t * pOutput;

	if (global.dataSendToSlave.getDeviceDataNow) {
		global.dataSendToSlave.getDeviceDataNow = 0;
		pOutput = (uint8_t*) &(global.deviceDataSendToMaster);
	} else {
		pOutput = (uint8_t*) &(global.dataSendToMaster);
	}

	if (HAL_SPI_TransmitReceive_DMA(&hspi1, pOutput,(uint8_t*) &(global.dataSendToSlave), EXCHANGE_BUFFERSIZE)!= HAL_OK) {
		// Transfer error in transmission process
		SPI_Error_Handler();
	}
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	/* restart SPI */
	if (hspi == &hspi1) {

		global.check_sync_not_running = 0;
		/* stop data exchange? */
		if (global.mode == MODE_SHUTDOWN) {
			global.mode = MODE_SLEEP;
			global.dataSendToSlavePending = 0;
			global.dataSendToSlaveIsValid = 1;
			global.dataSendToSlaveIsNotValidCount = 0;
			return;
		}

		/* data consistent? */
		if (SPI_check_header_and_footer_ok()) {
//		GPIO_new_DEBUG_HIGH(); //For debug.
			global.dataSendToSlaveIsValid = 1;
			global.dataSendToSlaveIsNotValidCount = 0;
		} else {
//		GPIO_new_DEBUG_LOW(); //For debug.
			global.dataSendToSlaveIsValid = 0;
			global.dataSendToSlaveIsNotValidCount++;
			MX_SPI_DeInit();
			HAL_Delay(30);
			MX_DMA_Init();
			MX_SPI1_Init();
		}
		global.dataSendToMaster.power_on_reset = 0;
		global.deviceDataSendToMaster.power_on_reset = 0;

//TODO:REMOVE
//		if ( !global.dataSendToSlaveStopEval ) {
//			scheduleSpecial_Evaluate_DataSendToSlave();
//		}
		scheduleSpecial_Evaluate_DataSendToSlave();
		SPI_Start_single_TxRx_with_Master(); //Send data always.
	}
}

static uint8_t SPI_check_header_and_footer_ok(void) {
	if (global.dataSendToSlave.header.checkCode[0] != 0xBB)
		return 0;
	if (global.dataSendToSlave.header.checkCode[1] != 0x01)
		return 0;
	if (global.dataSendToSlave.header.checkCode[2] != 0x01)
		return 0;
	if (global.dataSendToSlave.header.checkCode[3] != 0xBB)
		return 0;
	if (global.dataSendToSlave.footer.checkCode[0] != 0xF4)
		return 0;
	if (global.dataSendToSlave.footer.checkCode[1] != 0xF3)
		return 0;
	if (global.dataSendToSlave.footer.checkCode[2] != 0xF2)
		return 0;
	if (global.dataSendToSlave.footer.checkCode[3] != 0xF1)
		return 0;

	return 1;
}

static void SPI_Error_Handler(void) {
	//The device is locks. Hard to recover.
//  while(1)
//  {
//  }
}

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
