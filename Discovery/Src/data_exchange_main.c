/**
  ******************************************************************************
  * @file    data_exchange_main.c
  * @author  heinrichs weikamp gmbh
  * @date    13-Oct-2014
  * @version V0.0.3
  * @since   17-Feb-2016

	* @brief   Communication with the second CPU == RTE system
  *
  @verbatim
  ==============================================================================
                        ##### Version Changes #####
  ==============================================================================
	160217 V0.0.3 pStateUsed->decolistXXXXX.tickstamp = HAL_GetTick(); added
	150627 V0.0.2 
	
  ==============================================================================
                        ##### How to use #####
  ==============================================================================

  ==============================================================================
            ##### Button, Set Time, Clear Deco etc Request #####
  ==============================================================================
	was updated (151207) for buttons and clear deco at the moment only
	using requestNecessary and checking in DataEX_copy_to_LifeData()
	Hence if there is no confirm from the smallCPU on the data after the request
	the request will be send again.
	
	==============================================================================
                        ##### Device Data #####
  ==============================================================================
	
	main CPU always sends the device data info that it has at the moment

		on start it is INT32_MIN, INT32_MAX and 0 
		as initialized  in data_central.c variable declaration
	
	second small CPU gets request to send its device data
		
		on receiption the data is merged with the data in externLogbookFlash,
		stored on the externLogbookFlash and from now on send to small CPU

  ==============================================================================
            ##### Magnet Reset #####
  ==============================================================================

	@endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h> // for memcopy
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "ostc.h"
#include "settings.h"
#include "data_central.h"
#include "data_exchange_main.h"
#include "base.h"
#include "decom.h"
#include "calc_crush.h" /* for vpm_init */
#include "simulation.h"
#include "tCCR.h"
#include "timer.h"
#include "buehlmann.h"
#include "externLogbookFlash.h"
#include "bonex_mini.h" // for voltage to battery percentage


/* Expoted variables --------------------------------------------------------*/
uint8_t	wasPowerOn = 0;
confirmbit8_Type requestNecessary = { .uw = 0 };
uint8_t wasUpdateNotPowerOn = 0;
uint8_t scooterFoundThisPowerOnCylce = 0;

/* Private variables with external access ------------------------------------*/


/* Private variables ---------------------------------------------------------*/
uint8_t	told_reset_logik_alles_ok = 0;

SDataReceiveFromMaster dataOut;
SDataExchangeSlaveToMaster dataIn;

uint32_t systick_last;
uint8_t data_old__lost_connection_to_slave_counter_temp = 0;
uint8_t data_old__lost_connection_to_slave_counter_retry = 0;
uint32_t data_old__lost_connection_to_slave_counter_total = 0;

/* Private types -------------------------------------------------------------*/

typedef enum
{
  CPU2_TRANSFER_STOP	       = 0x00,    /*!<     */
  CPU2_TRANSFER_TEST_REQUEST = 0x01,    /*!<     */
  CPU2_TRANSFER_TEST_RECEIVE = 0x02,    /*!<     */
  CPU2_TRANSFER_SEND_OK      = 0x03,    /*!<     */
  CPU2_TRANSFER_SEND_FALSE   = 0x04,    /*!<     */
  CPU2_TRANSFER_DATA			   = 0x05,    /*!<     */
}CPU2_TRANSFER_StatusTypeDef;

const uint8_t header_test_request[4]	= {0xBB, 0x00, 0x00, 0xBB};
const uint8_t header_test_receive[4]	= {0xBB, 0x01, 0x01, 0xBB};
const uint8_t header_false[4]					= {0xBB, 0xFF, 0xFF, 0xBB};
const uint8_t header_correct[4]				= {0xBB, 0xCC, 0xCC, 0xBB};
const uint8_t header_data[4]					= {0xAA, 0x01, 0x01, 0xAA};

/* Private function prototypes -----------------------------------------------*/
uint8_t DataEX_check_header_and_footer_ok(void);
uint8_t DataEX_check_header_and_footer_devicedata(void);
void DataEX_check_DeviceData(void);

/* Exported functions --------------------------------------------------------*/
void DataEX_set_update_RTE_not_power_on(void)
{
	wasUpdateNotPowerOn = 1;
}


uint8_t DataEX_was_power_on(void)
{
	return wasPowerOn;
}

uint8_t count_DataEX_Error_Handler = 0;
uint8_t last_error_DataEX_Error_Handler = 0;

void DataEX_Error_Handler(uint8_t answer)
{
	count_DataEX_Error_Handler++;
	last_error_DataEX_Error_Handler = answer;
  return;
}


uint32_t DataEX_lost_connection_count(void)
{
	return data_old__lost_connection_to_slave_counter_total; 
}


uint32_t DataEX_time_elapsed_ms(uint32_t ticksstart,uint32_t ticksnow)
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

SDataReceiveFromMaster * dataOutGetPointer(void)
{
	return &dataOut;
}

void DataEX_init(void)
{
	SDiveState * pStateReal = stateRealGetPointerWrite();
	pStateReal->data_old__lost_connection_to_slave = 0; //initial value
	data_old__lost_connection_to_slave_counter_temp = 0;
	data_old__lost_connection_to_slave_counter_total = 0;

	memset((void *)&dataOut, 0, sizeof(SDataReceiveFromMaster));
	// old 160307:	for(int i=0;i<EXCHANGE_BUFFERSIZE;i++)
//		*(uint8_t *)(((uint32_t)&dataOut) + i)  = 0;

	dataOut.header.checkCode[0] = 0xBB;
	dataOut.header.checkCode[1] = 0x01;
	dataOut.header.checkCode[2] = 0x01;
	dataOut.header.checkCode[3] = 0xBB;

	dataOut.footer.checkCode[0] = 0xF4;
	dataOut.footer.checkCode[1] = 0xF3;
	dataOut.footer.checkCode[2] = 0xF2;
	dataOut.footer.checkCode[3] = 0xF1;


	pStateReal->lifeData.scooterType							= 0xFF;
	pStateReal->lifeData.scooterWattstunden 			= 0;
	pStateReal->lifeData.scooterRestkapazitaet 		= 0;
	pStateReal->lifeData.scooterDrehzahl 					= 0;
	pStateReal->lifeData.scooterSpannung					= 0;
	pStateReal->lifeData.scooterTemperature 			= 0;
	pStateReal->lifeData.scooterAmpere				 		= 0;
	pStateReal->lifeData.scooterRestkapazitaetWhBased	= 0;
	pStateReal->lifeData.scooterRestkapazitaetVoltageBased	= 0;
	pStateReal->lifeData.scooterAgeInMilliSeconds = 0;
	
	systick_last = HAL_GetTick() - 100;
}


