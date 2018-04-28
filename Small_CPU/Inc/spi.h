/**
  ******************************************************************************
  * @file    spi.h
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    16-Sept-2014
  * @brief   Header file for spi control
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SPI_H
#define SPI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

extern SPI_HandleTypeDef hspi1;

void MX_SPI1_Init(void);
//void SPI_Start_single_TxRx_with_Master_and_Stop_ChipSelectControl(void);
void SPI_Start_single_TxRx_with_Master(void);
void SPI_synchronize_with_Master(void);
void MX_SPI_DeInit(void);

	 /* button adjust */
void MX_SPI3_Init(void);
void MX_SPI3_DeInit(void);
uint8_t SPI3_ButtonAdjust(uint8_t *arrayInput, uint8_t *arrayOutput);
	 
#ifdef __cplusplus
}
#endif
#endif /* SPI_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
