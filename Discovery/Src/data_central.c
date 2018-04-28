/**
  ******************************************************************************
	* @copyright heinrichs weikamp
  * @file   		data_central.c
  * @author 		heinrichs weikamp gmbh
  * @date   		10-November-2014
  * @version		V1.0.2
  * @since			10-Nov-2014
  * @brief			All the data EXCEPT
  *							 - settings (settings.c)
	*									feste Werte, die nur an der Oberfl�che ge�ndert werden
	*							 - dataIn and dataOut (data_exchange.h and data_exchange_main.c)
	*									Austausch mit Small CPU
	* @bug
	* @warning
  @verbatim
  ==============================================================================
              ##### SDiveState Real and Sim #####
  ==============================================================================
  [..] SDiveSettings
				copy of parts of Settings that are necessary during the dive
				and could be modified during the dive without post dive changes.

  [..] SLifeData
				written in DataEX_copy_to_LifeData();
				block 1 "lifedata" set by SmallCPU in stateReal
				block 2 "actualGas" set by main CPU from user input and send to Small CPU
				block 3 "calculated data" set by main CPU based on "lifedata"

  [..] SVpm

	[..] SEvents

  [..] SDecoinfo

  [..] mode
				set by SmallCPU in stateReal, can be surface, dive, ...

  [..] data_old__lost_connection_to_slave
				set by DataEX_copy_to_LifeData();

  ==============================================================================
              ##### SDiveState Deco #####
  ==============================================================================
  [..] kjbkldafj�lasdfjasdf

  ==============================================================================
              ##### decoLock #####
  ==============================================================================
  [..] The handler that synchronizes the data between IRQ copy and main deco loop


	 @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "data_central.h"
#include "calc_crush.h"
#include "decom.h"
#include "stm32f4xx_hal.h"
#include "settings.h"
#include "data_exchange_main.h"
#include "ostc.h" // for button adjust on hw testboard 1
#include "tCCR.h"
#include "crcmodel.h"

SDiveState stateReal = { 0 };
SDiveState stateSim = { 0 };
SDiveState stateDeco = { 0 };

SLifeData2 secondaryInformation  = { 0 };

SDevice stateDevice =
{
	/* max is 0x7FFFFFFF, min is 0x80000000 but also defined in stdint.h :-) */

	/* count, use 0 */
	.batteryChargeCompleteCycles.value_int32 = 0,
	.batteryChargeCycles.value_int32 = 0,
	.diveCycles.value_int32 = 0,
	.hoursOfOperation.value_int32 = 0,

	/* max values, use min. */
	.temperatureMaximum.value_int32 = INT32_MIN,
	.depthMaximum.value_int32 = INT32_MIN,

	/* min values, use max. */
	.temperatureMinimum.value_int32 = INT32_MAX,
	.voltageMinimum.value_int32 = INT32_MAX,
};

SVpmRepetitiveData stateVPM =
{
	.repetitive_variables_not_valid = 1,
	.is_data_from_RTE_CPU = 0,
};

const SDiveState * stateUsed = &stateReal;


void set_stateUsedToReal(void)
{
	stateUsed = &stateReal;
}

void set_stateUsedToSim(void)
{
	stateUsed = &stateSim;
}

_Bool is_stateUsedSetToSim(void)
{
	if(stateUsed == &stateSim)
		return 1;
	else
		return 0;

}

const SDiveState * stateRealGetPointer(void)
{
	return &stateReal;
}

SDiveState * stateRealGetPointerWrite(void)
{
	return &stateReal;
}


const SDiveState * stateSimGetPointer(void)
{
	return &stateSim;
}


SDiveState * stateSimGetPointerWrite(void)
{
	return &stateSim;
}


const SDevice * stateDeviceGetPointer(void)
{
	return &stateDevice;
}


SDevice * stateDeviceGetPointerWrite(void)
{
	return &stateDevice;
}


const SVpmRepetitiveData * stateVpmRepetitiveDataGetPointer(void)
{
	return &stateVPM;
}


SVpmRepetitiveData * stateVpmRepetitiveDataGetPointerWrite(void)
{
	return &stateVPM;
}


uint32_t time_elapsed_ms(uint32_t ticksstart,uint32_t ticksnow)
{
	if(ticksstart <= ticksnow)
		return ticksnow - ticksstart;
	else
		return 0xFFFFFFFF - ticksstart + ticksnow;
}


uint8_t decoLock = DECO_CALC_undefined;
int ascent_rate_meter_per_min  = 12;
int descent_rate_meter_per_min  = 20;
int max_depth = 70;
int bottom_time = 10;

