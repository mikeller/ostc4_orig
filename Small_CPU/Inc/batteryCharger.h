/**
  ******************************************************************************
  * @file    batteryCharger.h
  * @author  heinrichs weikamp gmbh
  * @date    09-Dec-2014
  * @version V0.0.1
  * @since   09-Dec-2014
  * @brief	 LTC4054 Standalone Linear Li-Ion Battery Charger 
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
#ifndef BATTERY_CHARGER_H
#define BATTERY_CHARGER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

uint8_t get_charge_status(void);

void init_battery_charger_status(void);
void ReInit_battery_charger_status_pins(void);
void DeInit_battery_charger_status_pins(void);
void battery_charger_get_status_and_contral_battery_gas_gauge(uint8_t cycleTimeBase);

#ifdef __cplusplus
}
#endif

#endif /* BATTERY_CHARGER_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
