///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tCCR.c
/// \brief  HUD data via optical port
/// \author Heinrichs Weikamp gmbh
/// \date   18-Dec-2014
///
/// \details
///
/// $Id$
///////////////////////////////////////////////////////////////////////////////
/// \par Copyright (c) 2014-2018 Heinrichs Weikamp gmbh
///
///     This program is free software: you can redistribute it and/or modify
///     it under the terms of the GNU General Public License as published by
///     the Free Software Foundation, either version 3 of the License, or
///     (at your option) any later version.
///
///     This program is distributed in the hope that it will be useful,
///     but WITHOUT ANY WARRANTY; without even the implied warranty of
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///     GNU General Public License for more details.
///
///     You should have received a copy of the GNU General Public License
///     along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "tCCR.h"
#include "ostc.h"
#include "data_central.h"
#include "data_exchange.h"
#include "check_warning.h"
#include "configuration.h"

/* Private types -------------------------------------------------------------*/
typedef struct
{
    uint8_t hud_firmwareVersion;
    bit8_Type status_byte;
    uint16_t sensor_voltage_100uV[3];
    uint8_t sensor_ppo2_cbar[3];
    uint8_t temp1;
    uint16_t battery_voltage_mV;
    uint16_t checksum;
} 	SIrLink;

#define HUD_BABBLING_IDIOT			(30u)		/* 30 Bytes received without break */
#define HUD_RX_FRAME_LENGTH			(15u)		/* Length of a HUD data frame */
#define HUD_RX_FRAME_BREAK_MS		(100u)		/* Time used to detect a gap between two byte receptions => frame start */
#define HUD_RX_START_DELAY_MS		(500u)		/* Delay for start of RX function to avoid start of reception while a transmission is ongoing. */
												/* Based on an assumed cycle time by the sensor of 1 second. Started at time of last RX */

#define BOTTLE_SENSOR_TIMEOUT		(6000u)     /* signal pressure budget as not received after 10 minutes (6000 * 100ms) */

/* Private variables ---------------------------------------------------------*/
static SIrLink receiveHUD[2];
static uint8_t boolHUDdata = 0;
static uint8_t data_old__lost_connection_to_HUD = 1;

static uint8_t receiveHUDraw[16];

static uint8_t StartListeningToUART_HUD = 0;
static uint16_t HUDTimeoutCount = 0;

static __IO ITStatus UartReadyHUD = RESET;
static uint32_t LastReceivedTick_HUD = 0;

/* Private variables with external access via get_xxx() function -------------*/

/* Private function prototypes -----------------------------------------------*/
static void tCCR_fallbackToFixedSetpoint(void);

#ifndef USART_IR_HUD

void tCCR_init(void)
{
}
void tCCR_control(void)
{
}
void tCCR_test(void)
{
}
void tCCR_restart(void)
{
}
float get_ppO2Sensor_bar(uint8_t sensor_id)
{
}
float get_sensorVoltage_mV(uint8_t sensor_id)
{
}
float get_HUD_battery_voltage_V(void)
{
}
void tCCR_tick(void)
{
}

#else
/* Exported functions --------------------------------------------------------*/

float get_ppO2Sensor_bar(uint8_t sensor_id)
{
    if((sensor_id > 2) || data_old__lost_connection_to_HUD)
        return 0;

    return (float)(receiveHUD[boolHUDdata].sensor_ppo2_cbar[sensor_id]) / 100.0f;
}

float get_sensorVoltage_mV(uint8_t sensor_id)
{
    if((sensor_id > 2) || data_old__lost_connection_to_HUD)
        return 0;

    return (float)(receiveHUD[boolHUDdata].sensor_voltage_100uV[sensor_id]) / 10.0f;
}

float get_HUD_battery_voltage_V(void)
{
    if(data_old__lost_connection_to_HUD)
        return 0;

    return (float)(receiveHUD[boolHUDdata].battery_voltage_mV) / 1000.0f;
}