_Bool vpm_crush(SDiveState* pDiveState);
void setSimulationValues(int _ascent_rate_meter_per_min, int _descent_rate_meter_per_min, int _max_depth, int _bottom_time )
{
    ascent_rate_meter_per_min = _ascent_rate_meter_per_min;
    descent_rate_meter_per_min = _descent_rate_meter_per_min;
    max_depth = _max_depth;
    bottom_time = _bottom_time;
}



int current_second(void) {

    return HAL_GetTick() / 1000;
    // printf("milliseconds: %lld\n", milliseconds);
    //return milliseconds;
}



#define OXY_ONE_SIXTIETH_PART 			0.0166667f

/*void oxygen_calculate_cns(float* oxygen_cns, float pressure_oxygen_real)
{
	int cns_no_range = 0;
	_Bool not_found = 1;
    //for the cns calculation
    const float cns_ppo2_ranges[60][2] = {	{0.50, 0.00}, {0.60, 0.14}, {0.64, 0.15}, {0.66, 0.16}, {0.68, 0.17}, {0.70, 0.18},
										{0.74, 0.19}, {0.76, 0.20}, {0.78, 0.21}, {0.80, 0.22}, {0.82, 0.23}, {0.84, 0.24},
										{0.86, 0.25}, {0.88, 0.26}, {0.90, 0.28}, {0.92, 0.29}, {0.94, 0.30}, {0.96, 0.31},
										{0.98, 0.32}, {1.00, 0.33}, {1.02, 0.35}, {1.04, 0.36}, {1.06, 0.38}, {1.08, 0.40},
										{1.10, 0.42}, {1.12, 0.43}, {1.14, 0.43}, {1.16, 0.44}, {1.18, 0.46}, {1.20, 0.47},
										{1.22, 0.48}, {1.24, 0.51},	{1.26, 0.52}, {1.28, 0.54}, {1.30, 0.56}, {1.32, 0.57},
										{1.34, 0.60}, {1.36, 0.62}, {1.38, 0.63}, {1.40, 0.65}, {1.42, 0.68}, {1.44, 0.71},
										{1.46, 0.74}, {1.48, 0.78}, {1.50, 0.83}, {1.52, 0.93}, {1.54, 1.04}, {1.56, 1.19},
										{1.58, 1.47}, {1.60, 2.22}, {1.62, 5.00}, {1.65, 6.25}, {1.67, 7.69}, {1.70, 10.0},
										{1.72,12.50}, {1.74,20.00}, {1.77,25.00}, {1.79,31.25}, {1.80,50.00}, {1.82,100.0}};
	//find the correct cns range for the corresponding ppo2
	cns_no_range = 58;
	while (cns_no_range && not_found)
	{
		if (pressure_oxygen_real > cns_ppo2_ranges[cns_no_range][0])
		{
			cns_no_range++;
			not_found = 0;
		}
		else
			cns_no_range--;
	}

	//calculate cns for the actual ppo2 for 1 second
	*oxygen_cns += OXY_ONE_SIXTIETH_PART * cns_ppo2_ranges[cns_no_range][1];
}*/

uint8_t calc_MOD(uint8_t gasId)
{
	int16_t oxygen, maxppO2, result;
	SSettings *pSettings;

	pSettings = settingsGetPointer();

	oxygen = (int16_t)(pSettings->gas[gasId].oxygen_percentage);

	if(pSettings->gas[gasId].note.ub.deco > 0)
		maxppO2 =(int16_t)(pSettings->ppO2_max_deco);
	else
		maxppO2 =(int16_t)(pSettings->ppO2_max_std);

	result = 10 *  maxppO2;
	result /= oxygen;
	result -= 10;

	if(result < 0)
		return 0;

	if(result > 255)
		return 255;

	return result;
}

uint8_t calc_MinOD(uint8_t gasId)
{
	int16_t oxygen, minppO2, result;
	SSettings *pSettings;

	pSettings = settingsGetPointer();

	oxygen = (int16_t)(pSettings->gas[gasId].oxygen_percentage);
	minppO2 =(int16_t)(pSettings->ppO2_min);
	result = 10 *  minppO2;
	result += 9;
	result /= oxygen;
	result -= 10;

	if(result < 0)
		return 0;

	if(result > 255)
		return 255;

	return result;
}
/*
float calc_ppO2(float input_ambient_pressure_bar, SGas* pGas)
{
    float percent_N2 = 0;
	float percent_He = 0;
	float percent_O2 = 0;
    decom_get_inert_gases(input_ambient_pressure_bar, pGas, &percent_N2, &percent_He);
    percent_O2 = 1 - percent_N2 - percent_He;

    return  (input_ambient_pressure_bar - WATER_VAPOUR_PRESSURE) * percent_O2;
}*/

