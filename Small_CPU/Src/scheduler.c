/**
  ******************************************************************************
  * @file    scheduler.c 
  * @author  heinrichs weikamp gmbh
  * @date    27-March-2014
  * @version V0.0.6
  * @since   18-June-2015
  * @brief   the main part except for base.c
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
	
	
//#define DEBUGMODE

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "baseCPU2.h"
#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "scheduler.h"
#include "pressure.h"
#include "compass.h"
#include "batteryGasGauge.h"
#include "batteryCharger.h"
#include "spi.h"
#include "rtc.h"
#include "dma.h"
#include "adc.h"
#include "calc_crush.h"
#include "stm32f4xx_hal_rtc_ex.h"
#include "decom.h"
#include "wireless.h"
#include "tm_stm32f4_otp.h"


#define INVALID_PREASURE_VALUE (100.0F)

/* Private types -------------------------------------------------------------*/
const SGas Air = {79,0,0,0,0};

uint8_t testarrayindex = 0; 
uint32_t testarray[256];
uint32_t testarrayMain[256];

/* Exported variables --------------------------------------------------------*/
SGlobal global;
SDevice DeviceDataFlash;
uint8_t deviceDataFlashValid = 0;
uint8_t deviceDataSubSeconds = 0;

/* Private variables ---------------------------------------------------------*/
/* can be lost while in sleep */
uint8_t clearDecoNow = 0;
uint8_t setButtonsNow = 0;

/* has to be in SRAM2 */
uint8_t secondsCount = 0;
 
/* Private function prototypes -----------------------------------------------*/

_Bool vpm_crush2(void);
void scheduleUpdateDeviceData(void);
void initStructWithZeero(uint8_t* data, uint16_t length);
long get_nofly_time_minutes(void);
void copyActualGas(SGas gas);
void copyPressureData(void);
void copyCnsAndOtuData(void);
void copyTimeData(void);
void copyCompassData(void);
void copyCompassDataDuringCalibration(int16_t dx, int16_t dy, int16_t dz);
//void copyBatteryData(void); now in header
void copyAmbientLightData(void);
void copyTissueData(void);
void copyVpmCrushingData(void);
void copyDeviceData(void);
void changeAgeWirelessData(void);
void copyWirelessData(void);
void copyPICdata(void);
uint16_t schedule_update_timer_helper(int8_t thisSeconds);


uint32_t time_elapsed_ms(uint32_t ticksstart,uint32_t ticksnow);

_Bool scheduleCheck_pressure_reached_dive_mode_level(void);
void scheduleSetDate(SDeviceLine *line);

/* Exported functions --------------------------------------------------------*/

void initGlobals(void)
{
	initStructWithZeero((uint8_t*) &global, sizeof(SGlobal));
	
	global.dataSendToSlavePending = 0;
	global.dataSendToSlaveIsValid = 1;
	global.dataSendToSlaveIsNotValidCount = 0;

	global.mode = MODE_POWERUP;
	global.repetitive_dive = 0;
	global.conservatism = 0;
	global.whichGas = 0;
	global.aktualGas[0] = Air;
	global.lifeData.actualGas = global.aktualGas[0];

	const uint8_t button_standard_sensitivity = ((2400-(  40  *24))/10)+15;
	global.ButtonResponsiveness[0] = button_standard_sensitivity;
	global.ButtonResponsiveness[1] = button_standard_sensitivity;
	global.ButtonResponsiveness[2] = button_standard_sensitivity;
	global.ButtonResponsiveness[3] = button_standard_sensitivity;
	
	global.ButtonPICdata[0] = 0xFF;
	global.ButtonPICdata[1] = 0xFF;
	global.ButtonPICdata[2] = 0xFF;
	global.ButtonPICdata[3] = 0xFF;

	global.I2C_SystemStatus = 0xFF; // 0x00 would be everything working
	
	global.lifeData.pressure_ambient_bar = INVALID_PREASURE_VALUE;
	global.lifeData.pressure_surface_bar = INVALID_PREASURE_VALUE;
	decom_reset_with_1000mbar(&global.lifeData);
	
	global.demo_mode = 0;

	for(int i = 0; i < MAX_SENSORS; i++)
	{
		global.sensorError[i] = HAL_OK; // HAL_OK = 0;
	}

	global.dataSendToMaster.RTE_VERSION_high = firmwareVersionHigh();//RTE_VERSION_HIGH;;
	global.dataSendToMaster.RTE_VERSION_low = firmwareVersionLow();//RTE_VERSION_LOW;;
	global.dataSendToMaster.chargeStatus = 0;
	
	global.dataSendToMaster.power_on_reset = 1;
	global.dataSendToMaster.header.checkCode[0] = 0xA1;
	global.dataSendToMaster.header.checkCode[1] = 0xA2;
	global.dataSendToMaster.header.checkCode[2] = 0xA3;
	global.dataSendToMaster.header.checkCode[3] = 0xA4;
	global.dataSendToMaster.footer.checkCode[3] = 0xE4;
	global.dataSendToMaster.footer.checkCode[2] = 0xE3;
	global.dataSendToMaster.footer.checkCode[1] = 0xE2;
	global.dataSendToMaster.footer.checkCode[0] = 0xE1;
	global.dataSendToMaster.sensorErrors = 0;

	global.sync_error_count = 0;
	global.check_sync_not_running = 0;

	global.deviceDataSendToMaster.RTE_VERSION_high = firmwareVersionHigh();//RTE_VERSION_HIGH;
	global.deviceDataSendToMaster.RTE_VERSION_low = firmwareVersionLow();//RTE_VERSION_LOW;
	global.deviceDataSendToMaster.chargeStatus = 0;

	global.deviceDataSendToMaster.power_on_reset = 1;
	global.deviceDataSendToMaster.header.checkCode[0] = 0xDF;
	global.deviceDataSendToMaster.header.checkCode[1] = 0xDE;
	global.deviceDataSendToMaster.header.checkCode[2] = 0xDD;
	global.deviceDataSendToMaster.header.checkCode[3] = 0xDC;
	global.deviceDataSendToMaster.footer.checkCode[3] = 0xE4;
	global.deviceDataSendToMaster.footer.checkCode[2] = 0xE3;
	global.deviceDataSendToMaster.footer.checkCode[1] = 0xE2;
	global.deviceDataSendToMaster.footer.checkCode[0] = 0xE1;
	
	global.dataSendToSlave.getDeviceDataNow = 0;
	
	global.deviceData.batteryChargeCompleteCycles.value_int32 = 0;
	global.deviceData.batteryChargeCycles.value_int32 = 0;
	global.deviceData.depthMaximum.value_int32 = 0;
	global.deviceData.diveCycles.value_int32 = 0;
	global.deviceData.hoursOfOperation.value_int32 = 0;
	global.deviceData.temperatureMaximum.value_int32 = INT32_MIN;
	global.deviceData.temperatureMinimum.value_int32 = INT32_MAX;
	global.deviceData.voltageMinimum.value_int32 = INT32_MAX;
}


