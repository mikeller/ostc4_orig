/**
  ******************************************************************************
  * @file    check_warning.c
  * @author  heinrichs weikamp gmbh
  * @date    17-Nov-2014
  * @version V0.0.1
  * @since   17-Nov-2014
  * @brief   check and set warnings for warnings
  *
  @verbatim
  ==============================================================================
              ##### How to use #####
  ==============================================================================
  OSTC3 Warnings:
		niedriger Batteriezustand (
		zu hoher oder zu niedriger Sauerstoffpartialdruck (ppO2) 0.2 - 1.6
		zu hoher CNS (Gefahr der Sauerstoffvergiftung) 90%
		zu hohe Gradientenfaktoren 90 - 90
		Missachtung der Dekostopps (der �berschrittene Dekostopp wird rot angezeigt) 0 m
		zu hohe Aufstiegsgeschwindigkeit 30 m/min
		aGF-Warnung: die Berechnung der Dekompression wird �ber alternative GF-Werte durchgef�hrt
		Fallback-Warnung bei ausgefallenem Sensor

	@endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "data_exchange.h"
#include "check_warning.h"
#include "settings.h"
#include "decom.h"
#include "tCCR.h"

/* Private variables wit access ----------------------------------------------*/
uint8_t betterGasId = 0;
uint8_t betterSetpointId = 0;
int8_t fallback = 0;

/* Private function prototypes -----------------------------------------------*/
int8_t check_fallback(SDiveState * pDiveState);
int8_t check_ppO2(SDiveState * pDiveState);
int8_t check_O2_sensors(SDiveState * pDiveState);
int8_t check_CNS(SDiveState * pDiveState);
int8_t check_Deco(SDiveState * pDiveState);
int8_t check_AscentRate(SDiveState * pDiveState);
int8_t check_aGF(SDiveState * pDiveState);
int8_t check_BetterGas(SDiveState * pDiveState);
int8_t check_BetterSetpoint(SDiveState * pDiveState);
int8_t check_Battery(SDiveState * pDiveState);

int8_t check_helper_same_oxygen_and_helium_content(SGasLine * gas1, SGasLine * gas2);

/* Exported functions --------------------------------------------------------*/

void check_warning(void)
{
	SDiveState * pDiveState;

	if(stateUsed == stateRealGetPointer())
		pDiveState = stateRealGetPointerWrite();
	else
		pDiveState = stateSimGetPointerWrite();

  check_warning2(pDiveState);
}


void check_warning2(SDiveState * pDiveState)
{
  pDiveState->warnings.numWarnings = 0;

	pDiveState->warnings.numWarnings += check_aGF(pDiveState);
	pDiveState->warnings.numWarnings += check_AscentRate(pDiveState);
	pDiveState->warnings.numWarnings += check_CNS(pDiveState);
	pDiveState->warnings.numWarnings += check_Deco(pDiveState);
	pDiveState->warnings.numWarnings += check_ppO2(pDiveState);
	pDiveState->warnings.numWarnings += check_O2_sensors(pDiveState);
	pDiveState->warnings.numWarnings += check_BetterGas(pDiveState);
	pDiveState->warnings.numWarnings += check_BetterSetpoint(pDiveState);
	pDiveState->warnings.numWarnings += check_Battery(pDiveState);
	pDiveState->warnings.numWarnings += check_fallback(pDiveState);
}


void set_warning_fallback(void)
{
	fallback = 1;
}


void clear_warning_fallback(void)
{
	fallback = 0;
}


uint8_t actualBetterGasId(void)
{
	return betterGasId;
}


uint8_t actualBetterSetpointId(void)
{
	return betterSetpointId;
}


uint8_t actualLeftMaxDepth(const SDiveState * pDiveState)
{
	if(pDiveState->lifeData.depth_meter > (pDiveState->lifeData.max_depth_meter - 3.0f))
		return 0;
	else
		return 1;
}


/* Private functions ---------------------------------------------------------*/
int8_t check_fallback(SDiveState * pDiveState)
{
	if(fallback && ((pDiveState->mode != MODE_DIVE) || (pDiveState->diveSettings.diveMode != DIVEMODE_CCR)))
		fallback = 0;
	
	pDiveState->warnings.fallback = fallback;
	return pDiveState->warnings.fallback;
}