float get_ambiant_pressure_simulation(long dive_time_seconds, float surface_pressure_bar )
{
  static
    long descent_time;
    float depth_meter;

    descent_time = 60 * max_depth / descent_rate_meter_per_min;

    if(dive_time_seconds <= descent_time)
    {
        depth_meter = ((float)(dive_time_seconds * descent_rate_meter_per_min)) / 60;
        return surface_pressure_bar + depth_meter / 10;
    }
    //else if(dive_time_seconds <= (descent_time + bottom_time * 60))
    return surface_pressure_bar + max_depth / 10;



}

void UpdateLifeDataTest(SDiveState * pDiveState)
{
    static int last_second = -1;
    int now =  current_second();
    if(last_second == now)
        return;
    last_second = now;

    pDiveState->lifeData.dive_time_seconds += 1;
    pDiveState->lifeData.pressure_ambient_bar = get_ambiant_pressure_simulation(pDiveState->lifeData.dive_time_seconds,pDiveState->lifeData.pressure_surface_bar);

    pDiveState->lifeData.depth_meter = (pDiveState->lifeData.pressure_ambient_bar - pDiveState->lifeData.pressure_surface_bar) * 10.0f;
		if(pDiveState->lifeData.max_depth_meter < pDiveState->lifeData.depth_meter)
				pDiveState->lifeData.max_depth_meter = pDiveState->lifeData.depth_meter;
    decom_tissues_exposure(1, &pDiveState->lifeData);
    pDiveState->lifeData.ppO2 = decom_calc_ppO2( pDiveState->lifeData.pressure_ambient_bar, &pDiveState->lifeData.actualGas);
    decom_oxygen_calculate_cns(& pDiveState->lifeData.cns, pDiveState->lifeData.ppO2);

    vpm_crush(pDiveState);
}


_Bool vpm_crush(SDiveState* pDiveState)
{
    int i = 0;
    static float starting_ambient_pressure = 0;
    static float ending_ambient_pressure = 0;
    static float time_calc_begin = -1;
	static float initial_helium_pressure[16];
	static float initial_nitrogen_pressure[16];
	ending_ambient_pressure = pDiveState->lifeData.pressure_ambient_bar * 10;

	if((pDiveState->lifeData.dive_time_seconds <= 4) || (starting_ambient_pressure >= ending_ambient_pressure))
	{
		time_calc_begin = pDiveState->lifeData.dive_time_seconds;
		starting_ambient_pressure = pDiveState->lifeData.pressure_ambient_bar * 10;
		for( i = 0; i < 16; i++)
		{
			initial_helium_pressure[i] = pDiveState->lifeData.tissue_helium_bar[i] * 10;
			initial_nitrogen_pressure[i] = pDiveState->lifeData.tissue_nitrogen_bar[i] * 10;
		}
		return false;
	}
	if(pDiveState->lifeData.dive_time_seconds - time_calc_begin >= 4)
	{
		if(ending_ambient_pressure > starting_ambient_pressure + 0.5f)
		{
			float rate = (ending_ambient_pressure - starting_ambient_pressure) * 60 / 4;
			calc_crushing_pressure(&pDiveState->lifeData, &pDiveState->vpm, initial_helium_pressure, initial_nitrogen_pressure, starting_ambient_pressure, rate);

			time_calc_begin = pDiveState->lifeData.dive_time_seconds;
			starting_ambient_pressure = pDiveState->lifeData.pressure_ambient_bar * 10;
			for( i = 0; i < 16; i++)
			{
				initial_helium_pressure[i] = pDiveState->lifeData.tissue_helium_bar[i] * 10;
				initial_nitrogen_pressure[i] =  pDiveState->lifeData.tissue_nitrogen_bar[i] * 10;
			}

			return true;
		}

	}
	return false;
};


