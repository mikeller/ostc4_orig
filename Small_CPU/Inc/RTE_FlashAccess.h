/**
  ******************************************************************************
  * @file    RTE_FLashAccess.h based on BonexFLashAccess.h based on firmwareEraseProgram.h  
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    20-July-2016
  * @version V0.0.1
  * @since   20-July-2016
  * @brief   erase and program the STM32F4xx internal FLASH memory for compasss calib etc.
  *
  @verbatim
  ==============================================================================
                        ##### How to use #####
  ==============================================================================
  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef RTE_FLASH_ACCESS_H
#define RTE_FLASH_ACCESS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported variables --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
enum
{
	BFA_OK = 0,		//= HAL_OK
	BFA_ERROR 	= (uint8_t)HAL_ERROR,
	BFA_BUSY 		= (uint8_t)HAL_BUSY,
	BFA_TIMEOUT = (uint8_t)HAL_TIMEOUT,
	BFA_EMPTY,
};

	 /* Exported functions --------------------------------------------------------*/

uint8_t BFA_readLastDataBlock(uint32_t *dataArray4);
uint8_t BFA_writeDataBlock(const uint32_t *dataArray4);

#ifdef __cplusplus
}
#endif

#endif /* RTE_FLASH_ACCESS_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
