/**
  ******************************************************************************
  * @file    scheduler.h
  * @author  heinrichs weikamp gmbh
  * @version V0.0.5
  * @date    27-March-2014
  * @brief
  *           
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 


#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifdef __cplusplus
 extern "C" {
#endif

	 
/* Includes ------------------------------------------------------------------*/
#include "data_central.h"
#include "data_exchange.h"
#include "settings.h"

/* Types -------------------------------------------------------------*/
#define MAX_WIRELESS_BYTES 10
#define SENSOR_PRESSURE_ID 0
#define MAX_SENSORS 1
	 
typedef struct
{
	uint8_t mode;
	short conservatism;
	short repetitive_dive; 
	long seconds_since_last_dive;
	long no_fly_time_minutes;
	uint8_t whichGas;
	SGas aktualGas[2];
	float ceiling_from_main_CPU_mbar;
	SLifeData lifeData;
	SVpm vpm;
	SSettings settings;
	SDevice deviceData;
	SDataExchangeSlaveToMasterDeviceData deviceDataSendToMaster; 
	SDataExchangeSlaveToMaster dataSendToMaster; 
	SDataReceiveFromMaster 			dataSendToSlave; 
	_Bool demo_mode;
	uint8_t dataSendToSlaveIsValid;
	uint8_t dataSendToSlavePending;
	uint8_t dataUpdateIsNeeded;
	uint32_t sync_error_count;
	uint32_t check_sync_not_running;
	uint8_t ButtonResponsiveness[4];
	uint8_t chargerStatus;
	uint8_t	dataSendToSlaveIsNotValidCount;
	uint8_t wirelessdata[MAX_WIRELESS_BYTES];
	uint8_t wirelessReceived;
	uint8_t wirelessConfidenceStatus;
	uint8_t ButtonPICdata[4];
	uint8_t accidentFlag;
	uint32_t accidentRemainingSeconds;
	uint8_t sensorError[MAX_SENSORS];
	uint8_t I2C_SystemStatus;
} SGlobal;

typedef struct
{
	long seconds_since_last_dive;
	long no_fly_time_minutes;
} SBackup;

/* Variables ---------------------------------------------------------*/
extern SGlobal global;


/* Function prototypes -----------------------------------------------*/

void initGlobals(void);

void scheduleSurfaceMode(void);
void scheduleDiveMode(void);
void scheduleSleepMode(void);
void scheduleCompassCalibrationMode(void);
void scheduleTestMode(void);

void scheduleUpdateLifeData(int32_t asynchron_milliseconds_since_last);
void scheduleSpecial_Evaluate_DataSendToSlave(void);
void scheduleUpdateDeviceDataChargerFull(void);
void scheduleUpdateDeviceDataChargerCharging(void);

uint8_t scheduleSetButtonResponsiveness(void);

void copyBatteryData(void);

//void scheduleSurfaceMode_test(void);
//void scheduleSleepMode_test(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDULER_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
