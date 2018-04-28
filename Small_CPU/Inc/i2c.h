/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef I2C_H
#define I2C_H

/* Drucksensor */
#define DEVICE_PRESSURE        0xEE

/* Kompass */
#define DEVICE_ACCELARATOR_MMA8452Q 0x38 // 0x1C  // chip 3
#define DEVICE_COMPASS_HMC5883L			0x3C  //0x1E  // chip 4

//#define DEVICE_ACCELARATOR_303D 		0x1E // x0011110 // SA0 to GND
#define DEVICE_COMPASS_303D						0x3C // 0x1E // x0011110_ // SA0 to GND
#define DEVICE_ACCELARATOR_303DLHC 		0x32 // 0x19 // x0011001_ // SA0 to GND

/* Battery Gas Gauge */
#define DEVICE_BATTERYGAUGE 0xC8 // 0x64


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