void DataEx_call_helper_requests(void)
{
	static uint8_t setDateWasSend = 0;
	static uint8_t setTimeWasSend = 0;
	static uint8_t calibrateCompassWasSend = 0;
	static uint8_t setButtonSensitivityWasSend = 0;
	static uint8_t clearDecoWasSend = 0;
	static uint8_t getDeviceDataWasSend = 0;	
	static uint8_t setAccidentFlagWasSend = 0;
	static uint8_t setEndDiveWasSend = 0;
	
	if(getDeviceDataWasSend)
	{
		dataOut.getDeviceDataNow = 0;
		requestNecessary.ub.devicedata = 1;
	}
	getDeviceDataWasSend = 0;
	if(dataOut.getDeviceDataNow)
	{
		getDeviceDataWasSend = 1;
	}
	
	if(setEndDiveWasSend)
	{
		dataOut.setEndDive = 0;
		//requestNecessary.ub.XXX = 1; not implemented and no space here
	}
	setEndDiveWasSend = 0;
	if(dataOut.setEndDive)
	{
		setEndDiveWasSend = 1;
	}
	
	if(setAccidentFlagWasSend)
	{
		dataOut.setAccidentFlag = 0;
		requestNecessary.ub.accident = 1;
	}
	setAccidentFlagWasSend = 0;
	if(dataOut.setAccidentFlag)
	{
		setAccidentFlagWasSend = 1;
	}

	if(setDateWasSend)
	{
		dataOut.setDateNow = 0;
		requestNecessary.ub.date = 1;
	}
	setDateWasSend = 0;
	if(dataOut.setDateNow)
	{
		setDateWasSend = 1;
	}

	if(setTimeWasSend)
	{
		dataOut.setTimeNow = 0;
		requestNecessary.ub.time = 1;
	}
	setTimeWasSend = 0;
	if(dataOut.setTimeNow)
	{
		setTimeWasSend = 1;
	}

	if(calibrateCompassWasSend)
	{
		dataOut.calibrateCompassNow = 0;
		requestNecessary.ub.compass = 1;
	}
	calibrateCompassWasSend = 0;
	if(dataOut.calibrateCompassNow)
	{
		calibrateCompassWasSend = 1;
	}

	if(clearDecoWasSend)
	{
		dataOut.clearDecoNow = 0;
		requestNecessary.ub.clearDeco = 1;
	}
	if(dataOut.clearDecoNow)
	{
		clearDecoWasSend = 1;
	}
	
	if(setButtonSensitivityWasSend)
	{
		dataOut.setButtonSensitivityNow = 0;
		requestNecessary.ub.button = 1;
	}
	setButtonSensitivityWasSend = 0;
	if(dataOut.setButtonSensitivityNow)
	{
		setButtonSensitivityWasSend = 1;
	}
}


uint8_t DataEX_call(void)
{
	uint8_t SPI_DMA_answer = 0;
	
	HAL_GPIO_WritePin(SMALLCPU_CSB_GPIO_PORT,SMALLCPU_CSB_PIN,GPIO_PIN_SET);
	delayMicros(20); //~exchange time(+20% reserve)
	HAL_GPIO_WritePin(SMALLCPU_CSB_GPIO_PORT,SMALLCPU_CSB_PIN,GPIO_PIN_RESET);
	/* one cycle with NotChipSelect true to clear slave spi buffer */

	if(data_old__lost_connection_to_slave_counter_temp >= 3)
	{
		data_old__lost_connection_to_slave_counter_temp = 0;
		data_old__lost_connection_to_slave_counter_retry++;
	}
//	else
//	{
//		HAL_GPIO_WritePin(SMALLCPU_CSB_GPIO_PORT,SMALLCPU_CSB_PIN,GPIO_PIN_RESET);
//	}

	DataEx_call_helper_requests();

	systick_last = HAL_GetTick();

//HAL_GPIO_WritePin(OSCILLOSCOPE2_GPIO_PORT,OSCILLOSCOPE2_PIN,GPIO_PIN_RESET); /* only for testing with Oscilloscope */

	SPI_DMA_answer = HAL_SPI_TransmitReceive_DMA(&cpu2DmaSpi, (uint8_t *)&dataOut, (uint8_t *)&dataIn, EXCHANGE_BUFFERSIZE);
//	HAL_Delay(3);
	if(SPI_DMA_answer != HAL_OK)
    DataEX_Error_Handler(SPI_DMA_answer);
//	HAL_GPIO_WritePin(SMALLCPU_CSB_GPIO_PORT,SMALLCPU_CSB_PIN,GPIO_PIN_SET);
//HAL_Delay(3);
//HAL_GPIO_WritePin(OSCILLOSCOPE2_GPIO_PORT,OSCILLOSCOPE2_PIN,GPIO_PIN_SET); /* only for testing with Oscilloscope */

	return 1;
}


uint32_t SPI_CALLBACKS;
uint32_t get_num_SPI_CALLBACKS(void){
	return SPI_CALLBACKS;
}

SDataExchangeSlaveToMaster* get_dataInPointer(void){
	return &dataIn;
}


void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{


	if(hspi == &cpu2DmaSpi)
	{
		SPI_CALLBACKS+=1;
	}
}





void DateEx_copy_to_dataOut(void)
{
	const SDiveState * pStateReal = stateRealGetPointer();
	SSettings *settings = settingsGetPointer();

	if(get_globalState() == StStop)
		dataOut.mode = MODE_SHUTDOWN;
	else
		dataOut.mode = 0;

	dataOut.diveModeInfo = pStateReal->diveSettings.diveMode; // hw 170215
	
	memcpy(&dataOut.data.DeviceData, stateDeviceGetPointer(), sizeof(SDevice));

	dataOut.data.VPMconservatism = pStateReal->diveSettings.vpm_conservatism;
	dataOut.data.actualGas = pStateReal->lifeData.actualGas;
	dataOut.data.ambient_pressure_mbar_ceiling = (pStateReal->decolistBuehlmann.output_ceiling_meter * 100) + (pStateReal->lifeData.pressure_surface_bar * 1000);
	dataOut.data.divetimeToCreateLogbook = settings->divetimeToCreateLogbook;
	dataOut.data.timeoutDiveReachedZeroDepth = settings->timeoutDiveReachedZeroDepth;
	
	dataOut.data.offsetPressureSensor_mbar = settings->offsetPressure_mbar;
	dataOut.data.offsetTemperatureSensor_centiDegree = settings->offsetTemperature_centigrad;

	if((hardwareDataGetPointer()->primarySerial <= 32) || (((hardwareDataGetPointer()->primarySerial == 72) && (hardwareDataGetPointer()->secondarySerial == 15))))
	{
		dataOut.revisionHardware = 0x00;
		dataOut.revisionCRCx0x7A = 0x7A;
	}
	else
	if(hardwareDataGetPointer()->primarySerial < 0xFFFF)
	{
		dataOut.revisionHardware = hardwareDataGetPointer()->revision8bit;
		dataOut.revisionCRCx0x7A = hardwareDataGetPointer()->revision8bit ^ 0x7A;
	}
	else
	{
		dataOut.revisionHardware = 0xFF;
		dataOut.revisionCRCx0x7A = 0xFF;
	}
		
	/*
	for(int i = 0; i< 16; i++)
	{
    dataOut.data.VPM_adjusted_critical_radius_he[i] = pStateReal->vpm.adjusted_critical_radius_he[i];
    dataOut.data.VPM_adjusted_critical_radius_n2[i] = pStateReal->vpm.adjusted_critical_radius_n2[i];
    dataOut.data.VPM_adjusted_crushing_pressure_he[i] = pStateReal->vpm.adjusted_crushing_pressure_he[i];
    dataOut.data.VPM_adjusted_crushing_pressure_n2[i] = pStateReal->vpm.adjusted_crushing_pressure_n2[i];
    dataOut.data.VPM_initial_allowable_gradient_he[i] = pStateReal->vpm.initial_allowable_gradient_he[i];
    dataOut.data.VPM_initial_allowable_gradient_n2[i] = pStateReal->vpm.initial_allowable_gradient_n2[i];
    dataOut.data.VPM_max_actual_gradient[i] = pStateReal->vpm.max_actual_gradient[i];
	}
*/
	
	if(DataEX_check_header_and_footer_ok() && !told_reset_logik_alles_ok)
	{
		MX_tell_reset_logik_alles_ok();
		told_reset_logik_alles_ok = 1;
	}

	if(DataEX_check_header_and_footer_ok() && (dataIn.power_on_reset == 1))
	{
		if(!wasUpdateNotPowerOn)
			wasPowerOn = 1;

		RTC_DateTypeDef Sdate;
		RTC_TimeTypeDef Stime;

		translateDate(settings->backup_localtime_rtc_dr, &Sdate);
		translateTime(settings->backup_localtime_rtc_tr, &Stime);

		dataOut.data.newTime = Stime;
		dataOut.setTimeNow = 1;
		dataOut.data.newDate = Sdate;
		dataOut.setDateNow = 1;
		
		settingsHelperButtonSens_keepPercentageValues(settingsGetPointerStandard()->ButtonResponsiveness[3], settings->ButtonResponsiveness);
		setButtonResponsiveness(settings->ButtonResponsiveness);
		
		// hw 160720 new lastKnownBatteryPercentage
		if(!wasUpdateNotPowerOn)
		{
//			dataOut.data.newBatteryGaugePercentageFloat = settingsGetPointer()->lastKnownBatteryPercentage;
			dataOut.data.newBatteryGaugePercentageFloat = 0;
			dataOut.setBatteryGaugeNow = 1;
		}
	}
}