int8_t check_ppO2(SDiveState * pDiveState)
{
	if(pDiveState->mode != MODE_DIVE)
	{
		pDiveState->warnings.ppO2Low = 0;
		pDiveState->warnings.ppO2High = 0;
		return 0;
	}

	uint8_t localPPO2, testPPO2high;

	if(pDiveState->lifeData.ppO2 < 0)
		localPPO2 = 0;
	else
	if(pDiveState->lifeData.ppO2 >= 2.5f)
		localPPO2 = 255;
	else
	localPPO2 = (uint8_t)(pDiveState->lifeData.ppO2 * 100);

	if((localPPO2 + 1) <= settingsGetPointer()->ppO2_min)
			pDiveState->warnings.ppO2Low = 1;
	else
			pDiveState->warnings.ppO2Low = 0;
	
	if(actualLeftMaxDepth(pDiveState))
		testPPO2high = settingsGetPointer()->ppO2_max_deco;
	else
		testPPO2high = settingsGetPointer()->ppO2_max_std;

	if(localPPO2 >= (testPPO2high + 1))
			pDiveState->warnings.ppO2High = 1;
	else
			pDiveState->warnings.ppO2High = 0;

	return pDiveState->warnings.ppO2Low + pDiveState->warnings.ppO2High;
}


int8_t check_O2_sensors(SDiveState * pDiveState)
{
	pDiveState->warnings.sensorLinkLost = 0;
	pDiveState->warnings.sensorOutOfBounds[0] = 0;
	pDiveState->warnings.sensorOutOfBounds[1] = 0;
	pDiveState->warnings.sensorOutOfBounds[2] = 0;

	if((pDiveState->diveSettings.diveMode == DIVEMODE_CCR) && (pDiveState->diveSettings.CCR_Mode == CCRMODE_Sensors))
	{
		if(!get_HUD_battery_voltage_V())
			pDiveState->warnings.sensorLinkLost = 1;
		
		test_HUD_sensor_values_outOfBounds(&pDiveState->warnings.sensorOutOfBounds[0], &pDiveState->warnings.sensorOutOfBounds[1], &pDiveState->warnings.sensorOutOfBounds[2]);
		
	}
	return 		pDiveState->warnings.sensorLinkLost
					+ pDiveState->warnings.sensorOutOfBounds[0]
					+ pDiveState->warnings.sensorOutOfBounds[1]
					+ pDiveState->warnings.sensorOutOfBounds[2];
}


int8_t check_BetterGas(SDiveState * pDiveState)
{
	if(stateUsed->mode != MODE_DIVE)
	{
		pDiveState->warnings.betterGas = 0;
		betterGasId = 0;
		return 0;
	}

	uint8_t  gasIdOffset;
	uint8_t bestGasDepth, betterGasIdLocal;

  SLifeData* pLifeData = &pDiveState->lifeData;
  SDiveSettings* pDiveSettings = &pDiveState->diveSettings;

  pDiveState->warnings.betterGas = 0;
	betterGasId = 0;
	betterGasIdLocal = pLifeData->actualGas.GasIdInSettings;
	bestGasDepth = 255;

	if(pDiveSettings->diveMode == DIVEMODE_CCR)
		gasIdOffset = NUM_OFFSET_DILUENT;
	else
		gasIdOffset = 0;

	/* life data is float, gas data is uint8 */
	if(actualLeftMaxDepth(pDiveState)) /* deco gases */
	{
		for(int i=1+gasIdOffset; i<= 5+gasIdOffset; i++)
		{
			if(	 (pDiveSettings->gas[i].note.ub.active)
				&& (pDiveSettings->gas[i].note.ub.deco)
				&& (pDiveSettings->gas[i].depth_meter)
				&& (pDiveSettings->gas[i].depth_meter >= (pLifeData->depth_meter - 0.01f ))
				&& (pDiveSettings->gas[i].depth_meter <= bestGasDepth)
				)
				{
					betterGasIdLocal = i;
					bestGasDepth = pDiveSettings->gas[i].depth_meter;
				}
		}

		if(betterGasIdLocal != pLifeData->actualGas.GasIdInSettings)
		{
			if(!check_helper_same_oxygen_and_helium_content(&pDiveSettings->gas[betterGasIdLocal], &pDiveSettings->gas[pLifeData->actualGas.GasIdInSettings]))
			{
				betterGasId = betterGasIdLocal;
				pDiveState->warnings.betterGas = 1;
			}
		}
	}
	else /* travel gases */
	{
	  bestGasDepth = 0;
	  //check for travalgas
	  for(int i=1+gasIdOffset; i<= 5+gasIdOffset; i++)
    {
      if(	 (pDiveSettings->gas[i].note.ub.active)
        && (pDiveSettings->gas[i].note.ub.travel)
        && (pDiveSettings->gas[i].depth_meter_travel)
        && (pDiveSettings->gas[i].depth_meter_travel <= (pLifeData->depth_meter + 0.01f ))
        && (pDiveSettings->gas[i].depth_meter_travel >= bestGasDepth)
        )
        {
          betterGasIdLocal = i;
          bestGasDepth = pDiveSettings->gas[i].depth_meter;
        }
    }

    if(betterGasIdLocal != pLifeData->actualGas.GasIdInSettings)
    {
			if(!check_helper_same_oxygen_and_helium_content(&pDiveSettings->gas[betterGasIdLocal], &pDiveSettings->gas[pLifeData->actualGas.GasIdInSettings]))
			{
				betterGasId = betterGasIdLocal;
				pDiveState->warnings.betterGas = 1;
			}
    }
	}
	return pDiveState->warnings.betterGas;
}