void createDiveSettings(void)
{
	SSettings* pSettings = settingsGetPointer();

	setActualGasFirst(&stateReal.lifeData);

	stateReal.diveSettings.compassHeading = pSettings->compassBearing;
	stateReal.diveSettings.ascentRate_meterperminute = 10;

	stateReal.diveSettings.diveMode = pSettings->dive_mode;
	stateReal.diveSettings.CCR_Mode = pSettings->CCR_Mode;
	if(stateReal.diveSettings.diveMode == DIVEMODE_CCR)
		stateReal.diveSettings.ccrOption = 1;
	else
		stateReal.diveSettings.ccrOption = 0;
	memcpy(stateReal.diveSettings.gas, pSettings->gas,sizeof(pSettings->gas));
	memcpy(stateReal.diveSettings.setpoint, pSettings->setpoint,sizeof(pSettings->setpoint));
	stateReal.diveSettings.gf_high = pSettings->GF_high;
	stateReal.diveSettings.gf_low = pSettings->GF_low;
	stateReal.diveSettings.input_next_stop_increment_depth_bar = ((float)pSettings->stop_increment_depth_meter) / 10.0f;
	stateReal.diveSettings.last_stop_depth_bar = ((float)pSettings->last_stop_depth_meter) / 10.0f;
	stateReal.diveSettings.vpm_conservatism = pSettings->VPM_conservatism.ub.standard;
	stateReal.diveSettings.deco_type.uw = pSettings->deco_type.uw;
	stateReal.diveSettings.fallbackOption = pSettings->fallbackToFixedSetpoint;
	stateReal.diveSettings.ppo2sensors_deactivated = pSettings->ppo2sensors_deactivated;
	stateReal.diveSettings.future_TTS_minutes = pSettings->future_TTS;
	
	decom_CreateGasChangeList(&stateReal.diveSettings, &stateReal.lifeData); // decogaslist
	stateReal.diveSettings.internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero = 0;

	/* for safety */
	stateReal.diveSettings.input_second_to_last_stop_depth_bar = stateReal.diveSettings.last_stop_depth_bar + stateReal.diveSettings.input_next_stop_increment_depth_bar;
	/* and the proper calc */
	for(int i = 1; i <10; i++)
	{
		if(stateReal.diveSettings.input_next_stop_increment_depth_bar * i > stateReal.diveSettings.last_stop_depth_bar)
		{
			 stateReal.diveSettings.input_second_to_last_stop_depth_bar = stateReal.diveSettings.input_next_stop_increment_depth_bar * i;
			 break;
		}
	}
}


void copyDiveSettingsToSim(void)
{
	memcpy(&stateSim, &stateReal, sizeof(stateReal));
}


void copyVpmRepetetiveDataToSim(void)
{
	SDiveState * pSimData = stateSimGetPointerWrite();
	const SVpmRepetitiveData * pVpmData = stateVpmRepetitiveDataGetPointer();
	
	if(pVpmData->is_data_from_RTE_CPU)
	{
		for(int i=0; i<16;i++)
		{
			pSimData->vpm.adjusted_critical_radius_he[i] = pVpmData->adjusted_critical_radius_he[i];
			pSimData->vpm.adjusted_critical_radius_n2[i] = pVpmData->adjusted_critical_radius_n2[i];

			pSimData->vpm.adjusted_crushing_pressure_he[i] = pVpmData->adjusted_crushing_pressure_he[i];
			pSimData->vpm.adjusted_crushing_pressure_n2[i] = pVpmData->adjusted_crushing_pressure_n2[i];

			pSimData->vpm.initial_allowable_gradient_he[i] = pVpmData->initial_allowable_gradient_he[i];
			pSimData->vpm.initial_allowable_gradient_n2[i] = pVpmData->initial_allowable_gradient_n2[i];

			pSimData->vpm.max_actual_gradient[i] = pVpmData->max_actual_gradient[i];
		}
		pSimData->vpm.repetitive_variables_not_valid = pVpmData->repetitive_variables_not_valid;
	}
}


void updateSetpointStateUsed(void)
{
	SLifeData *pLifeDataWrite;
	
	if(is_stateUsedSetToSim())
		pLifeDataWrite = &stateSimGetPointerWrite()->lifeData;
	else
		pLifeDataWrite = &stateRealGetPointerWrite()->lifeData;

	if(stateUsed->diveSettings.diveMode != DIVEMODE_CCR)
	{
		pLifeDataWrite->actualGas.setPoint_cbar = 0;
		pLifeDataWrite->ppO2 = decom_calc_ppO2(stateUsed->lifeData.pressure_ambient_bar, &stateUsed->lifeData.actualGas);
	}
	else
	{
		if(stateUsed->diveSettings.CCR_Mode == CCRMODE_Sensors)
		{
			pLifeDataWrite->actualGas.setPoint_cbar = get_ppO2SensorWeightedResult_cbar();
		}

		if((stateUsed->lifeData.pressure_ambient_bar * 100) < stateUsed->lifeData.actualGas.setPoint_cbar)
			pLifeDataWrite->ppO2 = stateUsed->lifeData.pressure_ambient_bar;
		else
			pLifeDataWrite->ppO2 = ((float)stateUsed->lifeData.actualGas.setPoint_cbar) / 100;
	}
}