void test_HUD_sensor_values_outOfBounds(int8_t * outOfBouds1, int8_t * outOfBouds2, int8_t * outOfBouds3)
{
    uint8_t sensorNotActiveBinary;
    uint8_t sensorActive[3];

    // test1: user deactivation
    sensorNotActiveBinary = stateUsed->diveSettings.ppo2sensors_deactivated;

    for(int i=0;i<3;i++)
        sensorActive[i] = 1;

    if(sensorNotActiveBinary)
    {
        if(sensorNotActiveBinary & 1)
            sensorActive[0] = 0;

        if(sensorNotActiveBinary & 2)
            sensorActive[1] = 0;

        if(sensorNotActiveBinary & 4)
            sensorActive[2] = 0;
    }

    // test2: mV of remaining sensors
    for(int i=0;i<3;i++)
    {
        if(sensorActive[i])
        {
            if(	(receiveHUD[boolHUDdata].sensor_voltage_100uV[i] < 80) ||
                    (receiveHUD[boolHUDdata].sensor_voltage_100uV[i] > 2500))
            {
                sensorActive[i] = 0;
                switch(i)
                {
                    case 0:
                        sensorNotActiveBinary |= 1;
                    break;
                    case 1:
                        sensorNotActiveBinary |= 2;
                    break;
                    case 2:
                        sensorNotActiveBinary |= 4;
                    break;
                }
            }
        }
    }

    *outOfBouds1 = 0;
    *outOfBouds2 = 0;
    *outOfBouds3 = 0;

    /* with two, one or no sensor, there is nothing to compare anymore
     */
    if(sensorNotActiveBinary)
    {
        // set outOfBounds for both tests
        if(!sensorActive[0])
            *outOfBouds1 = 1;

        if(!sensorActive[1])
            *outOfBouds2 = 1;

        if(!sensorActive[2])
            *outOfBouds3 = 1;

        return;
    }
    else
    {
        uint8_t sensor_id_ordered[3];
        uint8_t difference[2];

        if((receiveHUD[boolHUDdata].sensor_ppo2_cbar[1]) > (receiveHUD[boolHUDdata].sensor_ppo2_cbar[0]))
        {
            sensor_id_ordered[0] = 0;
            sensor_id_ordered[1] = 1;
        }
        else
        {
            sensor_id_ordered[0] = 1;
            sensor_id_ordered[1] = 0;
        }
        if(receiveHUD[boolHUDdata].sensor_ppo2_cbar[2] > receiveHUD[boolHUDdata].sensor_ppo2_cbar[sensor_id_ordered[1]])
        {
            sensor_id_ordered[2] = 2;
        }
        else
        {
            sensor_id_ordered[2] = sensor_id_ordered[1];
            if(receiveHUD[boolHUDdata].sensor_ppo2_cbar[2] > receiveHUD[boolHUDdata].sensor_ppo2_cbar[sensor_id_ordered[0]])
            {
                sensor_id_ordered[1] = 2;
            }
            else
            {
                sensor_id_ordered[1] = sensor_id_ordered[0];
                sensor_id_ordered[0] = 2;
            }
        }

        difference[0] = receiveHUD[boolHUDdata].sensor_ppo2_cbar[sensor_id_ordered[1]]- receiveHUD[boolHUDdata].sensor_ppo2_cbar[sensor_id_ordered[0]];
        difference[1] = receiveHUD[boolHUDdata].sensor_ppo2_cbar[sensor_id_ordered[2]]- receiveHUD[boolHUDdata].sensor_ppo2_cbar[sensor_id_ordered[1]];

        if((difference[0] > difference[1]) && (difference[0] > 15))
        {
            switch(sensor_id_ordered[0])
            {
            case 0:
                *outOfBouds1 = 1;
            break;
            case 1:
                *outOfBouds2 = 1;
            break;
            case 2:
                *outOfBouds3 = 1;
            break;
            }
        }
        else
        if((difference[0] < difference[1]) && (difference[1] > 15))
        {
            switch(sensor_id_ordered[2])
            {
            case 0:
                *outOfBouds1 = 1;
            break;
            case 1:
                *outOfBouds2 = 1;
            break;
            case 2:
                *outOfBouds3 = 1;
            break;
            }
        }
    }
}


uint8_t get_ppO2SensorWeightedResult_cbar(void)
{
    int8_t sensorOutOfBound[3];
    uint16_t result = 0;
    uint8_t count = 0;

    test_HUD_sensor_values_outOfBounds(&sensorOutOfBound[0], &sensorOutOfBound[1], &sensorOutOfBound[2]);

    for(int i=0;i<3;i++)
    {
        if(!sensorOutOfBound[i])
        {
            result += receiveHUD[boolHUDdata].sensor_ppo2_cbar[i];
            count++;
        }
    }
    if(count == 0) // all sensors out of bounds!
        return 0;
    else
        return (uint8_t)(result / count);
}


void tCCR_init(void)
{
	uint8_t loop;

    StartListeningToUART_HUD = 1;

    SDiveState* pDiveData = stateRealGetPointerWrite();
    for(loop=0;loop<(2*NUM_GASES+1);loop++)
    {
    	pDiveData->lifeData.bottle_bar_age_MilliSeconds[loop] =  BOTTLE_SENSOR_TIMEOUT;
    }
}


 /* after 3 seconds without update from HUD
    * data is considered old
    */
void tCCR_tick(void)
{
    if(HUDTimeoutCount < 3 * 10)
        HUDTimeoutCount++;
    else
    {
        data_old__lost_connection_to_HUD = 1;
        if(HUDTimeoutCount < 20 * 10)
            HUDTimeoutCount++;
        else
            tCCR_fallbackToFixedSetpoint();
    }
}