void DataEX_copy_to_deco(void)
{
  SDiveState * pStateUsed;
	if(decoLock == DECO_CALC_running)
        return;
	if(stateUsed == stateRealGetPointer())
		pStateUsed = stateRealGetPointerWrite();
	else{
		pStateUsed = stateSimGetPointerWrite();
	}

		if(decoLock == DECO_CALC_init_as_is_start_of_dive)
		{
	    vpm_init(&pStateUsed->vpm,  pStateUsed->diveSettings.vpm_conservatism, 0, 0);
	    buehlmann_init();
	    timer_init();
	    resetEvents();
      pStateUsed->diveSettings.internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero = 0;
			/*
			 * ToDo by Peter
			 * copy VPM stuff etc. pp.
			 * was   void initDiveState(SDiveSettings * pDiveSettings, SVpm * pVpm);
			 */
		}



		if(decoLock == DECO_CALC_FINSHED_Buehlmann)
    {

    }
    switch(decoLock)
    {

    //Deco_calculation finished
    case DECO_CALC_FINSHED_vpm:
        memcpy(&pStateUsed->decolistVPM,&stateDeco.decolistVPM,sizeof(SDecoinfo));
				pStateUsed->decolistVPM.tickstamp = HAL_GetTick();
        pStateUsed->vpm.deco_zone_reached = stateDeco.vpm.deco_zone_reached;
        for(int i = 0; i< 16; i++)
        {
            pStateUsed->vpm.adjusted_critical_radius_he[i] = stateDeco.vpm.adjusted_critical_radius_he[i];
            pStateUsed->vpm.adjusted_critical_radius_n2[i] = stateDeco.vpm.adjusted_critical_radius_n2[i];
            pStateUsed->vpm.adjusted_crushing_pressure_he[i] = stateDeco.vpm.adjusted_crushing_pressure_he[i];
            pStateUsed->vpm.adjusted_crushing_pressure_n2[i] = stateDeco.vpm.adjusted_crushing_pressure_n2[i];
            pStateUsed->vpm.initial_allowable_gradient_he[i] = stateDeco.vpm.initial_allowable_gradient_he[i];
            pStateUsed->vpm.initial_allowable_gradient_n2[i] = stateDeco.vpm.initial_allowable_gradient_n2[i];
            pStateUsed->vpm.max_actual_gradient[i] = stateDeco.vpm.max_actual_gradient[i];
        }
        break;
    case DECO_CALC_FINSHED_Buehlmann:
        memcpy(&pStateUsed->decolistBuehlmann,&stateDeco.decolistBuehlmann,sizeof(SDecoinfo));
				pStateUsed->decolistBuehlmann.tickstamp = HAL_GetTick();
        //Copy Data to be stored if regular Buehlmann, not FutureBuehlmann
        pStateUsed->diveSettings.internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero = stateDeco.diveSettings.internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero;
        break;
    case DECO_CALC_FINSHED_FutureBuehlmann:
        memcpy(&pStateUsed->decolistFutureBuehlmann,&stateDeco.decolistFutureBuehlmann,sizeof(SDecoinfo));
				pStateUsed->decolistFutureBuehlmann.tickstamp = HAL_GetTick();
        break;
    case DECO_CALC_FINSHED_Futurevpm:
        memcpy(&pStateUsed->decolistFutureVPM,&stateDeco.decolistFutureVPM,sizeof(SDecoinfo));
				pStateUsed->decolistFutureVPM.tickstamp = HAL_GetTick();
        break;
    }

    //Copy Inputdata from stateReal to stateDeco
    memcpy(&stateDeco.lifeData,&pStateUsed->lifeData,sizeof(SLifeData));
    memcpy(&stateDeco.diveSettings,&pStateUsed->diveSettings,sizeof(SDiveSettings));

    stateDeco.vpm.deco_zone_reached = pStateUsed->vpm.deco_zone_reached;
    // memcpy(&stateDeco.vpm,&pStateUsed->vpm,sizeof(SVpm));
    for(int i = 0; i< 16; i++)
    {
        stateDeco.vpm.max_crushing_pressure_he[i] = pStateUsed->vpm.max_crushing_pressure_he[i];
        stateDeco.vpm.max_crushing_pressure_n2[i] = pStateUsed->vpm.max_crushing_pressure_n2[i];
        stateDeco.vpm.adjusted_critical_radius_he[i] = pStateUsed->vpm.adjusted_critical_radius_he[i];
        stateDeco.vpm.adjusted_critical_radius_n2[i] = pStateUsed->vpm.adjusted_critical_radius_n2[i];
    }
		decoLock = DECO_CALC_ready;
}


void DataEX_helper_copy_deviceData(SDeviceLine *lineWrite, const SDeviceLine *lineRead)
{	
	lineWrite->date_rtc_dr = lineRead->date_rtc_dr; 
	lineWrite->time_rtc_tr = lineRead->time_rtc_tr;
	lineWrite->value_int32 = lineRead->value_int32;
}



void DataEX_helper_SetTime(RTC_TimeTypeDef inStimestructure, uint32_t *outTimetmpreg)
{
  inStimestructure.TimeFormat = RTC_HOURFORMAT_24;

	*outTimetmpreg = 		(uint32_t)(((uint32_t)RTC_ByteToBcd2(inStimestructure.Hours) << 16U) | \
											((uint32_t)RTC_ByteToBcd2(inStimestructure.Minutes) << 8U) | \
											((uint32_t)RTC_ByteToBcd2(inStimestructure.Seconds)) | \
											(((uint32_t)inStimestructure.TimeFormat) << 16U));  
}


void DataEX_helper_SetDate(RTC_DateTypeDef inSdatestructure, uint32_t *outDatetmpreg)
{
   *outDatetmpreg = (((uint32_t)RTC_ByteToBcd2(inSdatestructure.Year) << 16U) | \
										((uint32_t)RTC_ByteToBcd2(inSdatestructure.Month) << 8U) | \
										((uint32_t)RTC_ByteToBcd2(inSdatestructure.Date)) | \
										((uint32_t)inSdatestructure.WeekDay << 13U));   
}