/*
void fallbackToFixedSetpoints(SLifeData *lifeData)
{
	
}
*/

void setActualGasFirst(SLifeData *lifeData)
{
	SSettings* pSettings = settingsGetPointer();
	uint8_t start = 0;
	uint8_t gasId = 0;
	uint8_t setpoint_cbar = 0;

	if(pSettings->dive_mode == DIVEMODE_CCR)
	{
		setpoint_cbar = pSettings->setpoint[1].setpoint_cbar;
		start = NUM_OFFSET_DILUENT+1;
	}
	else
	{
		setpoint_cbar = 0;
		start = 1;
	}

	gasId = start;
	for(int i=start;i<=NUM_GASES+start;i++)
	{
		if(pSettings->gas[i].note.ub.first)
		{
			gasId = i;
			break;
		}
	}
	setActualGas(lifeData, gasId, setpoint_cbar);
}

void setActualGasAir(SLifeData *lifeData)
{
	SSettings* pSettings = settingsGetPointer();
	uint8_t nitrogen;
	nitrogen = 79;
	lifeData->actualGas.GasIdInSettings = 0;
	lifeData->actualGas.nitrogen_percentage = nitrogen;
	lifeData->actualGas.helium_percentage =0;
	lifeData->actualGas.setPoint_cbar = 0;
	lifeData->actualGas.change_during_ascent_depth_meter_otherwise_zero = 0;
}


void setActualGas(SLifeData *lifeData, uint8_t gasId, uint8_t setpoint_cbar)
{
	SSettings* pSettings = settingsGetPointer();
	uint8_t nitrogen;

	nitrogen = 100;
	nitrogen -= pSettings->gas[gasId].oxygen_percentage;
	nitrogen -= pSettings->gas[gasId].helium_percentage;

	lifeData->actualGas.GasIdInSettings = gasId;
	lifeData->actualGas.nitrogen_percentage = nitrogen;
	lifeData->actualGas.helium_percentage = pSettings->gas[gasId].helium_percentage;
	lifeData->actualGas.setPoint_cbar = setpoint_cbar;
	lifeData->actualGas.change_during_ascent_depth_meter_otherwise_zero = 0;
	
	if((pSettings->dive_mode == DIVEMODE_CCR) && (gasId > NUM_OFFSET_DILUENT))
		lifeData->lastDiluent_GasIdInSettings = gasId;
}


void setActualGas_DM(SLifeData *lifeData, uint8_t gasId, uint8_t setpoint_cbar)
{
  //Real dive => Set events for logbook
	if(stateUsed == stateRealGetPointer())
  {
    SDiveState * pStateUsed;
		pStateUsed = stateRealGetPointerWrite();

    if(stateUsed->diveSettings.ccrOption && gasId < 6)
    {
      if(lifeData->actualGas.GasIdInSettings != gasId)
      {
        SSettings* pSettings = settingsGetPointer();
        pStateUsed->events.bailout = 1;
        pStateUsed->events.info_bailoutO2 = pSettings->gas[gasId].oxygen_percentage;
        pStateUsed->events.info_bailoutHe = pSettings->gas[gasId].helium_percentage;
      }
    }
    else
    {
      if(lifeData->actualGas.GasIdInSettings != gasId)
      {
          pStateUsed->events.gasChange = 1;
          pStateUsed->events.info_GasChange = gasId;
      }
      if(	lifeData->actualGas.setPoint_cbar != setpoint_cbar)
      {
				// setPoint_cbar = 255 -> change to sensor mode
        pStateUsed->events.setpointChange = 1;
        pStateUsed->events.info_SetpointChange = setpoint_cbar;
      }
    }
  }
	setActualGas(lifeData, gasId, setpoint_cbar);
}

