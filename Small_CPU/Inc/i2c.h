/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef I2C_H
#define I2C_H

/* Pressure Sensor */
#define DEVICE_PRESSURE_MS5803     		0xEE 	// gen 1 and gen 2 use 0xEE (MS5803)
#define	DEVICE_PRESSURE_MS5837   		0xEC	// end-2019 hardware (gen 3) uses 0xEC (MS5837)

/* Compass/Accelerometer */
#define COMPASS_NOT_RECOGNIZED  		0xAA	///< id used with hardwareCompass

typedef enum
{
	compass_generation_undef =			0x00,
	compass_generation1,						// Hardware gen 1 (Two chip solution with MMA8452Q and HMC5883L)
	compass_generation2,						// Hardware gen 2 (Single chip solution LSM303D)
	compass_generation3,						// Hardware gen 3 (Single chip solution LSM303AGR)
	compass_generation_future
} compass_generation_t;

#define DEVICE_ACCELARATOR_MMA8452Q 	0x38	// Hardware gen 1 (Two chip solution with MMA8452Q and HMC5883L)
#define DEVICE_COMPASS_HMC5883L			0x3C	// Hardware gen 1 (Two chip solution with MMA8452Q and HMC5883L)

#define DEVICE_COMPASS_303D				0x3C 	// Hardware gen 2 (Single chip solution LSM303D)

#define	DEVICE_COMPASS_303AGR			0x3C	// Hardware gen 3 (Single chip solution LSM303AGR)
#define	DEVICE_ACCELARATOR_303AGR		0x32	// Hardware gen 3 (Single chip solution LSM303AGR)

// Compass 3 defines (Can move in separate .h file...)
#define WHOIAM_VALUE_LSM303AGR			0x33

/* Battery Gas Gauge */
#define DEVICE_BATTERYGAUGE 			0xC8 	// LTC2941 battery gauge

/* ADC for external bulkhead */
#define DEVICE_EXTERNAL_ADC				0xD0	// MCP3424 with Adr0=GND and Adr1=GND (Hardware gen 3 only)


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

HAL_StatusTypeDef I2C_Master_Transmit(  uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef I2C_Master_TransmitNoStop(  uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef I2C_Master_Receive(  uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef MX_I2C1_Init(void);
void I2C_DeInit(void);
HAL_StatusTypeDef I2C1_Status(void);

GPIO_PinState MX_I2C1_TestAndClear(void);

//void I2C_Error(void);


#endif /* I2C_H */