/* check for better travel!!! setpoint hw 151210
 */ 
int8_t check_BetterSetpoint(SDiveState * pDiveState)
{
	pDiveState->warnings.betterSetpoint = 0;
	betterSetpointId = 0;

	if((stateUsed->mode != MODE_DIVE) || (pDiveState->diveSettings.diveMode != DIVEMODE_CCR) || (pDiveState->diveSettings.CCR_Mode != CCRMODE_FixedSetpoint))
	{
		return 0;
	}
	
	uint8_t bestSetpointDepth = 0; // travel the deeper, the better
	uint8_t betterSetpointIdLocal = 0; // nothing better

	if(!actualLeftMaxDepth(pDiveState)) /* travel gases */
	{
		for(int i=1; i<=NUM_GASES; i++)
		{
			if(	 (pDiveState->diveSettings.setpoint[i].note.ub.active)
				&& (pDiveState->diveSettings.setpoint[i].depth_meter)
				&& (pDiveState->diveSettings.setpoint[i].depth_meter <= ( pDiveState->lifeData.depth_meter + 0.01f ))
				&& (pDiveState->diveSettings.setpoint[i].depth_meter >= bestSetpointDepth)
			)
				{
					betterSetpointIdLocal = i;
					bestSetpointDepth = pDiveState->diveSettings.setpoint[i].depth_meter;
				}
		}
		if((betterSetpointIdLocal) && (pDiveState->diveSettings.setpoint[betterSetpointIdLocal].setpoint_cbar  != pDiveState->lifeData.actualGas.setPoint_cbar))
		{
			betterSetpointId = betterSetpointIdLocal;
			pDiveState->warnings.betterSetpoint = 1;
		}
	}
	return pDiveState->warnings.betterSetpoint;
}


/* hw 151030
 */
int8_t check_helper_same_oxygen_and_helium_content(SGasLine * gas1, SGasLine * gas2) 
{
	if(gas1->helium_percentage != gas2->helium_percentage)
		return 0;
	else
	if(gas1->oxygen_percentage != gas2->oxygen_percentage)
		return 0;
	else
		return 1;
}


int8_t check_CNS(SDiveState * pDiveState)
{
	if(stateUsed->mode != MODE_DIVE)
	{
		pDiveState->warnings.cnsHigh = 0;
		return 0;
	}
	
	if(pDiveState->lifeData.cns >= (float)(settingsGetPointer()->CNS_max))
			pDiveState->warnings.cnsHigh = 1;
	else
			pDiveState->warnings.cnsHigh = 0;
	return pDiveState->warnings.cnsHigh;
}


int8_t check_Battery(SDiveState * pDiveState)
{
	if(pDiveState->lifeData.battery_charge < 10)
		pDiveState->warnings.lowBattery = 1;
	else
		pDiveState->warnings.lowBattery = 0;
	
  return pDiveState->warnings.lowBattery;
}


int8_t check_Deco(SDiveState * pDiveState)
{
	if(stateUsed->mode != MODE_DIVE)
	{
		pDiveState->warnings.decoMissed = 0;
		return 0;
	}

	uint8_t depthNext = decom_get_actual_deco_stop(pDiveState);
	
	if(!depthNext)
      pDiveState->warnings.decoMissed = 0;
	else
  if(pDiveState->lifeData.depth_meter + 0.1f < (float)depthNext)
      pDiveState->warnings.decoMissed = 1;
  else
      pDiveState->warnings.decoMissed = 0;
	
  return pDiveState->warnings.decoMissed;
}


int8_t check_AscentRate(SDiveState * pDiveState)
{
	if(stateUsed->mode != MODE_DIVE)
	{
		pDiveState->warnings.ascentRateHigh = 0;
		return 0;
	}

	float warnAscentRateFloat;

	warnAscentRateFloat = (float)(settingsGetPointer()->ascent_MeterPerMinute_max);

	if(pDiveState->lifeData.ascent_rate_meter_per_min >= warnAscentRateFloat)
			pDiveState->warnings.ascentRateHigh = 1;
	else
			pDiveState->warnings.ascentRateHigh = 0;
	return pDiveState->warnings.ascentRateHigh;
}


int8_t check_aGF(SDiveState * pDiveState)
{
	if(stateUsed->mode != MODE_DIVE)
	{
		pDiveState->warnings.aGf = 0;
		return 0;
	}

  pDiveState->warnings.aGf = 0;
  if(pDiveState->diveSettings.deco_type.ub.standard == GF_MODE)
  {
    if((pDiveState->diveSettings.gf_high != settingsGetPointer()->GF_high) || (pDiveState->diveSettings.gf_low != settingsGetPointer()->GF_low))
      pDiveState->warnings.aGf = 1;
  }
  return pDiveState->warnings.aGf;
}

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/