void setActualGas_ExtraGas(SLifeData *lifeData, uint8_t oxygen, uint8_t helium, uint8_t setpoint_cbar)
{
	uint8_t nitrogen;

	nitrogen = 100;
	nitrogen -= oxygen;
	nitrogen -= helium;

  //Real dive => Set events for logbook
	if(stateUsed == stateRealGetPointer())
  {
    SDiveState * pStateUsed;
		pStateUsed = stateRealGetPointerWrite();
    if((lifeData->actualGas.nitrogen_percentage != nitrogen) || (lifeData->actualGas.helium_percentage != helium))
    {
      pStateUsed->events.manuelGasSet = 1;
      pStateUsed->events.info_manuelGasSetHe = helium;
      pStateUsed->events.info_manuelGasSetO2 = oxygen;
    }
    if(	lifeData->actualGas.setPoint_cbar != setpoint_cbar)
    {
      pStateUsed->events.setpointChange = 1;
      pStateUsed->events.info_SetpointChange = setpoint_cbar;
    }
  }
  lifeData->actualGas.GasIdInSettings = 0;
  lifeData->actualGas.nitrogen_percentage = nitrogen;
  lifeData->actualGas.helium_percentage = helium;
  lifeData->actualGas.setPoint_cbar = setpoint_cbar;
  lifeData->actualGas.change_during_ascent_depth_meter_otherwise_zero = 0;

}

void setButtonResponsiveness(uint8_t *ButtonSensitivyList)
{
	SDataReceiveFromMaster	*pDataOut = dataOutGetPointer();

	for(int i=0; i<4; i++)
	{
		pDataOut->data.buttonResponsiveness[i] = settingsHelperButtonSens_translate_percentage_to_hwOS_values(ButtonSensitivyList[i]);
	}
	pDataOut->setButtonSensitivityNow = 1;
}


void setDate(RTC_DateTypeDef Sdate)
{
	SDataReceiveFromMaster	*pDataOut = dataOutGetPointer();

	pDataOut->data.newDate = Sdate;
	pDataOut->setDateNow = 1;
}


void setTime(RTC_TimeTypeDef Stime)
{
	SDataReceiveFromMaster	*pDataOut = dataOutGetPointer();

	pDataOut->data.newTime = Stime;
	pDataOut->setTimeNow = 1;
}


void setBatteryPercentage(uint8_t newChargePercentage)
{
	SDataReceiveFromMaster	*pDataOut = dataOutGetPointer();

	pDataOut->data.newBatteryGaugePercentageFloat = settingsGetPointer()->lastKnownBatteryPercentage;
	pDataOut->setBatteryGaugeNow = 1;
}


void calibrateCompass(void)
{
	SDataReceiveFromMaster	*pDataOut = dataOutGetPointer();
	pDataOut->calibrateCompassNow = 1;
}


void clearDeco(void)
{
	SDataReceiveFromMaster	*pDataOut = dataOutGetPointer();
	pDataOut->clearDecoNow = 1;
	
	stateRealGetPointerWrite()->cnsHigh_at_the_end_of_dive = 0;
	stateRealGetPointerWrite()->decoMissed_at_the_end_of_dive	= 0;
}


int32_t helper_days_from_civil(int32_t y, uint32_t m, uint32_t d)
{
		y += 2000;
    y -= m <= 2;
    int32_t era = (y >= 0 ? y : y-399) / 400;
    uint32_t yoe = (uint32_t)(y - era * 400);      // [0, 399]
    uint32_t doy = (153*(m + (m > 2 ? -3 : 9)) + 2)/5 + d-1;  // [0, 365]
    uint32_t doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]
    return era * 146097 + (int32_t)(doe) - 719468;
}


uint8_t helper_weekday_from_days(int32_t z)
{
    return (uint8_t)(z >= -4 ? (z+4) % 7 : (z+5) % 7 + 6);
}


void setWeekday(RTC_DateTypeDef *sDate)
{
	uint8_t day;
	// [0, 6] -> [Sun, Sat]
	day = helper_weekday_from_days(helper_days_from_civil(sDate->Year, sDate->Month, sDate->Date));
	// [1, 7] -> [Mon, Sun]
	if(day == 0)
		day = 7;
	sDate->WeekDay = day;
}


void translateDate(uint32_t datetmpreg, RTC_DateTypeDef *sDate)
{
  datetmpreg = (uint32_t)(datetmpreg & RTC_DR_RESERVED_MASK);

  /* Fill the structure fields with the read parameters */
  sDate->Year = (uint8_t)((datetmpreg & (RTC_DR_YT | RTC_DR_YU)) >> 16);
  sDate->Month = (uint8_t)((datetmpreg & (RTC_DR_MT | RTC_DR_MU)) >> 8);
  sDate->Date = (uint8_t)(datetmpreg & (RTC_DR_DT | RTC_DR_DU));
  sDate->WeekDay = (uint8_t)((datetmpreg & (RTC_DR_WDU)) >> 13);

	/* Convert the date structure parameters to Binary format */
	sDate->Year = (uint8_t)RTC_Bcd2ToByte(sDate->Year);
	sDate->Month = (uint8_t)RTC_Bcd2ToByte(sDate->Month);
	sDate->Date = (uint8_t)RTC_Bcd2ToByte(sDate->Date);
}