void scheduleSpecial_Evaluate_DataSendToSlave(void)
{
	//TEMPORARY fix for compass calibration.
	//TODO: Fix I2C timeout for complete solving problem.
	if(global.mode==MODE_CALIB){
		return;
	}

	global.dataSendToSlavePending = 0;
	if(!global.dataSendToSlaveIsValid) return;
	
	global.dataSendToMaster.confirmRequest.uw = 0;
	
	if(TM_OTP_Read(0,0) == 0xFF)
	{
		if(global.dataSendToSlave.revisionHardware == (global.dataSendToSlave.revisionCRCx0x7A ^ 0x7A))
			TM_OTP_Write(0,0,global.dataSendToSlave.revisionHardware);
	}
	
	if(global.dataSendToSlave.setAccidentFlag)
	{
		global.dataSendToMaster.confirmRequest.ub.accident = 1;
		global.deviceData.diveAccident.value_int32 = global.dataSendToSlave.setAccidentFlag;
		scheduleSetDate(&global.deviceData.diveAccident);
		global.accidentFlag |= global.dataSendToSlave.setAccidentFlag;
		if(global.accidentFlag == ACCIDENT_CNS) // LVL1
			global.accidentRemainingSeconds = 2*60*60;
		else
			global.accidentRemainingSeconds = 24*60*60;
	}
	
	if(global.dataSendToSlave.setTimeNow)
	{
		global.dataSendToMaster.confirmRequest.ub.time = 1;
		RTC_SetTime(global.dataSendToSlave.data.newTime);
		schedule_update_timer_helper(0);
	}
	
	if(global.dataSendToSlave.setDateNow)
	{
		global.dataSendToMaster.confirmRequest.ub.date = 1;
		RTC_SetDate(global.dataSendToSlave.data.newDate);
		schedule_update_timer_helper(0);
	}		

	if(global.dataSendToSlave.calibrateCompassNow)
	{
		global.dataSendToMaster.confirmRequest.ub.compass = 1;
		global.mode = MODE_CALIB;
	}		

	if(global.dataSendToSlave.clearDecoNow)
	{
		global.dataSendToMaster.confirmRequest.ub.clearDeco = 1;
		clearDecoNow = 1;
	}		

	if(global.dataSendToSlave.setButtonSensitivityNow)
	{
		global.dataSendToMaster.confirmRequest.ub.button = 1;
		global.ButtonResponsiveness[0] = global.dataSendToSlave.data.buttonResponsiveness[0];
		global.ButtonResponsiveness[1] = global.dataSendToSlave.data.buttonResponsiveness[1];
		global.ButtonResponsiveness[2] = global.dataSendToSlave.data.buttonResponsiveness[2];
		global.ButtonResponsiveness[3] = global.dataSendToSlave.data.buttonResponsiveness[3];
		setButtonsNow = 1;
	}		

	if(global.dataSendToSlave.setBatteryGaugeNow)
	{
		if(global.mode!=MODE_CALIB){
		global.dataSendToMaster.confirmRequest.ub.batterygauge = 1;
		battery_gas_gauge_set(global.dataSendToSlave.data.newBatteryGaugePercentageFloat);
		}
	}		

	if((global.mode == MODE_SURFACE) && (global.dataSendToSlave.mode == MODE_SHUTDOWN))
	{
		global.mode = MODE_SHUTDOWN;
	}

	if(global.mode == MODE_DIVE)
	{
		copyActualGas(global.dataSendToSlave.data.actualGas);
	}
	else
	{
		copyActualGas(Air);
		global.settings.divetimeToCreateLogbook = global.dataSendToSlave.data.divetimeToCreateLogbook;
		global.settings.timeoutDiveReachedZeroDepth = global.dataSendToSlave.data.timeoutDiveReachedZeroDepth;
	}
	
	/* for simulation / testing */
	global.ceiling_from_main_CPU_mbar	= global.dataSendToSlave.data.ambient_pressure_mbar_ceiling;
	
	/* for device data updates */
	deviceDataFlashValid = 0;
	memcpy(&DeviceDataFlash, &global.dataSendToSlave.data.DeviceData, sizeof(SDevice));
	deviceDataFlashValid = 1;


	//TODO: Temporary placed here. Duration ~210 ms.
	if (global.I2C_SystemStatus != HAL_OK) {
		MX_I2C1_TestAndClear();
		MX_I2C1_Init();
//		init_pressure();
//		compass_init(0, 7);
//		accelerator_init();
	}
}


/**
  ******************************************************************************
* @brief   schedule_time_compare_helper. 
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    20-Oct-2016
  ******************************************************************************
  */

uint8_t RtcBugFixChsw(uint8_t inStupidTime)
{
		uint8_t multiplesOf16 = 0;
	
	multiplesOf16 = inStupidTime / 16;

	inStupidTime -= multiplesOf16 * 16;
	
	return (10 * multiplesOf16) + inStupidTime;
}


uint32_t minCounterDebug = 0;

uint32_t schedule_time_compare_helper(RTC_TimeTypeDef timeNow, RTC_DateTypeDef dateNow, RTC_TimeTypeDef timeLast, RTC_DateTypeDef dateLast)
{
	uint32_t nowInSeconds;
	uint32_t lastInSeconds;
	uint32_t resultDiff;
	
	if(timeNow.Minutes != timeLast.Minutes)
		minCounterDebug++;

	nowInSeconds  = (uint32_t)RtcBugFixChsw(timeNow.Hours) * 3600;
	nowInSeconds += (uint32_t)RtcBugFixChsw(timeNow.Minutes) * 60;
	nowInSeconds += (uint32_t)RtcBugFixChsw(timeNow.Seconds);

	lastInSeconds  = (uint32_t)RtcBugFixChsw(timeLast.Hours) * 3600;
	lastInSeconds += (uint32_t)RtcBugFixChsw(timeLast.Minutes) * 60;
	lastInSeconds += (uint32_t)RtcBugFixChsw(timeLast.Seconds);

/*
		nowInSeconds  = (uint32_t)timeNow.Hours * 3600;
	nowInSeconds += (uint32_t)timeNow.Minutes * 60;
	nowInSeconds += (uint32_t)timeNow.Seconds;

	lastInSeconds  = (uint32_t)timeLast.Hours * 3600;
	lastInSeconds += (uint32_t)timeLast.Minutes * 60;
	lastInSeconds += (uint32_t)timeLast.Seconds;
*/
	
	if(dateNow.Date != dateLast.Date)
	{
		resultDiff = 86400 + nowInSeconds - lastInSeconds;
	}
	else
	{
		resultDiff = nowInSeconds - lastInSeconds;
	}
	return resultDiff;
}



/**
  ******************************************************************************
* @brief   schedule_update_timer_helper. 
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    20-Oct-2016
	* @brief	use 0 for init
						use -1 for RTC controlled
						use >= 1 for manual control
  ******************************************************************************
  */
extern RTC_HandleTypeDef RTCHandle;

uint16_t schedule_update_timer_helper(int8_t thisSeconds)
{
	static RTC_TimeTypeDef sTimeLast;
	static RTC_DateTypeDef sDateLast;
	RTC_TimeTypeDef sTimeNow;
	RTC_DateTypeDef sDateNow;
	uint32_t secondsPast;
	uint32_t tempNewValue = 0;

	HAL_RTC_GetTime(&RTCHandle, &sTimeNow, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&RTCHandle, &sDateNow, RTC_FORMAT_BCD);

	if(thisSeconds != 0) // otherwise just store sTimeLast, sDateLast and return 0
	{
		secondsPast = schedule_time_compare_helper(sTimeNow, sDateNow, sTimeLast, sDateLast);

		if(thisSeconds > 0) // use this value instead, good for pre-loading sTimeLast and sDateLast
		{
			secondsPast	= thisSeconds;
		}

		if(global.seconds_since_last_dive)
		{
			if(secondsPast >= 777900)
			{
				global.seconds_since_last_dive = 0;
			}
			else
			{
				tempNewValue = ((uint32_t)global.seconds_since_last_dive) + secondsPast;
				if(tempNewValue > 777900) // a bit more than nine days [seconds]
					global.seconds_since_last_dive = 0;
				else
					global.seconds_since_last_dive = (long)tempNewValue;
			}
		}
	}
		
	sTimeLast = sTimeNow;
	sDateLast = sDateNow;
	
	return tempNewValue;
}




