/**
  ******************************************************************************
  * @file    batteryGasGauge.h
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    09-Dec-2014
  * @brief	 LTC2942
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
#ifndef BATTERY_GAS_GAUGE_H
#define BATTERY_GAS_GAUGE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

void init_battery_gas_gauge(void);

float get_voltage(void);
float get_charge(void);

void battery_gas_gauge_get_data(void);
void battery_gas_gauge_set_charge_full(void);
void battery_gas_gauge_set(float percentage);

#ifdef __cplusplus
}
#endif

#endif /* BATTERY_GAS_GAUGE_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