void translateTime(uint32_t tmpreg, RTC_TimeTypeDef *sTime)
{
  tmpreg = (uint32_t)(tmpreg & RTC_TR_RESERVED_MASK);

  /* Fill the structure fields with the read parameters */
  sTime->Hours = (uint8_t)((tmpreg & (RTC_TR_HT | RTC_TR_HU)) >> 16);
  sTime->Minutes = (uint8_t)((tmpreg & (RTC_TR_MNT | RTC_TR_MNU)) >>8);
  sTime->Seconds = (uint8_t)(tmpreg & (RTC_TR_ST | RTC_TR_SU));
  sTime->TimeFormat = (uint8_t)((tmpreg & (RTC_TR_PM)) >> 16);

	/* Convert the time structure parameters to Binary format */
	sTime->Hours = (uint8_t)RTC_Bcd2ToByte(sTime->Hours);
	sTime->Minutes = (uint8_t)RTC_Bcd2ToByte(sTime->Minutes);
	sTime->Seconds = (uint8_t)RTC_Bcd2ToByte(sTime->Seconds);
  sTime->SubSeconds = 0;
}


/*
void initDiveState(SDiveSettings * pDiveSettings, SVpm * pVpm)
{
    SSettings* pSettings = settingsGetPointer();
    for(int i = 0; i< NUM_GASES; i++)
    {
        pDiveSettings->gas[i] =  pSettings->gas[i];
        pDiveSettings->gas[NUM_OFFSET_DILUENT + i] =  pSettings->gas[NUM_OFFSET_DILUENT + i];
        pDiveSettings->setpoint[i] =  pSettings->setpoint[i];
    }
    pDiveSettings->diveMode = pSettings->dive_mode;

    pDiveSettings->gf_high = pSettings->GF_high;
    pDiveSettings->gf_low = pSettings->GF_low;
    pDiveSettings->last_stop_depth_bar = ((float)pSettings->last_stop_depth_meter) / 10.0;
    pDiveSettings->ascentRate_meterperminute = 10;
    pDiveSettings->vpm_conservatism = 1;

    pDiveSettings->input_next_stop_increment_depth_bar = ((float)pSettings->stop_increment_depth_meter) / 10.0f;

    vpm_init(pVpm,  pDiveSettings->vpm_conservatism, 0, 0);
}
*/
_Bool deco_zone_reached(void)
{
  	if(stateUsed->diveSettings.deco_type.ub.standard == GF_MODE)
      return stateUsed->lifeData.pressure_ambient_bar <= stateUsed->diveSettings.internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero;
    else
      return stateUsed->vpm.deco_zone_reached;

}


void resetEvents(void)
{
  SDiveState * pStateUsed;
	if(stateUsed == stateRealGetPointer())
		pStateUsed = stateRealGetPointerWrite();
	else
		pStateUsed = stateSimGetPointerWrite();

		memset(&pStateUsed->events,0, sizeof(SEvents));
}


/* This is derived from crc32b but does table lookup. First the table
itself is calculated, if it has not yet been set up.
Not counting the table setup (which would probably be a separate
function), when compiled to Cyclops with GCC, this function executes in
7 + 13n instructions, where n is the number of bytes in the input
message. It should be doable in 4 + 9n instructions. In any case, two
of the 13 or 9 instrucions are load byte.
   This is Figure 14-7 in the text. */

/* http://www.hackersdelight.org/ i guess ;-)  *hw */

uint32_t crc32c_checksum(uint8_t* message, uint16_t length, uint8_t* message2, uint16_t length2) {
	int i, j;
	uint32_t byte, crc, mask;
	static unsigned int table[256] = {0};

	/* Set up the table, if necessary. */
	if (table[1] == 0) {
		for (byte = 0; byte <= 255; byte++) {
			 crc = byte;
			 for (j = 7; j >= 0; j--) {    // Do eight times.
					mask = -(crc & 1);
					crc = (crc >> 1) ^ (0xEDB88320 & mask);
			 }
			 table[byte] = crc;
		}
	}

	/* Through with table setup, now calculate the CRC. */
	i = 0;
	crc = 0xFFFFFFFF;
	while (length--) {
		byte = message[i];
		crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
		i = i + 1;
	}
	if(length2)
	{
	 i = 0;
	 while (length2--) {
			byte = message2[i];
			crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
			i = i + 1;
	 }
	}
	return ~crc;
}