/**
  ******************************************************************************
* @brief   schedule_check_resync. 
  * @author  heinrichs weikamp gmbh
  * @version V0.0.2
  * @date    18-June-2015
  ******************************************************************************
  */

void schedule_check_resync(void)
{
	//TODO: REMOVE
	if((global.check_sync_not_running >= 3))
	{
//		global.dataSendToSlaveIsNotValidCount = 0;
		global.check_sync_not_running = 0;
		global.sync_error_count++;

		/* Try to start communication again. If exchange is stuck during execution for some reason the TX will be aborted by the
		 * function error handler
		 */
		SPI_Start_single_TxRx_with_Master();
	}
}


/**
  ******************************************************************************
* @brief   scheduleDiveMode. /  Dive Mode: Main Loop
  * @author  Peter Ryser
  * @version V0.0.1
  * @date    22-April-2014
  ******************************************************************************
  */
void scheduleDiveMode(void)
{
	uint32_t tickstart = 0;
	uint32_t ticksdiff = 0; 
	uint32_t lasttick = 0;
	tickstart = HAL_GetTick();
	uint8_t counterPressure100msec = 0;
	uint8_t counterCompass100msec = 0;
	uint8_t counterAmbientLight100msec = 0;
	uint16_t counterWireless1msec = 0;
//	uint8_t counterDemo250msec = 0;
	uint32_t turbo_seconds = 0;
	uint8_t counterAscentRate = 0;
	float lastPressure_bar = 0.0f;
	global.dataSendToMaster.mode = MODE_DIVE;
	global.deviceDataSendToMaster.mode = MODE_DIVE;
	//uint16_t counterSecondsShallowDepth = 0;
	uint8_t counter_exit = 0;
	
	tickstart = HAL_GetTick() - 1000;
	
	global.deviceData.diveCycles.value_int32++;
	scheduleSetDate(&global.deviceData.diveCycles);
	global.lifeData.counterSecondsShallowDepth = 0;

	while(global.mode == MODE_DIVE)
	{
		schedule_check_resync();
		lasttick = HAL_GetTick();
		ticksdiff = time_elapsed_ms(tickstart,lasttick);


		//Evaluate wireless data every ms, if present
		if(ticksdiff > counterWireless1msec)
		{
			counterWireless1msec++;
			changeAgeWirelessData();
			global.wirelessReceived = wireless_evaluate(global.wirelessdata,MAX_WIRELESS_BYTES, &global.wirelessConfidenceStatus);
			if((global.wirelessReceived  > 0) && !wireless_evaluate_crc_error(global.wirelessdata,global.wirelessReceived))
			{
				copyWirelessData();
			}
		}

		//Evaluate pressure at 20 ms, 120 ms, 220 ms,....
		if(ticksdiff >= counterPressure100msec * 100 + 20)
		{
				global.check_sync_not_running++;
				pressure_update();
				scheduleUpdateDeviceData();
				if(global.demo_mode)
				{
					turbo_seconds = demo_modify_temperature_and_pressure(global.lifeData.dive_time_seconds, counterPressure100msec, global.ceiling_from_main_CPU_mbar);
					if(turbo_seconds)
					{
						global.lifeData.dive_time_seconds += turbo_seconds;
						decom_tissues_exposure((int)(turbo_seconds), &global.lifeData);
						copyTissueData();
					}
					if((global.lifeData.counterSecondsShallowDepth > 1) && (global.lifeData.counterSecondsShallowDepth < (global.settings.timeoutDiveReachedZeroDepth - 10)))
						global.lifeData.counterSecondsShallowDepth = (global.settings.timeoutDiveReachedZeroDepth - 10);
				}
				
				
				//Calc ascentrate every two second (20 * 100 ms)
				counterAscentRate++;
				if(counterAscentRate == 20)
				{
					global.lifeData.pressure_ambient_bar = get_pressure_mbar() / 1000.0f;
					if(lastPressure_bar >= 0)
					{
							//2 seconds * 30 == 1 minute, bar * 10 = meter
							global.lifeData.ascent_rate_meter_per_min = (lastPressure_bar - global.lifeData.pressure_ambient_bar)  * 30 * 10;
					}
					lastPressure_bar = global.lifeData.pressure_ambient_bar;
					counterAscentRate = 0;
					
//					if(global.demo_mode)
//						global.lifeData.ascent_rate_meter_per_min /= 4;
				}
				copyPressureData();
				counterPressure100msec++;
		}
			//evaluate compass data at 50 ms, 150 ms, 250 ms,....
		if(ticksdiff >= counterCompass100msec * 100 + 50)
		{
			compass_read();
			acceleration_read();
			compass_calc();
			copyCompassData();
			counterCompass100msec++;
		}
		
		if(ticksdiff >= counterAmbientLight100msec * 100 + 70)
		{
			adc_ambient_light_sensor_get_data();
			copyAmbientLightData();
			counterAmbientLight100msec++;
		}

/*		if(global.demo_mode && (ticksdiff >= counterDemo250msec * 250 + 10))
		{
			global.lifeData.dive_time_seconds++;
			copyTimeData();
			counterDemo250msec++;
		}
*/
		//Evaluate tissues, toxic data, vpm, etc. once a second 
		if(ticksdiff >= 1000)
		{
			if(global.dataSendToSlave.diveModeInfo != DIVEMODE_Apnea)
			{
				scheduleUpdateLifeData(0); // includes tissues
				global.lifeData.dive_time_seconds++; // there is dive_time_seconds_without_surface_time too
				global.lifeData.ppO2 = decom_calc_ppO2(global.lifeData.pressure_ambient_bar, &global.lifeData.actualGas);
				decom_oxygen_calculate_cns(&global.lifeData.cns,global.lifeData.ppO2);
				decom_oxygen_calculate_otu(&global.lifeData.otu,global.lifeData.ppO2);
				battery_gas_gauge_get_data();


				/** counter_exit allows safe exit via button for testing
					* and demo_mode is exited too if aplicable.
					*/
				if(global.dataSendToMaster.mode == MODE_ENDDIVE)
				{
					counter_exit++;
					if(counter_exit >= 2)
					{
						global.mode = MODE_SURFACE;
						global.demo_mode = 0;
					}
				}
				
				if(is_ambient_pressure_close_to_surface(&global.lifeData))
				{
					global.lifeData.counterSecondsShallowDepth++;
					if((global.lifeData.counterSecondsShallowDepth >= global.settings.timeoutDiveReachedZeroDepth) || ((global.lifeData.dive_time_seconds < 60) && (global.demo_mode == 0)) || (global.dataSendToSlave.setEndDive))
					{
						global.seconds_since_last_dive = 1; // start counter
						schedule_update_timer_helper(0); // zum starten :-)
						global.dataSendToMaster.mode = MODE_ENDDIVE;
						global.deviceDataSendToMaster.mode = MODE_ENDDIVE;
					}
				}
				else
				{
					global.lifeData.counterSecondsShallowDepth = 0;
					global.lifeData.dive_time_seconds_without_surface_time++;
				}
				vpm_crush2();
			}
			else // DIVEMODE_Apnea
			{
				global.lifeData.dive_time_seconds++;

				// exit dive mode
				if(global.dataSendToMaster.mode == MODE_ENDDIVE)
				{
					counter_exit++;
					if(counter_exit >= 2)
					{
						scheduleUpdateLifeData(-1); // 'restart' tissue calculations without calculating time during apnea mode
						global.lifeData.dive_time_seconds = 0; // use backup noflytime and desaturation time
						global.mode = MODE_SURFACE;
						global.demo_mode = 0;
					}
				}

				// surface break
				if(is_ambient_pressure_close_to_surface(&global.lifeData))
				{
					global.lifeData.counterSecondsShallowDepth++;
					if(global.lifeData.counterSecondsShallowDepth > 3) // time for main cpu to copy to apnea_last_dive_time_seconds
					{
						global.lifeData.dive_time_seconds = 0; // this apnea dive ends here
					}
					if((global.lifeData.counterSecondsShallowDepth >= global.settings.timeoutDiveReachedZeroDepth) || (global.dataSendToSlave.setEndDive))
					{
						global.dataSendToMaster.mode = MODE_ENDDIVE;
						global.deviceDataSendToMaster.mode = MODE_ENDDIVE;
					}
				}
				else
				{
					global.lifeData.counterSecondsShallowDepth = 0;
					global.lifeData.dive_time_seconds_without_surface_time++; 
				}
			} // standard dive or DIVEMODE_Apnea
		
			copyVpmCrushingData();
			copyTimeData();
			copyCnsAndOtuData();
			copyBatteryData();

			// new hw 170523
			if(global.I2C_SystemStatus != HAL_OK)
			{
				MX_I2C1_TestAndClear();
				MX_I2C1_Init();
				if(!is_init_pressure_done())
				{
					init_pressure();
				}
			}
			
			counterCompass100msec = 0;
			counterPressure100msec = 0;
			counterAmbientLight100msec = 0;
			counterWireless1msec = 0;
//			counterDemo250msec = 0;

			/** keep it in rhythm, do not drop the execution time
			  * therefore do not use tickstart = HAL_GetTick();
				* here
				*/
			tickstart += 1000; 
		}
	}
}