void DataEX_helper_set_Unknown_Date_deviceData(SDeviceLine *lineWrite)
{	
	RTC_DateTypeDef sdatestructure;
	RTC_TimeTypeDef stimestructure;

	stimestructure.Hours = 1;
	stimestructure.Minutes = 0;
	stimestructure.Seconds = 0;

	sdatestructure.Date = 1;
	sdatestructure.Month = 1;
	sdatestructure.Year = 16;
	setWeekday(&sdatestructure);

	DataEX_helper_SetTime(stimestructure, &lineWrite->time_rtc_tr);
	DataEX_helper_SetDate(sdatestructure, &lineWrite->date_rtc_dr);
}


uint8_t DataEX_helper_Check_And_Correct_Date_deviceData(SDeviceLine *lineWrite)
{
	RTC_DateTypeDef sdatestructure;
	RTC_TimeTypeDef stimestructure;

	// from lineWrite to structure
	translateDate(lineWrite->date_rtc_dr, &sdatestructure);
	translateTime(lineWrite->time_rtc_tr, &stimestructure);
	
	if(		(sdatestructure.Year >= 15)
			&& (sdatestructure.Year <= 30)
			&& (sdatestructure.Month <= 12))
		return 0;


	DataEX_helper_set_Unknown_Date_deviceData(lineWrite);
	return 1;
}


uint8_t DataEX_helper_Check_And_Correct_Value_deviceData(SDeviceLine *lineWrite, int32_t from, int32_t to)
{
	if(lineWrite->value_int32 >= from && lineWrite->value_int32 <= to)
		return 0;

	if(lineWrite->value_int32 < from)
		lineWrite->value_int32 = from;
	else
		lineWrite->value_int32 = to;
		
	DataEX_helper_set_Unknown_Date_deviceData(lineWrite);
	return 0;
}


void DataEX_check_DeviceData(void)
{
	SDevice *DeviceData = stateDeviceGetPointerWrite();

	DataEX_helper_Check_And_Correct_Date_deviceData(&DeviceData->batteryChargeCompleteCycles);
	DataEX_helper_Check_And_Correct_Date_deviceData(&DeviceData->batteryChargeCycles);
	DataEX_helper_Check_And_Correct_Date_deviceData(&DeviceData->depthMaximum);
	DataEX_helper_Check_And_Correct_Date_deviceData(&DeviceData->diveCycles);
	DataEX_helper_Check_And_Correct_Date_deviceData(&DeviceData->hoursOfOperation);
	DataEX_helper_Check_And_Correct_Date_deviceData(&DeviceData->temperatureMaximum);
	DataEX_helper_Check_And_Correct_Date_deviceData(&DeviceData->temperatureMinimum);
	DataEX_helper_Check_And_Correct_Date_deviceData(&DeviceData->voltageMinimum);

	DataEX_helper_Check_And_Correct_Value_deviceData(&DeviceData->batteryChargeCompleteCycles, 0, 10000);
	DataEX_helper_Check_And_Correct_Value_deviceData(&DeviceData->batteryChargeCycles, 0, 20000);
	DataEX_helper_Check_And_Correct_Value_deviceData(&DeviceData->depthMaximum, 0, (500*100)+1000);
	DataEX_helper_Check_And_Correct_Value_deviceData(&DeviceData->diveCycles, 0, 20000);
	DataEX_helper_Check_And_Correct_Value_deviceData(&DeviceData->hoursOfOperation, 0, 1000000);
	DataEX_helper_Check_And_Correct_Value_deviceData(&DeviceData->temperatureMaximum, -30*100, 150*100);
	DataEX_helper_Check_And_Correct_Value_deviceData(&DeviceData->temperatureMinimum, -30*100, 150*100);
	DataEX_helper_Check_And_Correct_Value_deviceData(&DeviceData->voltageMinimum, -1*1000, 6*1000);
}


void DataEX_merge_DeviceData_and_store(void)
{
	uint16_t dataLengthRead;
	SDevice DeviceDataFlash;
	SDevice *DeviceData = stateDeviceGetPointerWrite();

	dataLengthRead = ext_flash_read_devicedata((uint8_t *)&DeviceDataFlash,sizeof(SDevice));

	if(dataLengthRead == 0)
	{
		ext_flash_write_devicedata();
		return;
	}

/*
	SDeviceLine batteryChargeCycles;
	SDeviceLine batteryChargeCompleteCycles;
	SDeviceLine temperatureMinimum;
	SDeviceLine temperatureMaximum;
	SDeviceLine depthMaximum;
	SDeviceLine diveCycles;
	SDeviceLine voltageMinimum;
*/

	/* max values */
	if(DeviceData->batteryChargeCompleteCycles.value_int32 < DeviceDataFlash.batteryChargeCompleteCycles.value_int32)
	{
		DataEX_helper_copy_deviceData(&DeviceData->batteryChargeCompleteCycles, &DeviceDataFlash.batteryChargeCompleteCycles);
	}
	if(DeviceData->batteryChargeCycles.value_int32 < DeviceDataFlash.batteryChargeCycles.value_int32)
	{
		DataEX_helper_copy_deviceData(&DeviceData->batteryChargeCycles, &DeviceDataFlash.batteryChargeCycles);
	}
	if(DeviceData->temperatureMaximum.value_int32 < DeviceDataFlash.temperatureMaximum.value_int32)
	{
		DataEX_helper_copy_deviceData(&DeviceData->temperatureMaximum, &DeviceDataFlash.temperatureMaximum);
	}
	if(DeviceData->depthMaximum.value_int32 < DeviceDataFlash.depthMaximum.value_int32)
	{
		DataEX_helper_copy_deviceData(&DeviceData->depthMaximum, &DeviceDataFlash.depthMaximum);
	}
	if(DeviceData->diveCycles.value_int32 < DeviceDataFlash.diveCycles.value_int32)
	{
		DataEX_helper_copy_deviceData(&DeviceData->diveCycles, &DeviceDataFlash.diveCycles);
	}
	
	/* min values */
	if(DeviceData->temperatureMinimum.value_int32 > DeviceDataFlash.temperatureMinimum.value_int32)
	{
		DataEX_helper_copy_deviceData(&DeviceData->temperatureMinimum, &DeviceDataFlash.temperatureMinimum);
	}
	// Voltage minimum, keep limit to 2.0 Volt; hw 09.09.2015
	if(DeviceData->voltageMinimum.value_int32 > DeviceDataFlash.voltageMinimum.value_int32)
	{
		if(DeviceDataFlash.voltageMinimum.value_int32 > 2000) // do not copy back 2000 and below
			DataEX_helper_copy_deviceData(&DeviceData->voltageMinimum, &DeviceDataFlash.voltageMinimum);
	}
	if(DeviceData->voltageMinimum.value_int32 < 2000)
		DeviceData->voltageMinimum.value_int32 = 2000;
	
	DataEX_check_DeviceData	();
	ext_flash_write_devicedata();
}


void DataEX_copy_to_DeviceData(void)
{
	SDataExchangeSlaveToMasterDeviceData * dataInDevice = (SDataExchangeSlaveToMasterDeviceData *)&dataIn;
	SDevice * pDeviceState = stateDeviceGetPointerWrite();

	memcpy(pDeviceState, &dataInDevice->DeviceData[dataInDevice->boolDeviceData], sizeof(SDevice));
}


void DataEX_copy_to_VpmRepetitiveData(void)
{
	SDataExchangeSlaveToMasterDeviceData * dataInDevice = (SDataExchangeSlaveToMasterDeviceData *)&dataIn;
	SVpmRepetitiveData * pVpmState = stateVpmRepetitiveDataGetPointerWrite();

	if(dataInDevice->boolVpmRepetitiveDataValid)
	{
		memcpy(pVpmState, &dataInDevice->VpmRepetitiveData, sizeof(SVpmRepetitiveData));
		pVpmState->is_data_from_RTE_CPU = 1;
	}
}


