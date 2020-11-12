/**
  ******************************************************************************
  * @file    externalInterface.h
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    07-Nov-2020
  * @brief	 Interface functionality to proceed external analog signal via i2c connection
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef EXTERNAL_INTERFACE_H
#define EXTERNAL_INTERFACE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#define MAX_ADC_CHANNEL		(3u)		/* number of channels to be read */
#define EXTERNAL_ADC_NO_DATA	0xFF

void externalInterface_Init(void);
uint8_t externalInterface_StartConversion(uint8_t channel);
uint8_t externalInterface_ReadAndSwitch();
float externalInterface_CalculateADCValue(uint8_t channel);
float getExternalInterfaceChannel(uint8_t channel);

#endif /* EXTERNAL_INTERFACE_H */