uint32_t	CRC_CalcBlockCRC_moreThan768000(uint32_t *buffer1, uint32_t *buffer2, uint32_t words)
{
 cm_t        crc_model;
 uint32_t      word_to_do;
 uint8_t       byte_to_do;
 int         i;
 
     // Values for the STM32F generator.
 
     crc_model.cm_width = 32;            // 32-bit CRC
     crc_model.cm_poly  = 0x04C11DB7;    // CRC-32 polynomial
     crc_model.cm_init  = 0xFFFFFFFF;    // CRC initialized to 1's
     crc_model.cm_refin = FALSE;         // CRC calculated MSB first
     crc_model.cm_refot = FALSE;         // Final result is not bit-reversed
     crc_model.cm_xorot = 0x00000000;    // Final result XOR'ed with this
 
     cm_ini(&crc_model);
 
     while (words--)
     {
         // The STM32F10x hardware does 32-bit words at a time!!!
				if(words > (768000/4))
					word_to_do = *buffer2++;
				else
					word_to_do = *buffer1++;
 
         // Do all bytes in the 32-bit word.
 
         for (i = 0; i < sizeof(word_to_do); i++)
         {
             // We calculate a *byte* at a time. If the CRC is MSB first we
             // do the next MS byte and vica-versa.
 
             if (crc_model.cm_refin == FALSE)
             {
                 // MSB first. Do the next MS byte.
 
                 byte_to_do = (uint8_t) ((word_to_do & 0xFF000000) >> 24);
                 word_to_do <<= 8;
             }
             else
             {
                 // LSB first. Do the next LS byte.
 
                 byte_to_do = (uint8_t) (word_to_do & 0x000000FF);
                 word_to_do >>= 8;
             }
 
             cm_nxt(&crc_model, byte_to_do);
         }
     }
 
     // Return the final result.
 
     return (cm_crc(&crc_model));
}
 
 
uint32_t	CRC_CalcBlockCRC(uint32_t *buffer, uint32_t words)
{
 cm_t        crc_model;
 uint32_t      word_to_do;
 uint8_t       byte_to_do;
 int         i;
 
     // Values for the STM32F generator.
 
     crc_model.cm_width = 32;            // 32-bit CRC
     crc_model.cm_poly  = 0x04C11DB7;    // CRC-32 polynomial
     crc_model.cm_init  = 0xFFFFFFFF;    // CRC initialized to 1's
     crc_model.cm_refin = FALSE;         // CRC calculated MSB first
     crc_model.cm_refot = FALSE;         // Final result is not bit-reversed
     crc_model.cm_xorot = 0x00000000;    // Final result XOR'ed with this
 
     cm_ini(&crc_model);
 
     while (words--)
     {
         // The STM32F10x hardware does 32-bit words at a time!!!
 
         word_to_do = *buffer++;
 
         // Do all bytes in the 32-bit word.
 
         for (i = 0; i < sizeof(word_to_do); i++)
         {
             // We calculate a *byte* at a time. If the CRC is MSB first we
             // do the next MS byte and vica-versa.
 
             if (crc_model.cm_refin == FALSE)
             {
                 // MSB first. Do the next MS byte.
 
                 byte_to_do = (uint8_t) ((word_to_do & 0xFF000000) >> 24);
                 word_to_do <<= 8;
             }
             else
             {
                 // LSB first. Do the next LS byte.
 
                 byte_to_do = (uint8_t) (word_to_do & 0x000000FF);
                 word_to_do >>= 8;
             }
 
             cm_nxt(&crc_model, byte_to_do);
         }
     }
 
     // Return the final result.
 
     return (cm_crc(&crc_model));
}
 

_Bool is_ambient_pressure_close_to_surface(SLifeData *lifeData)
{
	if(lifeData->pressure_ambient_bar < (lifeData->pressure_surface_bar + 0.04f))
		return true;
	else
		return false;
}

uint8_t stateUsed_scooterRemainingBattCapacity(void)
{
	const uint8_t useCapacityValue = 1; // 2 is the new one, 1 = scooterRestkapazitaetWhBased is the official used
	
	switch(useCapacityValue)
	{
		case 0:
		default:
			return stateUsed->lifeData.scooterRestkapazitaet;

		case 1:
			return stateUsed->lifeData.scooterRestkapazitaetWhBased;
			
		case 2:
			return stateUsed->lifeData.scooterRestkapazitaetVoltageBased;
	}		
}
