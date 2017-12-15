/**
  ******************************************************************************
  * @file    data_exchange_main.c
  * @author  heinrichs/weikamp, Christian Weikamp
  * @date    13-Oct-2014
  * @version V0.0.2
  * @since   27-May-2015

	* @brief   Communication with the second CPU == RTE system
  *
  @verbatim
  ==============================================================================
                        ##### How to use #####
  ==============================================================================

  ==============================================================================
                        ##### Device Data #####
  ==============================================================================
	
	main CPU always sends the device data info that it has at the moment

		on start it is INT32_MIN, INT32_MAX and 0 
		as initialized  in data_central.c variable declaration
	
	second small CPU gets request to send its device data
		
		on receiption the data is merged with the data in externLogbookFlash,
		stored on the externLogbookFlash and from now on send to small CPU

	@endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h> // for memcopy
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "ostc.h"
#include "data_central.h"
#include "data_exchange_main.h"
#include "base.h"
#include "externLogbookFlash.h"


/* Expoted variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

SDataReceiveFromMaster dataOut;
SDataExchangeSlaveToMaster dataIn;

uint8_t data_old__lost_connection_to_slave_counter_temp = 0;
/* Private types -------------------------------------------------------------*/

uint8_t DataEX_check_header_and_footer_ok(void);
void DataEX_control_connection_while_asking_for_sleep(void);

/* Exported functions --------------------------------------------------------*/

uint8_t DataEX_call(void)
{
	DataEX_control_connection_while_asking_for_sleep();
	
	for(int i=0;i<EXCHANGE_BUFFERSIZE;i++)
		*(uint8_t *)(((uint32_t)&dataOut) + i)  = 0;

	dataOut.mode = MODE_SHUTDOWN;

	dataOut.header.checkCode[0] = 0xBB;
	dataOut.header.checkCode[1] = 0x01;
	dataOut.header.checkCode[2] = 0x01;
	dataOut.header.checkCode[3] = 0xBB;

	dataOut.footer.checkCode[0] = 0xF4;
	dataOut.footer.checkCode[1] = 0xF3;
	dataOut.footer.checkCode[2] = 0xF2;
	dataOut.footer.checkCode[3] = 0xF1;

	HAL_GPIO_WritePin(SMALLCPU_CSB_GPIO_PORT,SMALLCPU_CSB_PIN,GPIO_PIN_SET);
	delayMicros(10);

	if(data_old__lost_connection_to_slave_counter_temp >= 3)
	{
		data_old__lost_connection_to_slave_counter_temp = 0;
	}
	else
	{
		HAL_GPIO_WritePin(SMALLCPU_CSB_GPIO_PORT,SMALLCPU_CSB_PIN,GPIO_PIN_RESET);
	}

	HAL_SPI_TransmitReceive_DMA(&cpu2DmaSpi, (uint8_t *)&dataOut, (uint8_t *)&dataIn, EXCHANGE_BUFFERSIZE+1);
	return 1;
}


void DataEX_control_connection_while_asking_for_sleep(void)
{
 	if(!DataEX_check_header_and_footer_ok())
	{
		data_old__lost_connection_to_slave_counter_temp += 1;
	}
}

uint8_t DataEX_check_header_and_footer_ok(void)
{
	if(dataIn.header.checkCode[0] != 0xA1)
		return 0;
	if(dataIn.header.checkCode[1] != 0xA2)
		return 0;
	if(dataIn.header.checkCode[2] != 0xA3)
		return 0;
	if(dataIn.header.checkCode[3] != 0xA4)
		return 0;
	if(dataIn.footer.checkCode[0] != 0xE1)
		return 0;
	if(dataIn.footer.checkCode[1] != 0xE2)
		return 0;
	if(dataIn.footer.checkCode[2] != 0xE3)
		return 0;
	if(dataIn.footer.checkCode[3] != 0xE4)
		return 0;

	return 1;
}