void DataEX_control_connection_while_asking_for_sleep(void)
{
 	if(!DataEX_check_header_and_footer_ok())
	{
		if(DataEX_check_header_and_footer_devicedata())
		{
			data_old__lost_connection_to_slave_counter_retry = 0;
			data_old__lost_connection_to_slave_counter_temp = 0;
			stateRealGetPointerWrite()->data_old__lost_connection_to_slave = 0;
		}
		else
		{
			stateRealGetPointerWrite()->data_old__lost_connection_to_slave = 1;
			data_old__lost_connection_to_slave_counter_temp += 1;
			data_old__lost_connection_to_slave_counter_total += 1;
		}
	}
}


void DataEX_copy_to_LifeData(_Bool *modeChangeFlag)
{
	SDiveState * pStateReal = stateRealGetPointerWrite();
	static uint16_t getDeviceDataAfterStartOfMainCPU = 20;
	
	/* internal sensor: HUD data
	 */
	for(int i=0;i<3;i++)
	{
		pStateReal->lifeData.ppO2Sensor_bar[i] = get_ppO2Sensor_bar(i);
		pStateReal->lifeData.sensorVoltage_mV[i] = get_sensorVoltage_mV(i);
	}
	pStateReal->lifeData.HUD_battery_voltage_V = get_HUD_battery_voltage_V();

	
	// wireless - �ltere daten aufr�umen
	for(int i=0;i<(2*NUM_GASES+1);i++)
	{
		if(pStateReal->lifeData.bottle_bar[i])
		{
			if((pStateReal->lifeData.bottle_bar_age_MilliSeconds[i] == 0) || (pStateReal->lifeData.bottle_bar_age_MilliSeconds[i] > 60000))
			{
				pStateReal->lifeData.bottle_bar_age_MilliSeconds[i] =  0;
				pStateReal->lifeData.bottle_bar[i] = 0;
			}
			else
				pStateReal->lifeData.bottle_bar_age_MilliSeconds[i] +=  100;
		}
	}
	
/* Why? hw 8.6.2015
	if(DataEX_check_header_and_footer_ok() && dataIn.power_on_reset)
	{
		return;
	}
*/
	if(!DataEX_check_header_and_footer_ok())
	{
		if(DataEX_check_header_and_footer_devicedata())
		{
			DataEX_copy_to_DeviceData();
			DataEX_merge_DeviceData_and_store();
			DataEX_copy_to_VpmRepetitiveData();
			data_old__lost_connection_to_slave_counter_temp = 0;
			data_old__lost_connection_to_slave_counter_retry = 0;
			pStateReal->data_old__lost_connection_to_slave = 0;
		}
		else
		{
			pStateReal->data_old__lost_connection_to_slave = 1;
			data_old__lost_connection_to_slave_counter_temp += 1;
			data_old__lost_connection_to_slave_counter_total += 1;
		}
		return;
	}
	
	if(getDeviceDataAfterStartOfMainCPU)
	{
		getDeviceDataAfterStartOfMainCPU--;
		if(getDeviceDataAfterStartOfMainCPU == 0)
		{
			dataOut.getDeviceDataNow = 1;
			getDeviceDataAfterStartOfMainCPU = 10*60*10;// * 100ms
		}
	}

	/* new 151207 hw */
	if(requestNecessary.uw != 0)
	{
		if(((dataIn.confirmRequest.uw) & CRBUTTON) != 0)
		{
			requestNecessary.ub.button = 0;
		}

		if(requestNecessary.ub.button == 1)
		{
			setButtonResponsiveness(settingsGetPointer()->ButtonResponsiveness);
		}
/*
	}
		if((dataIn.confirmRequest.ub.clearDeco != 1) && (requestNecessary.ub.clearDeco == 1))
		{
			clearDeco(); // is dataOut.clearDecoNow = 1;
		}
*/		
	}
	requestNecessary.uw = 0; // clear all 
	
	float ambient, surface, density, meter;
	SSettings *pSettings;
	
	/*	uint8_t IAmStolenPleaseKillMe;
	 */
	pSettings = settingsGetPointer();

	if(pSettings->IAmStolenPleaseKillMe > 3)
	{
		pSettings->salinity = 0;
		dataIn.data[dataIn.boolPressureData].surface_mbar = 999;
		dataIn.data[dataIn.boolPressureData].pressure_mbar = 98971;
		dataIn.mode = MODE_DIVE;
	}


	ambient = dataIn.data[dataIn.boolPressureData].pressure_mbar / 1000.0f;
	surface = dataIn.data[dataIn.boolPressureData].surface_mbar / 1000.0f;

	density = ((float)( 100 + pSettings->salinity)) / 100.0f;
	meter = (ambient - surface);
	meter /= (0.09807f * density);


	pStateReal->pressure_uTick_old = pStateReal->pressure_uTick_new;
	pStateReal->pressure_uTick_new = dataIn.data[dataIn.boolPressureData].pressure_uTick;
	pStateReal->pressure_uTick_local_new = HAL_GetTick();
	
	if(ambient < (surface + 0.04f))

	pStateReal->lifeData.dateBinaryFormat = dataIn.data[dataIn.boolTimeData].localtime_rtc_dr;
	pStateReal->lifeData.timeBinaryFormat = dataIn.data[dataIn.boolTimeData].localtime_rtc_tr;

	dataOut.setAccidentFlag = 0;

	//Start of diveMode?
	if(pStateReal->mode != MODE_DIVE && dataIn.mode == MODE_DIVE)
	{
		if(modeChangeFlag)
			*modeChangeFlag = 1;
	  if(stateUsed == stateSimGetPointer())
			{
				simulation_exit();
			}
			// new 170508
			settingsGetPointer()->bluetoothActive = 0;
			MX_Bluetooth_PowerOff();
			//Init dive Mode
		decoLock = DECO_CALC_init_as_is_start_of_dive;
		pStateReal->lifeData.boolResetAverageDepth = 1;
		pStateReal->lifeData.boolResetStopwatch = 1;
	}
	
	//End of diveMode?
	if(pStateReal->mode == MODE_DIVE && dataIn.mode != MODE_DIVE)
	{
		if(modeChangeFlag)
			*modeChangeFlag = 1;
		createDiveSettings();

		if(pStateReal->warnings.cnsHigh)
		{
			if(pStateReal->lifeData.cns >= 130)
				dataOut.setAccidentFlag += ACCIDENT_CNSLVL2;
			else if(pStateReal->lifeData.cns >= 100)
				dataOut.setAccidentFlag += ACCIDENT_CNS;
		}
		if(pStateReal->warnings.decoMissed)
			dataOut.setAccidentFlag += ACCIDENT_DECOSTOP;
	}
	pStateReal->mode = dataIn.mode;
	pStateReal->chargeStatus = dataIn.chargeStatus;

	pStateReal->lifeData.pressure_ambient_bar = ambient;
	pStateReal->lifeData.pressure_surface_bar = surface;
	if(is_ambient_pressure_close_to_surface(&pStateReal->lifeData))
	{
		pStateReal->lifeData.depth_meter = 0;
	}
	else
	{
		pStateReal->lifeData.depth_meter = meter;
	}
	pStateReal->lifeData.temperature_celsius = dataIn.data[dataIn.boolPressureData].temperature;
	pStateReal->lifeData.ascent_rate_meter_per_min = dataIn.data[dataIn.boolPressureData].ascent_rate_meter_per_min;
	if(pStateReal->mode != MODE_DIVE)
		pStateReal->lifeData.max_depth_meter = 0;
	else
  {
    if(meter > pStateReal->lifeData.max_depth_meter)
      pStateReal->lifeData.max_depth_meter = meter;
  }

	if(dataIn.accidentFlags & ACCIDENT_DECOSTOP)
		pStateReal->decoMissed_at_the_end_of_dive = 1;
	if(dataIn.accidentFlags & ACCIDENT_CNS)
		pStateReal->cnsHigh_at_the_end_of_dive = 1;
	
	pStateReal->lifeData.dive_time_seconds = (int32_t)dataIn.data[dataIn.boolTimeData].divetime_seconds;
	pStateReal->lifeData.dive_time_seconds_without_surface_time = (int32_t)dataIn.data[dataIn.boolTimeData].dive_time_seconds_without_surface_time;
	pStateReal->lifeData.counterSecondsShallowDepth = dataIn.data[dataIn.boolTimeData].counterSecondsShallowDepth;
	pStateReal->lifeData.surface_time_seconds = (int32_t)dataIn.data[dataIn.boolTimeData].surfacetime_seconds;


	pStateReal->lifeData.compass_heading = dataIn.data[dataIn.boolCompassData].compass_heading;
	if(settingsGetPointer()->FlipDisplay) /* consider that diver is targeting into the opposite direction */
	{
		pStateReal->lifeData.compass_heading -= 180.0;
		if (pStateReal->lifeData.compass_heading < 0) pStateReal->lifeData.compass_heading +=360.0;
	}


	pStateReal->lifeData.compass_roll = dataIn.data[dataIn.boolCompassData].compass_roll;
	pStateReal->lifeData.compass_pitch = dataIn.data[dataIn.boolCompassData].compass_pitch;

	pStateReal->lifeData.compass_DX_f = dataIn.data[dataIn.boolCompassData].compass_DX_f;
	pStateReal->lifeData.compass_DY_f = dataIn.data[dataIn.boolCompassData].compass_DY_f;
	pStateReal->lifeData.compass_DZ_f = dataIn.data[dataIn.boolCompassData].compass_DZ_f;

	pStateReal->compass_uTick_old = pStateReal->compass_uTick_new;
	pStateReal->compass_uTick_new = dataIn.data[dataIn.boolCompassData].compass_uTick;
	pStateReal->compass_uTick_local_new = HAL_GetTick();
	
  pStateReal->lifeData.cns = dataIn.data[dataIn.boolToxicData].cns;
	pStateReal->lifeData.otu = dataIn.data[dataIn.boolToxicData].otu;
  pStateReal->lifeData.no_fly_time_minutes = dataIn.data[dataIn.boolToxicData].no_fly_time_minutes;
	pStateReal->lifeData.desaturation_time_minutes = dataIn.data[dataIn.boolToxicData].desaturation_time_minutes;

	memcpy(pStateReal->lifeData.tissue_nitrogen_bar, dataIn.data[dataIn.boolTisssueData].tissue_nitrogen_bar,sizeof(pStateReal->lifeData.tissue_nitrogen_bar));
	memcpy(pStateReal->lifeData.tissue_helium_bar, dataIn.data[dataIn.boolTisssueData].tissue_helium_bar,sizeof(pStateReal->lifeData.tissue_helium_bar));

	if(pStateReal->mode == MODE_DIVE)
	{
		for(int i= 0; i <16; i++)
		{
			pStateReal->vpm.max_crushing_pressure_he[i] =  dataIn.data[dataIn.boolCrushingData].max_crushing_pressure_he[i];
			pStateReal->vpm.max_crushing_pressure_n2[i] = dataIn.data[dataIn.boolCrushingData].max_crushing_pressure_n2[i];
			pStateReal->vpm.adjusted_critical_radius_he[i] =  dataIn.data[dataIn.boolCrushingData].adjusted_critical_radius_he[i];
			pStateReal->vpm.adjusted_critical_radius_n2[i] = dataIn.data[dataIn.boolCrushingData].adjusted_critical_radius_n2[i];
		}
	}

	/* battery and ambient light sensors
	 */
	pStateReal->lifeData.ambient_light_level = dataIn.data[dataIn.boolAmbientLightData].ambient_light_level;
	pStateReal->lifeData.battery_charge = dataIn.data[dataIn.boolBatteryData].battery_charge;
	pStateReal->lifeData.battery_voltage = dataIn.data[dataIn.boolBatteryData].battery_voltage;

/* now in ext_flash_write_settings() // hw 161027
 *	if((pStateReal->lifeData.battery_charge > 1) && !DataEX_was_power_on() && ((uint8_t)(pStateReal->lifeData.battery_charge) !=  0x10)) // get rid of 16% (0x10)
 *		pSettings->lastKnownBatteryPercentage = (uint8_t)(pStateReal->lifeData.battery_charge);
 */
	
	/* OC and CCR but no sensors -> moved to updateSetpointStateUsed();
	float oxygen = 0;
	if(pStateReal->diveSettings.diveMode == 0)
	{
		oxygen = 1.00f;
		oxygen -= ((float)pStateReal->lifeData.actualGas.nitrogen_percentage)/100.0f;
		oxygen -= ((float)pStateReal->lifeData.actualGas.helium_percentage)/100.0f;
		pStateReal->lifeData.ppO2 = pStateReal->lifeData.pressure_ambient_bar * oxygen;
	}
  else if(pStateReal->diveSettings.diveMode == 1)
	{
	  pStateReal->lifeData.ppO2 = ((float)pStateReal->lifeData.actualGas.setPoint_cbar) /100;
	}
	 */

	/* apnea specials
	 */
	if(pStateReal->diveSettings.diveMode == DIVEMODE_Apnea)
	{
		if(pStateReal->mode != MODE_DIVE)
		{
			pStateReal->lifeData.apnea_total_max_depth_meter = 0;
			pStateReal->lifeData.apnea_last_dive_time_seconds = 0;
			pStateReal->lifeData.apnea_last_max_depth_meter = 0;
		}
		else
		{
			if(pStateReal->lifeData.max_depth_meter > pStateReal->lifeData.apnea_total_max_depth_meter)
				pStateReal->lifeData.apnea_total_max_depth_meter = pStateReal->lifeData.max_depth_meter;
		}

		if(pStateReal->lifeData.dive_time_seconds > 15)
		{
			pStateReal->lifeData.apnea_last_dive_time_seconds = pStateReal->lifeData.dive_time_seconds;
		}

		if(pStateReal->lifeData.counterSecondsShallowDepth)
		{
			if(pStateReal->lifeData.max_depth_meter > 1.5f)
			{
				pStateReal->lifeData.apnea_last_max_depth_meter = pStateReal->lifeData.max_depth_meter;
			}
		// eset max_depth_meter, average_depth_meter and internal values
			pStateReal->lifeData.max_depth_meter = 0;
			pStateReal->lifeData.boolResetAverageDepth = 1;
			pStateReal->lifeData.boolResetStopwatch = 1;
		}
	}

	/* average depth 
	 */
	float *AvgDepthValue = &pStateReal->lifeData.average_depth_meter;
	float	DepthNow = pStateReal->lifeData.depth_meter; 
	uint32_t *AvgDepthCount = &pStateReal->lifeData.internal.average_depth_meter_Count;
	uint32_t *AvgDepthTimer = &pStateReal->lifeData.internal.average_depth_last_update_dive_time_seconds_without_surface_time;
	uint32_t AvgSecondsSinceLast;
	uint32_t DiveTime = pStateReal->lifeData.dive_time_seconds_without_surface_time;
	
	if(pStateReal->lifeData.boolResetAverageDepth)
	{
		*AvgDepthValue = DepthNow;
		*AvgDepthCount = 1;
		*AvgDepthTimer = DiveTime;
		pStateReal->lifeData.boolResetAverageDepth = 0;
	}
	else if (DiveTime > *AvgDepthTimer)
	{
		AvgSecondsSinceLast = DiveTime - *AvgDepthTimer;
		for(int i=0;i<AvgSecondsSinceLast;i++)
		{
			*AvgDepthValue = (*AvgDepthValue * *AvgDepthCount + DepthNow) / (*AvgDepthCount + 1);
			*AvgDepthCount += 1;
		}
		*AvgDepthTimer = DiveTime;
	}
	if(*AvgDepthCount == 0)
		*AvgDepthValue = 0;


	/* stop watch
	 */
	if(pStateReal->lifeData.boolResetStopwatch)
	{
		pStateReal->lifeData.internal.stopwatch_start_at_this_dive_time_seconds = pStateReal->lifeData.dive_time_seconds;
		pStateReal->lifeData.boolResetStopwatch = 0;
	}
	pStateReal->lifeData.stopwatch_seconds = pStateReal->lifeData.dive_time_seconds - pStateReal->lifeData.internal.stopwatch_start_at_this_dive_time_seconds;

	/* wireless data
	 */
	uint16_t wirelessData[4][3];
	for(int i=0;i<4;i++)
	{
		pStateReal->lifeData.wireless_data[i].ageInMilliSeconds = dataIn.data[dataIn.boolWirelessData].wireless_data[i].ageInMilliSeconds;	
		pStateReal->lifeData.wireless_data[i].status = dataIn.data[dataIn.boolWirelessData].wireless_data[i].status;	
		pStateReal->lifeData.wireless_data[i].numberOfBytes = dataIn.data[dataIn.boolWirelessData].wireless_data[i].numberOfBytes;	
		for(int j=0;j<12;j++)
			pStateReal->lifeData.wireless_data[i].data[j] = dataIn.data[dataIn.boolWirelessData].wireless_data[i].data[j];
	}

	/* old stuff
	// crc - is done in RTE 160325
	// size at the moment 4 bytes + one empty + crc -> minimum 5 bytes (+ crc)
	// kopieren: Id, Wert, Alter
	for(int i=0;i<4;i++)
	{
		uint8_t numberOfBytes = pStateReal->lifeData.wireless_data[i].numberOfBytes - 1;
		
		if((numberOfBytes < 5) || (numberOfBytes > 7))
		{
			wirelessData[i][0] = 0;
			wirelessData[i][1] = 0;
			wirelessData[i][2] = 0;
		}
		else
		{
			if((crc32c_checksum(pStateReal->lifeData.wireless_data[i].data, numberOfBytes, 0, 0) & 0xFF)!= pStateReal->lifeData.wireless_data[i].data[numberOfBytes])
			{
// no crc is send at the moment
wirelessData[i][0] = (pStateReal->lifeData.wireless_data[i].data[0] * 256) + pStateReal->lifeData.wireless_data[i].data[1];
wirelessData[i][1] = (pStateReal->lifeData.wireless_data[i].data[3] * 256) + pStateReal->lifeData.wireless_data[i].data[4];
wirelessData[i][2] = pStateReal->lifeData.wireless_data[i].ageInMilliSeconds;

//				wirelessData[i][0] = 0;
//				wirelessData[i][1] = 0;
//				wirelessData[i][2] = 0;
				
			}

			else
			{
				wirelessData[i][0] = (pStateReal->lifeData.wireless_data[i].data[0] * 256) + pStateReal->lifeData.wireless_data[i].data[1];
				wirelessData[i][1] = (pStateReal->lifeData.wireless_data[i].data[3] * 256) + pStateReal->lifeData.wireless_data[i].data[4];
				wirelessData[i][2] = pStateReal->lifeData.wireless_data[i].ageInMilliSeconds;
			}
		}
	}	
*/
	// neu 160412
	for(int i=0;i<4;i++)
	{
		if(pStateReal->lifeData.wireless_data[i].numberOfBytes == 10)
		{
			wirelessData[i][0] = (pStateReal->lifeData.wireless_data[i].data[0] >> 4) & 0x7F;
			wirelessData[i][1] = 0;
			wirelessData[i][2] = pStateReal->lifeData.wireless_data[i].ageInMilliSeconds;
		}
		else
		{
			wirelessData[i][0] = 0;
			wirelessData[i][1] = 0;
			wirelessData[i][2] = 0;
		}
	}

	// aussortieren doppelte ids, j�ngster datensatz ist relevant
	for(int i=0;i<3;i++)
	{
		if(wirelessData[i][0])
		{
			for(int j=i+1; j<4; j++)
			{
				if(wirelessData[i][0] == wirelessData[j][0])
				{
					if(wirelessData[i][2] > wirelessData[j][2])
					{
						wirelessData[i][0] = wirelessData[j][0];
						wirelessData[i][1] = wirelessData[j][1];
						wirelessData[i][2] = wirelessData[j][2];
					}
					wirelessData[j][0] = 0;
					wirelessData[j][1] = 0;
					wirelessData[j][2] = 0;
				}
			}
		}
	}
/*
	// neu 160325
	for(int i=0;i<4;i++)
	{
		if(pStateReal->lifeData.wireless_data[i].numberOfBytes == 10)
		{
			wirelessData[i][0] = (pStateReal->lifeData.wireless_data[i].data[0] * 256) + pStateReal->lifeData.wireless_data[i].data[1];
			wirelessData[i][1] = (pStateReal->lifeData.wireless_data[i].data[3] * 256) + pStateReal->lifeData.wireless_data[i].data[4];
			wirelessData[i][2] = pStateReal->lifeData.wireless_data[i].ageInMilliSeconds;
		}
		else
		{
			wirelessData[i][0] = 0;
			wirelessData[i][1] = 0;
			wirelessData[i][2] = 0;
		}
	}

	// aussortieren doppelte ids, j�ngster datensatz ist relevant
	for(int i=0;i<3;i++)
	{
		if(wirelessData[i][0])
		{
			for(int j=i+1; j<4; j++)
			{
				if(wirelessData[i][0] == wirelessData[j][0])
				{
					if(wirelessData[i][2] > wirelessData[j][2])
					{
						wirelessData[i][0] = wirelessData[j][0];
						wirelessData[i][1] = wirelessData[j][1];
						wirelessData[i][2] = wirelessData[j][2];
					}
					wirelessData[j][0] = 0;
					wirelessData[j][1] = 0;
					wirelessData[j][2] = 0;
				}
			}
		}
	}
*/
/* old 
	// copy to lifeData
	for(int i=0;i<4;i++)
	{
		if((wirelessData[i][0]) && (wirelessData[i][2]) && (wirelessData[i][2] < 60000))
		{
			for(int j=1;j<=(2*NUM_GASES+1);j++)
			{
				if(pStateReal->diveSettings.gas[j].bottle_wireless_id == wirelessData[i][0])
				{
					pStateReal->lifeData.bottle_bar[j] = wirelessData[i][1];
					pStateReal->lifeData.bottle_bar_age_MilliSeconds[j] = wirelessData[i][2];
					break;
				}
			}
		}
	}
*/
	// new: Bonex
	float scooterSpeedFloat;
	int32_t scooterRemainingBattCapacity;
	
	for(int i=0;i<4;i++)
	{
		if((wirelessData[i][0]))// && (wirelessData[i][2]) && (wirelessData[i][2] < 60000))
		{
			pStateReal->lifeData.scooterType							= (pStateReal->lifeData.wireless_data[i].data[0] >> 4) & 0x07;
			pStateReal->lifeData.scooterWattstunden 				= ((uint16_t)((((uint16_t)(pStateReal->lifeData.wireless_data[i].data[0] & 0x0F) << 8) | (pStateReal->lifeData.wireless_data[i].data[1]))));
//			pStateReal->lifeData.scooterWattstunden 		=  pStateReal->lifeData.wireless_data[i].data[0] & 0x0F;
//			pStateReal->lifeData.scooterWattstunden			*= 256;
//			pStateReal->lifeData.scooterWattstunden 		+=  pStateReal->lifeData.wireless_data[i].data[1];
			pStateReal->lifeData.scooterRestkapazitaet 		= pStateReal->lifeData.wireless_data[i].data[2];
			pStateReal->lifeData.scooterDrehzahl 					= ((uint16_t)( (int16_t)((pStateReal->lifeData.wireless_data[i].data[4] << 8) | (pStateReal->lifeData.wireless_data[i].data[3]))));
			pStateReal->lifeData.scooterSpannung					= ((float)(pStateReal->lifeData.wireless_data[i].data[5])) / 5.0f;
			pStateReal->lifeData.scooterTemperature 			= ((uint16_t)( (int16_t)((pStateReal->lifeData.wireless_data[i].data[7] << 8) | (pStateReal->lifeData.wireless_data[i].data[6]))));
			pStateReal->lifeData.scooterAmpere				 		= pStateReal->lifeData.wireless_data[i].data[9] >> 1;
			pStateReal->lifeData.scooterAgeInMilliSeconds = pStateReal->lifeData.wireless_data[i].ageInMilliSeconds;

			if(pStateReal->lifeData.scooterWattstunden > 0)
				scooterRemainingBattCapacity = settingsGetPointer()->scooterBattSize / pStateReal->lifeData.scooterWattstunden;
			else
			scooterRemainingBattCapacity = 100;
			
			
			if(scooterRemainingBattCapacity < 0)
				scooterRemainingBattCapacity = 0;
			if(scooterRemainingBattCapacity > 100)
				scooterRemainingBattCapacity = 100;
			pStateReal->lifeData.scooterRestkapazitaetWhBased = scooterRemainingBattCapacity;

//			BONEX_calc_new_ResidualCapacity(&pStateReal->lifeData.scooterRestkapazitaetVoltageBased, (uint32_t)(1000 * pStateReal->lifeData.scooterSpannung),1000,1);
			pStateReal->lifeData.scooterRestkapazitaetVoltageBased = BONEX_mini_ResidualCapacityVoltageBased(pStateReal->lifeData.scooterSpannung, pStateReal->lifeData.scooterAgeInMilliSeconds);	

			scooterSpeedFloat = (float)pStateReal->lifeData.scooterDrehzahl;
			scooterSpeedFloat /= (37.0f / 1.1f); // 3700 rpm = 110 m/min
			switch(settingsGetPointer()->scooterDrag)
			{
				case 1:
					scooterSpeedFloat *= 0.95f;
					break;
				case 2:
					scooterSpeedFloat *= 0.85f;
					break;
				case 3:
					scooterSpeedFloat *= 0.75f;
					break;
				default:
					break;
			}
			switch(settingsGetPointer()->scooterLoad)
			{
				case 1:
					scooterSpeedFloat *= 0.90f;
					break;
				case 2:
					scooterSpeedFloat *= 0.80f;
					break;
				case 3:
					scooterSpeedFloat *= 0.70f;
					break;
				case 4:
					scooterSpeedFloat *= 0.60f;
					break;
				default:
					break;
			}
			if(scooterSpeedFloat < 0)
				pStateReal->lifeData.scooterSpeed = 0;
			else
			if(scooterSpeedFloat > 255)
				pStateReal->lifeData.scooterSpeed = 255;
			else
				pStateReal->lifeData.scooterSpeed = (uint16_t)scooterSpeedFloat;
			
			if(!scooterFoundThisPowerOnCylce && (pStateReal->lifeData.scooterAgeInMilliSeconds > 0))
				scooterFoundThisPowerOnCylce = 1;
		}
	}
 
	/* PIC data
 	 */
	for(int i=0;i<4;i++)
	{
		pStateReal->lifeData.buttonPICdata[i] = dataIn.data[dataIn.boolPICdata].button_setting[i];
	}

	/* sensorErrors
	 */
	pStateReal->sensorErrorsRTE = dataIn.sensorErrors;
	
	/* end
	 */
	data_old__lost_connection_to_slave_counter_temp = 0;
	data_old__lost_connection_to_slave_counter_retry = 0;
	pStateReal->data_old__lost_connection_to_slave = 0;
}


