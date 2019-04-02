/**
  ******************************************************************************
  * @file    uart.h
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    27-March-2014
  * @brief   button control
  *           
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UART_H
#define UART_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"

void MX_USART2_UART_Init(void);
void MX_USART2_UART_DeInit(void);
uint8_t UART_ButtonAdjust(uint8_t *array);


#ifdef __cplusplus
}
#endif

#endif /* UART_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
