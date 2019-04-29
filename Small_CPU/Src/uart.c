/**
  ******************************************************************************
  * @file    uart.c 
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    27-March-2014
  * @brief   button control
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
/* Includes ------------------------------------------------------------------*/
#include "uart.h"

/* Private variables ---------------------------------------------------------*/

UART_HandleTypeDef huart2;


/* Exported functions --------------------------------------------------------*/

void MX_USART2_UART_Init(void)
{
/* pullup special */
  GPIO_InitTypeDef   GPIO_InitStructure;
  __GPIOA_CLK_ENABLE();
  GPIO_InitStructure.Pin = GPIO_PIN_2;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 

/* regular init */	
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 1200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart2);
}


uint8_t UART_ButtonAdjust(uint8_t *array)
{
	uint8_t answer[4];
	
	HAL_UART_Transmit(&huart2,array,4,1000);
	HAL_UART_Receive(&huart2,answer,4,2000);
	if(	(answer[0] == array[0])
		&&(answer[1] == array[1])
		&&(answer[2] == array[2])
		&&(answer[3] == array[3]))
	return 1;
	else
	return 0;
}

void MX_USART2_UART_DeInit(void)
{
	HAL_UART_DeInit(&huart2);
}


/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