uint8_t DataEX_check_RTE_version__needs_update(void)
{
	if(data_old__lost_connection_to_slave_counter_retry > 10)
		return 1;
	else
	{
		if(stateRealGetPointer()->data_old__lost_connection_to_slave == 0)
		{
			setActualRTEversion(dataIn.RTE_VERSION_high, dataIn.RTE_VERSION_low);
			
			if(RTEminimum_required_high() < dataIn.RTE_VERSION_high)
				return 0;
			else
			if((RTEminimum_required_high() == dataIn.RTE_VERSION_high) && (RTEminimum_required_low() <= dataIn.RTE_VERSION_low))
				return 0;
			else
				return 1;
		}
		else
			return 0;
	}
}


uint8_t DataEX_scooterDataFound(void)
{
	return scooterFoundThisPowerOnCylce;
}


uint8_t DataEX_scooterFoundAndValidLicence(void)
{
	if(getLicence() != LICENCEBONEX)
		return 0;
	else
		return scooterFoundThisPowerOnCylce;
//return 0xFF;
//return LICENCEBONEX;
}

	/* Private functions ---------------------------------------------------------*/

uint8_t DataEX_check_header_and_footer_ok(void)
{
	if(dataIn.header.checkCode[0] != 0xA1)
		return 0;
	if(dataIn.header.checkCode[1] != 0xA2)
		return 0;
	if(dataIn.header.checkCode[2] != 0xA3)
		return 0;
	if(dataIn.header.checkCode[3] != 0xA4)
		return 0;
	if(dataIn.footer.checkCode[0] != 0xE1)
		return 0;
	if(dataIn.footer.checkCode[1] != 0xE2)
		return 0;
	if(dataIn.footer.checkCode[2] != 0xE3)
		return 0;
	if(dataIn.footer.checkCode[3] != 0xE4)
		return 0;

	return 1;
}

uint8_t DataEX_check_header_and_footer_devicedata(void)
{
	if(dataIn.header.checkCode[0] != 0xDF)
		return 0;
	if(dataIn.header.checkCode[1] != 0xDE)
		return 0;
	if(dataIn.header.checkCode[2] != 0xDD)
		return 0;
	if(dataIn.header.checkCode[3] != 0xDC)
		return 0;
	if(dataIn.footer.checkCode[0] != 0xE1)
		return 0;
	if(dataIn.footer.checkCode[1] != 0xE2)
		return 0;
	if(dataIn.footer.checkCode[2] != 0xE3)
		return 0;
	if(dataIn.footer.checkCode[3] != 0xE4)
		return 0;

	return 1;
}



