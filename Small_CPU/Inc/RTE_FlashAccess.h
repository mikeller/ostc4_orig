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

/// split word to 2 bytes
typedef struct{
uint8_t low; ///< split word to 2 bytes
uint8_t hi; ///< split word to 2 bytes
} two_byte;


/// split word to 2 bytes
typedef union{
two_byte Byte; ///< split word to 2 bytes
uint16_t Word; ///< split word to 2 bytes
} tword;


/// split signed word to 2 bytes
typedef union{
two_byte Byte; ///< split signed word to 2 bytes
int16_t Word; ///< split signed word to 2 bytes
} signed_tword;


/// split full32 to 2 words
typedef struct{
uint16_t low16; ///< split word to 2 bytes
uint16_t hi16; ///< split word to 2 bytes
} two_word;

typedef union{
two_word Word16; ///< split word to 2 bytes
uint32_t Full32; ///< split word to 2 bytes
} tfull32;

	 /* Exported functions --------------------------------------------------------*/

uint8_t BFA_readLastDataBlock(tfull32 *dataArray4);
uint8_t BFA_writeDataBlock(const tfull32 *dataArray4);
uint16_t BFA_calc_Block_Checksum(const tfull32 *dataArray4);

#ifdef __cplusplus
}
#endif

#endif /* RTE_FLASH_ACCESS_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
