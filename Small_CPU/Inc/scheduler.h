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
#define SENSOR_PRESSURE_ID 0
#define MAX_SENSORS 1
 
#define SPI_SYNC_METHOD_NONE	(0u)
#define SPI_SYNC_METHOD_HARD	(1u)	/* Scheduler shall reset all counters to 0 */
#define SPI_SYNC_METHOD_SOFT	(2u)	/* Scheduler shall reset adjust counters to 100ms SPI data exchange cycle */
#define SPI_SYNC_METHOD_INVALID	(4u)

#define SCHEDULER_TICK_EXE1SEC	(980u) 	/* tick count based on cycle start which is used to trigger functions which */
										/* shall be executed once in a second (20ms before cycle restarts) */

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
	uint32_t sync_error_count;
	uint32_t check_sync_not_running;
	uint8_t ButtonResponsiveness[4];
	uint8_t chargerStatus;
	uint8_t	dataSendToSlaveIsNotValidCount;
	uint8_t ButtonPICdata[4];
	uint8_t accidentFlag;
	uint32_t accidentRemainingSeconds;
	uint8_t sensorError[MAX_SENSORS];
	HAL_StatusTypeDef I2C_SystemStatus;
} SGlobal;

typedef struct
{
	long seconds_since_last_dive;
	long no_fly_time_minutes;
} SBackup;

typedef struct
{
	uint8_t counterSPIdata100msec;
	uint8_t counterPressure100msec;
	uint8_t counterCompass100msec;
	uint8_t counterAmbientLight100msec;
	uint32_t tick_execute1second;
	uint32_t tickstart;
} SScheduleCtrl;


/* Variables ---------------------------------------------------------*/
extern SGlobal global;


/* Function prototypes -----------------------------------------------*/

void initGlobals(void);
void reinitGlobals(void);

void scheduleSurfaceMode(void);
void scheduleDiveMode(void);
void scheduleSleepMode(void);
void scheduleCompassCalibrationMode(void);
void scheduleTestMode(void);

void scheduleUpdateLifeData(int32_t asynchron_milliseconds_since_last);
void scheduleSpecial_Evaluate_DataSendToSlave(void);
void scheduleUpdateDeviceDataChargerFull(void);
void scheduleUpdateDeviceDataChargerCharging(void);

void Scheduler_Request_sync_with_SPI(uint8_t SyncMethod);
void Scheduler_SyncToSPI(uint8_t TXtick);

uint8_t scheduleSetButtonResponsiveness(void);

void copyBatteryData(void);

//void scheduleSurfaceMode_test(void);
//void scheduleSleepMode_test(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDULER_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