/**
  ******************************************************************************
* @brief   scheduleSurfaceMode / surface mode: Main Loop
  * @author  Peter Ryser
  * @version V0.0.1
  * @date    22-April-2014
  ******************************************************************************
  */


//  ===============================================================================
//	scheduleTestMode
/// @brief	included for sealed hardware with permanent RTE update message
//  ===============================================================================
void scheduleTestMode(void)
{
	uint32_t tickstart = 0;
	uint32_t ticksdiff = 0; 
	uint32_t lasttick = 0;
	tickstart = HAL_GetTick();

	uint8_t counterPressure100msec = 0;
	
	float temperature_carousel = 0.0f;
	float temperature_changer = 0.1f;
	
	while(global.mode == MODE_TEST)
	{
		schedule_check_resync();
		lasttick = HAL_GetTick();
		ticksdiff = time_elapsed_ms(tickstart,lasttick);
		
		//Evaluate pressure at 20 ms, 120 ms, 220 ms,...
		if(ticksdiff >= counterPressure100msec * 100 + 20)
		{
				global.check_sync_not_running++;

pressure_update();
scheduleUpdateDeviceData();
global.lifeData.ascent_rate_meter_per_min = 0;
copyPressureData();

				if(temperature_carousel > 20.0f)
				{
					temperature_carousel = 20.0f;
					temperature_changer = -0.1f;
				}
				else
				if(temperature_carousel < 0)
				{
					temperature_carousel = 0;
					temperature_changer = +0.1f;
				}
					
				temperature_carousel += temperature_changer;
				
				uint8_t boolPressureData = !global.dataSendToMaster.boolPressureData;

global.dataSendToMaster.data[boolPressureData].pressure_mbar = get_pressure_mbar();

				global.dataSendToMaster.data[boolPressureData].temperature = temperature_carousel;
				global.dataSendToMaster.data[boolPressureData].pressure_uTick = HAL_GetTick();
				global.dataSendToMaster.boolPressureData = boolPressureData;


				counterPressure100msec++;
		}
		
		if(ticksdiff >= 1000)
		{
			//Set back tick counter
			tickstart = HAL_GetTick();
			counterPressure100msec = 0;
		}
	};	
}


void scheduleSurfaceMode(void)
{
	uint32_t tickstart = 0;
	uint32_t ticksdiff = 0; 
	uint32_t lasttick = 0;
	tickstart = HAL_GetTick();
	uint8_t counterPressure100msec = 0;
	uint8_t counterCompass100msec = 0;
	uint8_t counterAmbientLight100msec = 0;
	uint16_t counterWireless1msec = 0;
	global.dataSendToMaster.mode = MODE_SURFACE;
	global.deviceDataSendToMaster.mode = MODE_SURFACE;

	while(global.mode == MODE_SURFACE)
	{
	/*    printf("surface...\n"); */
//	    SPI_Start_single_TxRx_with_Master();
		schedule_check_resync();
		lasttick = HAL_GetTick();
		ticksdiff = time_elapsed_ms(tickstart,lasttick);

		if(ticksdiff > counterWireless1msec)
		{
			counterWireless1msec++;
			changeAgeWirelessData();
			global.wirelessReceived = wireless_evaluate(global.wirelessdata,MAX_WIRELESS_BYTES, &global.wirelessConfidenceStatus);
			if((global.wirelessReceived  > 0) && !wireless_evaluate_crc_error(global.wirelessdata,global.wirelessReceived))
			{
				copyWirelessData();
			}
		}
		if(setButtonsNow == 1)
		{
			if(scheduleSetButtonResponsiveness())
				setButtonsNow = 0;
		}
		
		//Evaluate pressure at 20 ms, 120 ms, 220 ms,...
		if(ticksdiff >= counterPressure100msec * 100 + 20)
		{
				global.check_sync_not_running++;
				pressure_update();
				scheduleUpdateDeviceData();
				global.lifeData.ascent_rate_meter_per_min = 0;
				copyPressureData();
				counterPressure100msec++;
				
				if(scheduleCheck_pressure_reached_dive_mode_level())
					global.mode = MODE_DIVE;
		}
		
		//evaluate compass data at 50 ms, 150 ms, 250 ms,...

		if(ticksdiff >= counterCompass100msec * 100 + 50)
		{
			compass_read();
			acceleration_read();
			compass_calc();
			copyCompassData();
			counterCompass100msec++;
		}

		//evaluate compass data at 70 ms, 170 ms, 270 ms,...
		if(ticksdiff >= counterAmbientLight100msec * 100 + 70)
		{
			adc_ambient_light_sensor_get_data();
			copyAmbientLightData();
			counterAmbientLight100msec++;
		}
		//Evaluate tissues, toxic data, etc. once a second
		if(ticksdiff >= 1000)
		{
			if(clearDecoNow)
			{
				decom_reset_with_1000mbar(&global.lifeData); ///< this should almost reset desaturation time
				// new 160215 hw
				global.repetitive_dive = 0;
				global.seconds_since_last_dive = 0; ///< this will reset OTU and CNS as well
				global.no_fly_time_minutes = 0;
				global.accidentFlag = 0;
				global.accidentRemainingSeconds = 0;
				vpm_init(&global.vpm, global.conservatism, global.repetitive_dive, global.seconds_since_last_dive);
				clearDecoNow = 0;
			}
	
			//Set back tick counter
			tickstart = HAL_GetTick();


			if(global.seconds_since_last_dive)
			{
				schedule_update_timer_helper(-1);
//				global.seconds_since_last_dive++;
//				if(global.seconds_since_last_dive > 777900) // a bit more than nine days [seconds]
//					global.seconds_since_last_dive = 0;
			}

			if(global.accidentRemainingSeconds)
			{
				global.accidentRemainingSeconds--;
				if(!global.accidentRemainingSeconds)
					global.accidentFlag = 0;
			}
			global.dataSendToMaster.accidentFlags = global.accidentFlag;

			update_surface_pressure(1);
			scheduleUpdateLifeData(0);
			decom_oxygen_calculate_otu_degrade(&global.lifeData.otu, global.seconds_since_last_dive);
			decom_oxygen_calculate_cns_degrade(&global.lifeData.cns, global.seconds_since_last_dive);

			/* start desaturation calculation after first valid measurement has been done */
			if(global.lifeData.pressure_surface_bar != INVALID_PREASURE_VALUE)
			{
				global.lifeData.desaturation_time_minutes = decom_calc_desaturation_time(global.lifeData.tissue_nitrogen_bar,global.lifeData.tissue_helium_bar,global.lifeData.pressure_surface_bar);
			}
			else
			{
				global.lifeData.desaturation_time_minutes = 0;
			}
			battery_charger_get_status_and_contral_battery_gas_gauge(0);
			battery_gas_gauge_get_data();

			copyCnsAndOtuData();
			copyTimeData();
			copyBatteryData();
			copyDeviceData();

			// new hw 170523
			if(global.I2C_SystemStatus != HAL_OK)
			{
				MX_I2C1_TestAndClear();
				MX_I2C1_Init();
				if(!is_init_pressure_done())
				{
					init_pressure();
				}
			}
			
			counterCompass100msec = 0;
			counterPressure100msec = 0;
			counterAmbientLight100msec = 0;
			counterWireless1msec = 0;
		}
	}
}


