/**
  ******************************************************************************
  * @file    externalInterface.c
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    07-Nov-2020
  * @brief   Interface functionality to proceed external analog signal via i2c connection
  *
  @verbatim
  ==============================================================================
                ##### stm32f4xx_hal_i2c.c modification #####
  ==============================================================================
	The LTC2942 requires an repeated start condition without stop condition
	for data reception.

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

#include <math.h>
#include "i2c.h"
#include "externalInterface.h"
#include "scheduler.h"

extern SGlobal global;

#define ADC_ANSWER_LENGTH	(5u)		/* 3424 will provide addr + 4 data bytes */
#define ADC_TIMEOUT			(10u)		/* conversion stuck for unknown reason => restart */
#define ADC_REF_VOLTAGE_MV	(2048.0f)	/* reference voltage of MPC3424*/

#define ADC_START_CONVERSION		(0x80)
#define ADC_GAIN_4					(0x02)
#define ADC_GAIN_4_VALUE			(4.0f)
#define ADC_GAIN_8					(0x03)
#define ADC_GAIN_8_VALUE			(8.0f)
#define ADC_RESOLUTION_16BIT		(0x08)
#define ADC_RESOLUTION_16BIT_VALUE	(16u)
#define ADC_RESOLUTION_18BIT		(0x0C)
#define ADC_RESOLUTION_18BIT_VALUE	(18u)

#define ANSWER_CONFBYTE_INDEX		(4u)

static uint8_t activeChannel = 0;			/* channel which is in request */
static uint8_t recBuf[ADC_ANSWER_LENGTH];
static uint8_t timeoutCnt = 0;
static uint8_t externalInterfacePresent = 0;

float externalChannel_mV[MAX_ADC_CHANNEL];


void externalInterface_Init(void)
{
	activeChannel = 0;
	timeoutCnt = 0;
	externalInterfacePresent = 0;
	if(externalInterface_StartConversion(activeChannel) == HAL_OK)
	{
		externalInterfacePresent = 1;
		global.deviceDataSendToMaster.hw_Info.extADC = 1;
	}
	global.deviceDataSendToMaster.hw_Info.checkADC = 1;
}


uint8_t externalInterface_StartConversion(uint8_t channel)
{
	uint8_t retval = 0;
	uint8_t confByte = 0;

	if(channel < MAX_ADC_CHANNEL)
	{
		confByte = ADC_START_CONVERSION | ADC_RESOLUTION_16BIT | ADC_GAIN_8;
		confByte |= channel << 5;
		retval = I2C_Master_Transmit(DEVICE_EXTERNAL_ADC, &confByte, 1);
	}
	return retval;
}

/* Check if conversion is done and trigger measurement of next channel */
uint8_t externalInterface_ReadAndSwitch()
{
	uint8_t retval = EXTERNAL_ADC_NO_DATA;

	if(externalInterfacePresent)
	{
		if(I2C_Master_Receive(DEVICE_EXTERNAL_ADC, recBuf, ADC_ANSWER_LENGTH) == HAL_OK)
		{
			if((recBuf[ANSWER_CONFBYTE_INDEX] & ADC_START_CONVERSION) == 0)		/* !ready set => received data contains new value */
			{
				retval = activeChannel;										/* return channel number providing new data */
				activeChannel++;
				if(activeChannel == MAX_ADC_CHANNEL)
				{
					activeChannel = 0;
				}
				externalInterface_StartConversion(activeChannel);
				timeoutCnt = 0;
			}
			else
			{
				if(timeoutCnt++ >= ADC_TIMEOUT)
				{
					externalInterface_StartConversion(activeChannel);
					timeoutCnt = 0;
				}
			}
		}
		else		/* take also i2c bus disturb into account */
		{
			if(timeoutCnt++ >= ADC_TIMEOUT)
			{
				externalInterface_StartConversion(activeChannel);
				timeoutCnt = 0;
			}
		}
	}
	return retval;
}
float externalInterface_CalculateADCValue(uint8_t channel)
{
	int32_t rawvalue = 0;
	float retValue = 0.0;
	if(channel < MAX_ADC_CHANNEL)
	{

		rawvalue = ((recBuf[0] << 16) | (recBuf[1] << 8) | (recBuf[2]));

		switch(recBuf[3] & 0x0C)			/* confbyte => Resolution bits*/
		{
			case ADC_RESOLUTION_16BIT:		rawvalue = rawvalue >> 8;										/* only 2 databytes received shift out confbyte*/
											if(rawvalue & (0x1 << (ADC_RESOLUTION_16BIT_VALUE-1)))			/* MSB set => negative number */
											{
												rawvalue |= 0xFFFF0000; 	/* set MSB for int32 */
											}
											else
											{
												rawvalue &= 0x0000FFFF;
											}
											externalChannel_mV[channel] = ADC_REF_VOLTAGE_MV * 2.0 / (float) pow(2,ADC_RESOLUTION_16BIT_VALUE);	/* calculate bit resolution */
				break;
			case ADC_RESOLUTION_18BIT:		if(rawvalue & (0x1 << (ADC_RESOLUTION_18BIT_VALUE-1)))			/* MSB set => negative number */
											{
												rawvalue |= 0xFFFE0000; 	/* set MSB for int32 */
											}
											externalChannel_mV[channel] = ADC_REF_VOLTAGE_MV * 2.0 / (float) pow(2,ADC_RESOLUTION_18BIT_VALUE);	/* calculate bit resolution */
							break;
			default: rawvalue = 0;
				break;
		}
		externalChannel_mV[channel] = externalChannel_mV[channel] * rawvalue / ADC_GAIN_8_VALUE;
		retValue = externalChannel_mV[channel];
	}
	return retValue;
}
float getExternalInterfaceChannel(uint8_t channel)
{
	float retval = 0;

	if(channel < MAX_ADC_CHANNEL)
	{
		retval = externalChannel_mV[channel];
	}
	return retval;
}
