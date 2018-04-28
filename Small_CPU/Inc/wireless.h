/**
  ******************************************************************************
  * @file    wireless.h
  * @author  heinrichs weikamp gmbh
  * @date    02-July-2015
  * @version V0.0.1
  * @since   02-July-2015
  * @brief   Data transfer via magnetic field using Manchester code
  *           
  @verbatim                 
  ============================================================================== 
                        ##### How to use #####
  ============================================================================== 
  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef WIRELESS_H
#define WIRELESS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

void wireless_init(void);

void wireless_trigger_FallingEdgeSignalHigh(void);
void wireless_trigger_RisingEdgeSilence(void);
	 
uint8_t wireless_evaluate(uint8_t *dataOut, uint8_t maxData, uint8_t *confidence);
uint8_t wireless_evaluate_crc_error(uint8_t *dataIn, uint8_t maxData);
	 
#ifdef __cplusplus
}
#endif

#endif /* WIRELESS_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