/**
  ******************************************************************************
	* @brief   scheduleCompassCalibrationMode
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @since   31-March-2015
  * @date    31-March-2015
  ******************************************************************************
  */
void scheduleCompassCalibrationMode(void)
{
	compass_init(1,7); // fast mode, max gain
	compass_calib(); // duration : 1 minute!
	compass_init(0,7); // back to normal mode

	if(global.seconds_since_last_dive)
	{
				schedule_update_timer_helper(-1);
//		global.seconds_since_last_dive += 60;
//				if(global.seconds_since_last_dive > 777900) // a bit more than nine days [seconds]
//					global.seconds_since_last_dive = 0;
	}

	scheduleUpdateLifeData(0);
	global.mode = MODE_SURFACE;
}


/**
  ******************************************************************************
	* @brief   scheduleSleepMode / sleep mode: Main Loop
  * @author  heinrichs weikamp gmbh
  * @version V0.0.2
  * @since   31-March-2015
  * @date    22-April-2014
  ******************************************************************************
  */

void scheduleSleepMode(void)
{
	global.dataSendToMaster.mode = 0;
	global.deviceDataSendToMaster.mode = 0;
	
	/* prevent button wake up problem while in sleep_prepare
	 * sleep prepare does I2C_DeInit()
	 */
	if(global.mode != MODE_SLEEP)
		MX_I2C1_Init();
	else
	do
	{
		I2C_DeInit();

#ifdef DEBUGMODE
		HAL_Delay(2000);
#else
		RTC_StopMode_2seconds();
#endif
		

		
		if(global.mode == MODE_SLEEP)
			secondsCount += 2;

		MX_I2C1_Init();
		pressure_sensor_get_pressure_raw();

		if(secondsCount >= 30)
		{
			pressure_sensor_get_temperature_raw();
			battery_gas_gauge_get_data();
//			ReInit_battery_charger_status_pins();
			battery_charger_get_status_and_contral_battery_gas_gauge(1);
//			DeInit_battery_charger_status_pins();
			secondsCount = 0;
		}
		
		pressure_calculation();

		scheduleUpdateDeviceData();
		update_surface_pressure(2);

		if(global.seconds_since_last_dive)
		{
			schedule_update_timer_helper(-1);
//			global.seconds_since_last_dive += 2;
//				if(global.seconds_since_last_dive > 777900) // a bit more than nine days [seconds]
//					global.seconds_since_last_dive = 0;
		}

		if(global.accidentRemainingSeconds)
		{
			if(global.accidentRemainingSeconds > 2)			
				global.accidentRemainingSeconds -= 2;
			else
			{
				global.accidentRemainingSeconds = 0;
				global.accidentFlag = 0;
			}
		}

		if(scheduleCheck_pressure_reached_dive_mode_level())
			global.mode = MODE_BOOT;

		scheduleUpdateLifeData(2000);
	}
	while(global.mode == MODE_SLEEP);
	/* new section for system after Standby */
	scheduleUpdateLifeData(-1);
	clearDecoNow = 0;
	setButtonsNow = 0;
}



/* Private functions ---------------------------------------------------------*/


/**
  ******************************************************************************
	* @brief   scheduleCheck_pressure_reached_dive_mode_level
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1 from inline code
  * @date    09-Sept-2015
  ******************************************************************************
  */
_Bool scheduleCheck_pressure_reached_dive_mode_level(void)
{
		if(get_pressure_mbar() > 1160)
			return 1;
		else
		if((global.mode == MODE_SURFACE) && (get_pressure_mbar() > (get_surface_mbar() + 100)) && (get_surface_mbar() > 880))
			return 1;
		else
			return 0;
}
		

/**
  ******************************************************************************
	* @brief   scheduleUpdateLifeData / calculates tissues
  * @author  Peter Ryser
  * @version V0.0.1
  * @date    22-April-2014
  ******************************************************************************
  */
	
	
void scheduleUpdateLifeData(int32_t asynchron_milliseconds_since_last)
{
	static _Bool first = 1;
	static uint32_t tickstart = 0;
	static uint32_t ticksrest = 0;

	uint32_t ticksdiff = 0; 
	uint32_t ticksnow = 0;
	uint32_t time_seconds = 0;
	uint8_t whichGasTmp = 0;
	
	uint8_t updateTissueData = 0;


	if(global.lifeData.pressure_surface_bar == INVALID_PREASURE_VALUE)
	{
		updateTissueData = 1;
	}

	if(asynchron_milliseconds_since_last < 0)
	{
		first = 1;
		tickstart = 0;
		ticksrest = 0;
		return;
	}

	if(!asynchron_milliseconds_since_last && first)
	{
		tickstart = HAL_GetTick();
		first = 0;
		return;
	}

	whichGasTmp = global.whichGas;
	global.lifeData.actualGas = global.aktualGas[whichGasTmp];
	global.lifeData.pressure_ambient_bar = get_pressure_mbar() / 1000.0f;
	global.lifeData.pressure_surface_bar = get_surface_mbar() / 1000.0f;

	if(updateTissueData)
	{
		decom_reset_with_ambientmbar(global.lifeData.pressure_surface_bar,&global.lifeData);
	}

	if(!asynchron_milliseconds_since_last)
	{
		ticksnow = HAL_GetTick();
		ticksdiff = time_elapsed_ms(tickstart,ticksnow);
	}
	else
	{
		first = 1;
		ticksdiff = asynchron_milliseconds_since_last;
	}

	if(ticksrest > 1000) 	// whatever happens after standby with STM32L476
		ticksrest = 0;			// maybe move static to SRAM2 

	ticksdiff += ticksrest;
	time_seconds = ticksdiff/ 1000;
	ticksrest = ticksdiff - time_seconds * 1000; 
	tickstart = ticksnow;

	decom_tissues_exposure((int)time_seconds, &global.lifeData);
	if(global.demo_mode)
		decom_tissues_exposure((int)(3*time_seconds), &global.lifeData);
  copyTissueData();
}


