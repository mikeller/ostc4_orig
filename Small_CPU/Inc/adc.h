/**
  ******************************************************************************
  * @file    adc.h
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    11-Dec-2014
  * @brief   ADC ambient light sensor
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ADC_H
#define ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
	 
uint16_t get_ambient_light_level(void);
	 
void adc_ambient_light_sensor_get_data(void);

void ADCx_Init(void);
void ADCx_DeInit(void);


#ifdef __cplusplus
}
#endif
#endif /* ADC_H */



/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
