#include "baseCPU2.h"
#include "i2c.h"
#include "scheduler.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/


//  ===============================================================================
//	I2C addresses - see i2c.h
///
///		#define DEVICE_PRESSURE        			0xEE (0x77)	Write 0xEE	Read 0xEF
///		#define DEVICE_COMPASS_HMC5883L			0x3C (0x1E) Write 0x3C	Read 0x3D
///		#define DEVICE_ACCELARATOR_MMA8452Q 0x38 (0x1C) Write 0x38	Read 0x39
///		#define DEVICE_BATTERYGAUGE 				0xC8 (0x64)	Write 0xC8

///		#define DEVICE_COMPASS_303D					0x3C // 0x1E // x0011110 // SA0 to GND
///		
/// 0x6E 0x48 -> 0x6F 3d d1 86 3f    0x66 58
///
//  ===============================================================================
I2C_HandleTypeDef I2cHandle;


/*
static void I2C_Error_Handler(void)
{
  while(1)
  {
  }
}
*/

HAL_StatusTypeDef I2C1_Status(void)
{
return (HAL_StatusTypeDef)global.I2C_SystemStatus;
}


GPIO_PinState HAL_I2C_Read_Data_PIN(void)
{
	return HAL_GPIO_ReadPin(I2Cx_SDA_GPIO_PORT,I2Cx_SDA_PIN);
}

void HAL_I2C_Send_One_CLOCK(void)
{
	HAL_GPIO_WritePin(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN, GPIO_PIN_SET);
	HAL_Delay(10);
}

GPIO_PinState MX_I2C1_TestAndClear(void)
{
	I2C_DeInit();
	HAL_I2C_ManualControl_MspInit();
	for(int i=0; i<9;i++)
	{
		if(HAL_I2C_Read_Data_PIN() == GPIO_PIN_RESET)
			HAL_I2C_Send_One_CLOCK();
		else
			break;
	}
	return HAL_I2C_Read_Data_PIN();
}

HAL_StatusTypeDef MX_I2C1_Init(void)
{
	I2cHandle.Instance             = I2Cx;
  I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  I2cHandle.Init.ClockSpeed      = 400000;//400000;
  I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
  I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
  I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
  I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLED;
  I2cHandle.Init.OwnAddress1     = 0x01;
  

	global.I2C_SystemStatus = HAL_I2C_Init(&I2cHandle);
	HAL_I2CEx_AnalogFilter_Config(&I2cHandle, I2C_ANALOGFILTER_ENABLED);


//	if(global.dataSendToSlavePending)
//	{
//		scheduleSpecial_Evaluate_DataSendToSlave();
//	}
	global.dataUpdateIsNeeded = 1;
	return (HAL_StatusTypeDef)global.I2C_SystemStatus;
}


void I2C_DeInit(void)
{
	HAL_I2C_DeInit(&I2cHandle);
}


uint8_t i2c_errors = 0;

void I2C_Error_count(void)
{
	i2c_errors++;
}


HAL_StatusTypeDef I2C_Master_TransmitNoStop(  uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{
	if(global.I2C_SystemStatus != HAL_OK)
		return (HAL_StatusTypeDef)global.I2C_SystemStatus;
	
	

  global.I2C_SystemStatus = HAL_I2C_Master_Transmit(&I2cHandle, DevAddress,  pData, Size, 100 /*FIXME , 0*/);
	if(global.I2C_SystemStatus != HAL_OK)
	{
		I2C_Error_count();
	}

//	if(global.dataSendToSlavePending)
//	{
//		scheduleSpecial_Evaluate_DataSendToSlave();
//	}
	global.dataUpdateIsNeeded = 1;
	return (HAL_StatusTypeDef)global.I2C_SystemStatus;
}


HAL_StatusTypeDef I2C_Master_Transmit(  uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{
	if(global.I2C_SystemStatus != HAL_OK)
		return (HAL_StatusTypeDef)(global.I2C_SystemStatus & 0x03);

	


	global.I2C_SystemStatus = HAL_I2C_Master_Transmit(&I2cHandle, DevAddress,  pData, Size, 100 /*FIXME , 1*/);
	if(global.I2C_SystemStatus != HAL_OK)
	{
		I2C_Error_count();
	}

//	if(global.dataSendToSlavePending)
//	{
//		scheduleSpecial_Evaluate_DataSendToSlave();
//	}
//
	global.dataUpdateIsNeeded = 1;
	return (HAL_StatusTypeDef)global.I2C_SystemStatus;
}


HAL_StatusTypeDef I2C_Master_Receive(  uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{
	if(global.I2C_SystemStatus != HAL_OK)
		return (HAL_StatusTypeDef)global.I2C_SystemStatus;

	uint8_t localHALstatusReturn = 0xFF;


  localHALstatusReturn = HAL_I2C_Master_Receive(&I2cHandle, DevAddress,  pData, Size, 100);
	if(localHALstatusReturn != HAL_OK)
	{
		I2C_Error_count();
	}

	global.dataUpdateIsNeeded = 1;
//	if(global.dataSendToSlavePending)
//	{
//		scheduleSpecial_Evaluate_DataSendToSlave();
//	}
	return (HAL_StatusTypeDef)localHALstatusReturn;
}