/**
  ******************************************************************************
* @brief   scheduleUpdateDeviceData
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    16-March-2015
	*
	* two step process
	* first compare with data from main CPU == externalLogbookFlash
	* second update with new sensor data
  ******************************************************************************
  */
void scheduleSetDate(SDeviceLine *line)
{
	extern RTC_HandleTypeDef RTCHandle;

	line->date_rtc_dr = (uint32_t)(RTCHandle.Instance->DR & RTC_DR_RESERVED_MASK); 
	line->time_rtc_tr = (uint32_t)(RTCHandle.Instance->TR & RTC_TR_RESERVED_MASK); 
}


void scheduleCopyDeviceData(SDeviceLine *lineWrite, const SDeviceLine *lineRead)
{	
	lineWrite->date_rtc_dr = lineRead->date_rtc_dr; 
	lineWrite->time_rtc_tr = lineRead->time_rtc_tr;
	lineWrite->value_int32 = lineRead->value_int32;
}


void scheduleUpdateDeviceData(void)
{
	/* first step, main CPU */

	if(deviceDataFlashValid)
	{
		/* max values */
		if(global.deviceData.batteryChargeCompleteCycles.value_int32 < DeviceDataFlash.batteryChargeCompleteCycles.value_int32)
		{
			scheduleCopyDeviceData(&global.deviceData.batteryChargeCompleteCycles, &DeviceDataFlash.batteryChargeCompleteCycles);
		}
		if(global.deviceData.batteryChargeCycles.value_int32 < DeviceDataFlash.batteryChargeCycles.value_int32)
		{
			scheduleCopyDeviceData(&global.deviceData.batteryChargeCycles, &DeviceDataFlash.batteryChargeCycles);
		}
		if(global.deviceData.temperatureMaximum.value_int32 < DeviceDataFlash.temperatureMaximum.value_int32)
		{
			scheduleCopyDeviceData(&global.deviceData.temperatureMaximum, &DeviceDataFlash.temperatureMaximum);
		}
		if(global.deviceData.depthMaximum.value_int32 < DeviceDataFlash.depthMaximum.value_int32)
		{
			scheduleCopyDeviceData(&global.deviceData.depthMaximum, &DeviceDataFlash.depthMaximum);
		}
		if(global.deviceData.diveCycles.value_int32 < DeviceDataFlash.diveCycles.value_int32)
		{
			scheduleCopyDeviceData(&global.deviceData.diveCycles, &DeviceDataFlash.diveCycles);
		}
		if(global.deviceData.hoursOfOperation.value_int32 < DeviceDataFlash.hoursOfOperation.value_int32)
		{
			scheduleCopyDeviceData(&global.deviceData.hoursOfOperation, &DeviceDataFlash.hoursOfOperation);
		}
		
		/* min values */
		if(global.deviceData.temperatureMinimum.value_int32 > DeviceDataFlash.temperatureMinimum.value_int32)
		{
			scheduleCopyDeviceData(&global.deviceData.temperatureMinimum, &DeviceDataFlash.temperatureMinimum);
		}
		if(global.deviceData.voltageMinimum.value_int32 > DeviceDataFlash.voltageMinimum.value_int32)
		{
			scheduleCopyDeviceData(&global.deviceData.voltageMinimum, &DeviceDataFlash.voltageMinimum);
		}
	}		
		
	/* second step, sensor data */
	int32_t temperature_centigrad_int32;
	int32_t pressure_mbar_int32;
	int32_t voltage_mvolt_int32;
	
	temperature_centigrad_int32 = (int32_t)(get_temperature() * 100);
	if(temperature_centigrad_int32 < global.deviceData.temperatureMinimum.value_int32)
	{
		global.deviceData.temperatureMinimum.value_int32 = temperature_centigrad_int32;
		scheduleSetDate(&global.deviceData.temperatureMinimum);
	}

	if(temperature_centigrad_int32 > global.deviceData.temperatureMaximum.value_int32)
	{
		global.deviceData.temperatureMaximum.value_int32 = temperature_centigrad_int32;
		scheduleSetDate(&global.deviceData.temperatureMaximum);
	}

	pressure_mbar_int32 = (int32_t)get_pressure_mbar();
	if(pressure_mbar_int32 > global.deviceData.depthMaximum.value_int32)
	{
		global.deviceData.depthMaximum.value_int32 = pressure_mbar_int32;
		scheduleSetDate(&global.deviceData.depthMaximum);
	}
	
	voltage_mvolt_int32 = (int32_t)(get_voltage() * 1000);
	if(voltage_mvolt_int32 < global.deviceData.voltageMinimum.value_int32)
	{
		global.deviceData.voltageMinimum.value_int32 = voltage_mvolt_int32;
		scheduleSetDate(&global.deviceData.voltageMinimum);
	}

	/* third step, counter */
	switch (global.mode)
	{
		case MODE_SURFACE:
		case MODE_DIVE:
		default:
			deviceDataSubSeconds++;
			if(deviceDataSubSeconds > 10)
			{
				deviceDataSubSeconds = 0;
				global.deviceData.hoursOfOperation.value_int32++;
			}
			break;

		case	MODE_SLEEP:
		case MODE_SHUTDOWN:
			break;
	}
}


void scheduleUpdateDeviceDataChargerFull(void)
{
	global.deviceData.batteryChargeCompleteCycles.value_int32++;
	scheduleSetDate(&global.deviceData.batteryChargeCompleteCycles);
}


void scheduleUpdateDeviceDataChargerCharging(void)
{
	global.deviceData.batteryChargeCycles.value_int32++;
	scheduleSetDate(&global.deviceData.batteryChargeCycles);
}


/**
  ******************************************************************************
* @brief   vpm_crush / calls vpm calc_crushing_pressure every four seconds during descend
  * @author  Peter Ryser
  * @version V0.0.1
  * @date    22-April-2014
  ******************************************************************************
  */
_Bool vpm_crush2(void)
{
    int i = 0;
    static float starting_ambient_pressure = 0;
    static float ending_ambient_pressure = 0;
    static float time_calc_begin = -1;
		static float initial_helium_pressure[16];
		static float initial_nitrogen_pressure[16];
		ending_ambient_pressure = global.lifeData.pressure_ambient_bar * 10;

	if((global.lifeData.dive_time_seconds <= 4) || (starting_ambient_pressure >= ending_ambient_pressure))
	{
		time_calc_begin = global.lifeData.dive_time_seconds;
		starting_ambient_pressure = global.lifeData.pressure_ambient_bar * 10;
		for( i = 0; i < 16; i++)
		{
			initial_helium_pressure[i] = global.lifeData.tissue_helium_bar[i] * 10;
			initial_nitrogen_pressure[i] = global.lifeData.tissue_nitrogen_bar[i] * 10;
		}
		return 0;
	}
	if(global.lifeData.dive_time_seconds - time_calc_begin >= 4)
	{
		if(ending_ambient_pressure > starting_ambient_pressure + 0.5f)
		{
			float rate = (ending_ambient_pressure - starting_ambient_pressure) * 60 / 4;
			calc_crushing_pressure(&global.lifeData, &global.vpm, initial_helium_pressure, initial_nitrogen_pressure, starting_ambient_pressure, rate);

			time_calc_begin = global.lifeData.dive_time_seconds;
			starting_ambient_pressure = global.lifeData.pressure_ambient_bar * 10;
			for( i = 0; i < 16; i++)
			{
				initial_helium_pressure[i] = global.lifeData.tissue_helium_bar[i] * 10;
				initial_nitrogen_pressure[i] =  global.lifeData.tissue_nitrogen_bar[i] * 10;
			}

			return 1;
		}

	}
	return 0;
};


