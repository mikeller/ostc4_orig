/**
  ******************************************************************************
  * @file    batteryGasGauge.c 
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    09-Dec-2014
  * @brief   LTC2942 Battery Gas Gauge
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
#include <string.h>	/* memset */
#include "batteryGasGauge.h"
#include "baseCPU2.h"
#include "stm32f4xx_hal.h"
#include "i2c.h"

static float battery_f_voltage = 0;
static float battery_f_charge_percent = 0;

#define BGG_BATTERY_OFFSET          (26123)  //; 65536-(3,35Ah/0,085mAh)
#define BGG_BATTERY_DIVIDER         (394)    //; 3,35Ah/0,085mAh/100 [%]

float get_voltage(void)
{
#ifdef OSTC_ON_DISCOVERY_HARDWARE
	return 3.0f;
#endif

	return battery_f_voltage;
}


float get_charge(void)
{
	#ifdef OSTC_ON_DISCOVERY_HARDWARE
		return 100.0f;
	#endif
	
	return battery_f_charge_percent;
}


void init_battery_gas_gauge(void)
{
	#ifdef OSTC_ON_DISCOVERY_HARDWARE
		return;
	#endif
	
	uint8_t buffer[2];
	buffer[0] = 0x01;

	// F8 = 11111000:
	// Vbat 3.0V (11)
	// Prescale M = 128 (111)
	// AL/CC pin disable (0)
	// Shutdown (0)
	buffer[1] = 0xF8;
	I2C_Master_Transmit(DEVICE_BATTERYGAUGE, buffer, 2);
}

uint8_t battery_gas_gauge_CheckConfigOK(void)
{
	#ifdef OSTC_ON_DISCOVERY_HARDWARE
		return;
	#endif

	uint8_t retval = 0;
	uint8_t bufferReceive[10];

	memset(bufferReceive,0,sizeof(bufferReceive));

	I2C_Master_Receive(DEVICE_BATTERYGAUGE, bufferReceive, 10);
	if(bufferReceive[1] == 0xf8)
	{
		retval = 1;
	}
	return retval;
}

static void disable_adc(void)
{
	uint8_t buffer[2];
	buffer[0] = 0x01;

	// according to the datasheet of the LTC2942, the adc shall
	// be disabled when writing to the gauge registers

	// 0xF9 = 11111001:
	// see init_battery_gas_gauge()
	// Shutdown (1)
	buffer[1] = 0xF9;
	I2C_Master_Transmit(DEVICE_BATTERYGAUGE, buffer, 2);
}


void battery_gas_gauge_get_data(void)
{
	#ifdef OSTC_ON_DISCOVERY_HARDWARE
		return;
	#endif
	
	float battery_f_voltage_local;
	float battery_f_charge_percent_local;
	
	uint8_t bufferReceive[10];
	I2C_Master_Receive(		DEVICE_BATTERYGAUGE, bufferReceive, 10);

	battery_f_voltage_local =  (float)(bufferReceive[8] * 256);
	battery_f_voltage_local += (float)(bufferReceive[9]);
	battery_f_voltage_local *= (float)6 / (float)0xFFFF;

	// max/full: 0.085 mAh * 1 * 65535 = 5570 mAh
	battery_f_charge_percent_local =  (float)(bufferReceive[2] * 256);
	battery_f_charge_percent_local += (float)(bufferReceive[3]);
	battery_f_charge_percent_local -= BGG_BATTERY_OFFSET;
	battery_f_charge_percent_local /= BGG_BATTERY_DIVIDER;
	
	if(battery_f_charge_percent_local < 0)
		battery_f_charge_percent_local = 0;
	
	battery_f_voltage = battery_f_voltage_local;
	battery_f_charge_percent = battery_f_charge_percent_local;
}


void battery_gas_gauge_set_charge_full(void)
{
	disable_adc();
	#ifdef OSTC_ON_DISCOVERY_HARDWARE
		return;
	#endif
	
	uint8_t bufferSend[3];
	bufferSend[0] = 0x02;
	bufferSend[1] = 0xFF;
	bufferSend[2] = 0xFF;
	I2C_Master_Transmit(  DEVICE_BATTERYGAUGE, bufferSend, 3);
	init_battery_gas_gauge();
}


void battery_gas_gauge_set(float percentage)
{

	disable_adc();
	#ifdef OSTC_ON_DISCOVERY_HARDWARE
		return;
	#endif

	uint16_t mAhSend;
	
	if(percentage >= 100)
		mAhSend = 0xFFFF;
	else {
		mAhSend = (percentage * BGG_BATTERY_DIVIDER) + BGG_BATTERY_OFFSET;
	}
	
	uint8_t bufferSend[3];
	bufferSend[0] = 0x02;
	bufferSend[1] = (uint8_t)(mAhSend / 256);
	bufferSend[2] = (uint8_t)(mAhSend & 0xFF);
	I2C_Master_Transmit(  DEVICE_BATTERYGAUGE, bufferSend, 3);
	init_battery_gas_gauge();
}


/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