void tCCR_SetRXIndication(void)
{
	static uint8_t floatingRXCount = 0;

	if((UartIR_HUD_Handle.RxXferSize == HUD_RX_FRAME_LENGTH) || (UartIR_HUD_Handle.RxXferSize == HUD_RX_FRAME_LENGTH - 1))	/* we expected a complete frame */
	{
		UartReadyHUD = SET;
		LastReceivedTick_HUD = HAL_GetTick();
		floatingRXCount = 0;
	}
	else	/* follow up of error handling */
	{
		if(time_elapsed_ms(LastReceivedTick_HUD, HAL_GetTick()) > HUD_RX_FRAME_BREAK_MS)	/* Reception took a while => frame start detected */
		{
			HAL_UART_Receive_IT(&UartIR_HUD_Handle, &receiveHUDraw[1], 14);					/* We have already the first byte => get the missing 14 */
		}
		else
		{
			if(floatingRXCount++ < HUD_BABBLING_IDIOT)
			{
				HAL_UART_Receive_IT(&UartIR_HUD_Handle, receiveHUDraw, 1);					/* Start polling of incoming bytes */
			}
			else																			/* Significant amount of data comming in without break => disable input */
			{																				/* by not reactivation HUD RX, no recovery fromthis state */
				stateUsedWrite->diveSettings.ppo2sensors_deactivated = 0x07;				/* Display deactivation */
			}
		}
	}

}

void tCCR_restart(void)
{
	HAL_UART_AbortReceive_IT(&UartIR_HUD_Handle);	/* Called by the error handler. RX will be restarted by control function */
	StartListeningToUART_HUD = 1;
}


void tCCR_control(void)
{
	uint16_t checksum = 0;
#ifdef ENABLE_BOTTLE_SENSOR
	SDiveState *pLivedata = stateRealGetPointerWrite();
#endif

	if((UartReadyHUD == RESET) && StartListeningToUART_HUD && (time_elapsed_ms(LastReceivedTick_HUD, HAL_GetTick()) > HUD_RX_START_DELAY_MS))
	{
		StartListeningToUART_HUD = 0;
		HAL_UART_Receive_IT(&UartIR_HUD_Handle, receiveHUDraw, HUD_RX_FRAME_LENGTH);
	}

    if(UartReadyHUD == SET)
    {
            UartReadyHUD = RESET;
            StartListeningToUART_HUD = 1;

    /* check if received package is valid */
			for(int i=0;i<13;i++)
			{
				checksum += receiveHUDraw[i];
			}
			receiveHUD[!boolHUDdata].checksum = receiveHUDraw[13] + (256 * receiveHUDraw[14]);
			if(checksum == receiveHUD[!boolHUDdata].checksum)
			{
#ifdef ENABLE_BOTTLE_SENSOR
		        if(receiveHUDraw[0] == 0xA5)				/* code for pressure sensor */
		        {
		        	pLivedata->lifeData.bottle_bar[pLivedata->lifeData.actualGas.GasIdInSettings] = receiveHUDraw[10];
		        	pLivedata->lifeData.bottle_bar_age_MilliSeconds[pLivedata->lifeData.actualGas.GasIdInSettings] = 0;
		        }
		        else
#endif
		        											/* handle O2 sensor data */
		        {
		        	memcpy(&receiveHUD[!boolHUDdata], receiveHUDraw, 11);
					receiveHUD[!boolHUDdata].battery_voltage_mV = receiveHUDraw[11] + (256 * receiveHUDraw[12]);
		        }

				boolHUDdata = !boolHUDdata;
				HUDTimeoutCount = 0;
				data_old__lost_connection_to_HUD = 0;
			}
			else
			{
				if(data_old__lost_connection_to_HUD)	/* we lost connection, maybe due to RX shift => start single byte read to resynchronize */
				{
					HAL_UART_Receive_IT(&UartIR_HUD_Handle, receiveHUDraw, 1);
					StartListeningToUART_HUD = 0;
				}
			}
			memset(receiveHUDraw,0,sizeof(receiveHUDraw));
    }
}

#endif
/* Private functions ---------------------------------------------------------*/

static void tCCR_fallbackToFixedSetpoint(void)
{
    if((stateUsed->mode == MODE_DIVE) && (stateUsed->diveSettings.diveMode == DIVEMODE_CCR) && (stateUsed->diveSettings.CCR_Mode == CCRMODE_Sensors) && (stateUsed->diveSettings.fallbackOption))
    {
        uint8_t setpointCbar, actualGasID;

        setpointCbar = stateUsed->diveSettings.setpoint[1].setpoint_cbar;
        stateUsedWrite->diveSettings.CCR_Mode = CCRMODE_FixedSetpoint;

        actualGasID = stateUsed->lifeData.actualGas.GasIdInSettings;
        setActualGas_DM(&stateUsedWrite->lifeData,actualGasID,setpointCbar);

        set_warning_fallback();
    }
}