void initStructWithZeero(uint8_t* data, uint16_t length)
{
	for(uint16_t i = 0; i < length; i++)
		data[i] = 0;
}


long get_nofly_time_minutes(void)
{
	
	if(global.no_fly_time_minutes <= 0)
		return 0;
	
	long minutes_since_last_dive = global.seconds_since_last_dive/60;

	if((global.seconds_since_last_dive > 0) && (global.no_fly_time_minutes > minutes_since_last_dive))
	{
			return (global.no_fly_time_minutes - minutes_since_last_dive);
	}
	else
	{
		global.no_fly_time_minutes = 0;
		return 0;
	}
}


//Supports threadsave copying!!!
void copyActualGas(SGas gas)
{
	uint8_t whichGas = !global.whichGas;
	global.aktualGas[whichGas] = gas;
	global.whichGas = whichGas;
}


//Supports threadsave copying!!!
void copyPressureData(void)
{
	global.dataSendToMaster.sensorErrors = I2C1_Status();
	//uint8_t dataSendToMaster.
	uint8_t boolPressureData = !global.dataSendToMaster.boolPressureData;
	global.dataSendToMaster.data[boolPressureData].temperature = get_temperature();
	global.dataSendToMaster.data[boolPressureData].pressure_mbar = get_pressure_mbar();
	global.dataSendToMaster.data[boolPressureData].surface_mbar = get_surface_mbar();
	global.dataSendToMaster.data[boolPressureData].ascent_rate_meter_per_min = global.lifeData.ascent_rate_meter_per_min;
	global.dataSendToMaster.data[boolPressureData].pressure_uTick = HAL_GetTick();
	global.dataSendToMaster.boolPressureData = boolPressureData;
}


//Supports threadsave copying!!!
void copyCnsAndOtuData(void)
{
	//uint8_t dataSendToMaster.
	uint8_t boolToxicData = !global.dataSendToMaster.boolToxicData;
	global.dataSendToMaster.data[boolToxicData].cns = global.lifeData.cns;
	global.dataSendToMaster.data[boolToxicData].otu = global.lifeData.otu;
	global.dataSendToMaster.data[boolToxicData].desaturation_time_minutes = global.lifeData.desaturation_time_minutes;
	global.dataSendToMaster.data[boolToxicData].no_fly_time_minutes = get_nofly_time_minutes();
	global.dataSendToMaster.boolToxicData = boolToxicData;
}


//Supports threadsave copying!!!
void copyTimeData(void)
{
	extern RTC_HandleTypeDef RTCHandle;
	
	uint8_t boolTimeData = !global.dataSendToMaster.boolTimeData;
	global.dataSendToMaster.data[boolTimeData].localtime_rtc_tr =  (uint32_t)(RTCHandle.Instance->TR & RTC_TR_RESERVED_MASK); 
	global.dataSendToMaster.data[boolTimeData].localtime_rtc_dr =  (uint32_t)(RTCHandle.Instance->DR & RTC_DR_RESERVED_MASK); 
	global.dataSendToMaster.data[boolTimeData].divetime_seconds = (uint32_t)global.lifeData.dive_time_seconds;
	global.dataSendToMaster.data[boolTimeData].dive_time_seconds_without_surface_time = (uint32_t)global.lifeData.dive_time_seconds_without_surface_time;
	global.dataSendToMaster.data[boolTimeData].surfacetime_seconds = (uint32_t)global.seconds_since_last_dive;
	global.dataSendToMaster.data[boolTimeData].counterSecondsShallowDepth = (uint32_t)global.lifeData.counterSecondsShallowDepth;
	global.dataSendToMaster.boolTimeData = boolTimeData;
}


//Supports threadsave copying!!!
void copyCompassData(void)
{
	extern float compass_heading;
	extern float compass_roll;
	extern float compass_pitch;
	//uint8_t dataSendToMaster.
	uint8_t boolCompassData = !global.dataSendToMaster.boolCompassData;
	global.dataSendToMaster.data[boolCompassData].compass_heading = compass_heading;
	global.dataSendToMaster.data[boolCompassData].compass_roll = compass_roll;
	global.dataSendToMaster.data[boolCompassData].compass_pitch = compass_pitch;
	global.dataSendToMaster.data[boolCompassData].compass_DX_f = 0;
	global.dataSendToMaster.data[boolCompassData].compass_DY_f = 0;
	global.dataSendToMaster.data[boolCompassData].compass_DZ_f = 0;
	global.dataSendToMaster.data[boolCompassData].compass_uTick = HAL_GetTick();
	global.dataSendToMaster.boolCompassData = boolCompassData;
}


void copyCompassDataDuringCalibration(int16_t dx, int16_t dy, int16_t dz)
{
	extern float compass_heading;
	extern float compass_roll;
	extern float compass_pitch;
	//uint8_t dataSendToMaster.
	uint8_t boolCompassData = !global.dataSendToMaster.boolCompassData;
	global.dataSendToMaster.data[boolCompassData].compass_heading = compass_heading;
	global.dataSendToMaster.data[boolCompassData].compass_roll = compass_roll;
	global.dataSendToMaster.data[boolCompassData].compass_pitch = compass_pitch;
	global.dataSendToMaster.data[boolCompassData].compass_DX_f = dx;
	global.dataSendToMaster.data[boolCompassData].compass_DY_f = dy;
	global.dataSendToMaster.data[boolCompassData].compass_DZ_f = dz;
	global.dataSendToMaster.boolCompassData = boolCompassData;
}


//Supports threadsave copying!!!
void copyBatteryData(void)
{
	uint8_t boolBatteryData = !global.dataSendToMaster.boolBatteryData;
	global.dataSendToMaster.data[boolBatteryData].battery_voltage = get_voltage();
	global.dataSendToMaster.data[boolBatteryData].battery_charge= get_charge();
	global.dataSendToMaster.boolBatteryData = boolBatteryData;
}


//Supports threadsave copying!!!
void copyAmbientLightData(void)
{
	uint8_t boolAmbientLightData = !global.dataSendToMaster.boolAmbientLightData;
	global.dataSendToMaster.data[boolAmbientLightData].ambient_light_level = get_ambient_light_level();
	global.dataSendToMaster.boolAmbientLightData = boolAmbientLightData;
}


//Supports threadsave copying!!!
void copyTissueData(void)
{
	//uint8_t dataSendToMaster.
	uint8_t boolTisssueData = !global.dataSendToMaster.boolTisssueData;
	for(int i = 0; i < 16; i++)
	{
		global.dataSendToMaster.data[boolTisssueData].tissue_nitrogen_bar[i] = global.lifeData.tissue_nitrogen_bar[i];
		global.dataSendToMaster.data[boolTisssueData].tissue_helium_bar[i] = global.lifeData.tissue_helium_bar[i];
	}
	global.dataSendToMaster.boolTisssueData = boolTisssueData;
}


//Supports threadsave copying!!!
void copyVpmCrushingData(void)
{
	//uint8_t dataSendToMaster.
	uint8_t boolCrushingData = !global.dataSendToMaster.boolCrushingData;
	for(int i = 0; i < 16; i++)
	{
		global.dataSendToMaster.data[boolCrushingData].max_crushing_pressure_n2[i] = global.vpm.max_crushing_pressure_n2[i];
		global.dataSendToMaster.data[boolCrushingData].max_crushing_pressure_he[i] = global.vpm.max_crushing_pressure_he[i];
		global.dataSendToMaster.data[boolCrushingData].adjusted_critical_radius_he[i] = global.vpm.adjusted_critical_radius_he[i];
		global.dataSendToMaster.data[boolCrushingData].adjusted_critical_radius_n2[i] = global.vpm.adjusted_critical_radius_n2[i];
	}
	global.dataSendToMaster.boolCrushingData = boolCrushingData;
}


void copyDeviceData(void)
{
	uint8_t boolDeviceData = !global.deviceDataSendToMaster.boolDeviceData;
	memcpy(&global.deviceDataSendToMaster.DeviceData[boolDeviceData], &global.deviceData,sizeof(SDevice));
	global.deviceDataSendToMaster.boolDeviceData = boolDeviceData; 

	global.deviceDataSendToMaster.boolVpmRepetitiveDataValid = 0;
	memcpy(&global.deviceDataSendToMaster.VpmRepetitiveData.adjusted_critical_radius_he, 		&global.vpm.adjusted_critical_radius_he,		sizeof(16*4));
	memcpy(&global.deviceDataSendToMaster.VpmRepetitiveData.adjusted_critical_radius_n2, 		&global.vpm.adjusted_critical_radius_n2,		sizeof(16*4));
	memcpy(&global.deviceDataSendToMaster.VpmRepetitiveData.adjusted_crushing_pressure_he,	&global.vpm.adjusted_crushing_pressure_he,	sizeof(16*4));
	memcpy(&global.deviceDataSendToMaster.VpmRepetitiveData.adjusted_crushing_pressure_n2,	&global.vpm.adjusted_crushing_pressure_n2,	sizeof(16*4));
	memcpy(&global.deviceDataSendToMaster.VpmRepetitiveData.initial_allowable_gradient_he,	&global.vpm.initial_allowable_gradient_he,	sizeof(16*4));
	memcpy(&global.deviceDataSendToMaster.VpmRepetitiveData.initial_allowable_gradient_n2,	&global.vpm.initial_allowable_gradient_n2,	sizeof(16*4));
	memcpy(&global.deviceDataSendToMaster.VpmRepetitiveData.max_actual_gradient, 					 	&global.vpm.max_actual_gradient,						sizeof(16*4));
	global.deviceDataSendToMaster.VpmRepetitiveData.repetitive_variables_not_valid = global.vpm.repetitive_variables_not_valid;
	global.deviceDataSendToMaster.boolVpmRepetitiveDataValid = 1;
}

void changeAgeWirelessData(void)
{
	for(int i=0;i<4;i++)
	{
		if(global.dataSendToMaster.data[global.dataSendToMaster.boolWirelessData].wireless_data[i].ageInMilliSeconds)
			global.dataSendToMaster.data[global.dataSendToMaster.boolWirelessData].wireless_data[i].ageInMilliSeconds++;
	}
}

void copyWirelessData(void)
{
	uint8_t boolWirelessData = !global.dataSendToMaster.boolWirelessData;
	SDataWireless *dataOld, *dataNew;
	for(int i=0;i<3;i++)
	{
		dataNew = &global.dataSendToMaster.data[boolWirelessData].wireless_data[i+1];
		dataOld = &global.dataSendToMaster.data[!boolWirelessData].wireless_data[i];
		dataNew->ageInMilliSeconds = dataOld->ageInMilliSeconds;
		dataNew->numberOfBytes = dataOld->numberOfBytes;
		dataNew->status = dataOld->status;
		memcpy(dataNew->data, dataOld->data,8);
	}
	
	global.dataSendToMaster.data[boolWirelessData].wireless_data[0].ageInMilliSeconds = 1;
	global.dataSendToMaster.data[boolWirelessData].wireless_data[0].numberOfBytes = global.wirelessReceived;
	global.dataSendToMaster.data[boolWirelessData].wireless_data[0].status = global.wirelessConfidenceStatus;
	for(int i=0;i<MAX_WIRELESS_BYTES;i++)
		global.dataSendToMaster.data[boolWirelessData].wireless_data[0].data[i] = global.wirelessdata[i];
	for(int i=MAX_WIRELESS_BYTES;i<12;i++)
		global.dataSendToMaster.data[boolWirelessData].wireless_data[0].data[i] = 0xFF;
	
	global.dataSendToMaster.boolWirelessData = boolWirelessData;
}

/* 			copyPICdata(); is used in spi.c */
void copyPICdata(void)
{
	uint8_t boolPICdata = !global.dataSendToMaster.boolPICdata;
	for(int i = 0; i < 3; i++)
	{
		global.dataSendToMaster.data[boolPICdata].button_setting[i] = global.ButtonPICdata[i];
	}
	global.dataSendToMaster.boolPICdata = boolPICdata;
}




typedef enum 
{
  SPI3_OK      = 0x00,
  SPI3_DEINIT  = 0x01,
} SPI3_StatusTypeDef;
/* if spi3 is running and the SPI3_ButtonAdjust call returns OK, all is fine
	 if the SPI3_ButtonAdjust call returns error, the spi3 is DeInit
	 and will be init the next call of scheduleSetButtonResponsiveness()
	 and data will be send again on the third call
	 therefore on return 0 of scheduleSetButtonResponsiveness() the caller flag should kept active
*/
uint8_t scheduleSetButtonResponsiveness(void)
{
	static uint8_t SPI3status = SPI3_OK;
	
	if((SPI3status == SPI3_OK) && (SPI3_ButtonAdjust(global.ButtonResponsiveness, global.ButtonPICdata)))
	{
		copyPICdata();
		return 1;
	}
	else
	{
		for(int i=0;i<3;i++)
		{
			global.ButtonPICdata[i] = 0xFF;
		}
		copyPICdata();
		
		if(SPI3status == SPI3_OK)
		{
			MX_SPI3_DeInit();
			SPI3status = SPI3_DEINIT;
		}
		else
		{
			MX_SPI3_Init();
			SPI3status = SPI3_OK;
		}
		return 0;
	}
}


//save time diffenrence
uint32_t time_elapsed_ms(uint32_t ticksstart,uint32_t ticksnow)
{
	if(ticksstart <= ticksnow)
	{
			return ticksnow - ticksstart;
	}
	else
	{
		return 0xFFFFFFFF - ticksstart + ticksnow;
	}
}

/* same as in data_central.c */
_Bool is_ambient_pressure_close_to_surface(SLifeData *lifeDataCall)
{
	if(lifeDataCall->pressure_ambient_bar < (lifeDataCall->pressure_surface_bar + 0.1f)) // hw 161121 now 1 mter, before 0.04f
		return true;
	else
		return false;
}


/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/

