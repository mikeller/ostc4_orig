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

/* Private variables ---------------------------------------------------------*/
SIrLink receiveHUD[2];
uint8_t boolHUDdata = 0;
uint8_t data_old__lost_connection_to_HUD = 1;

uint8_t receiveHUDraw[16];

uint8_t StartListeningToUART_HUD = 0;
uint16_t count = 0;

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
    StartListeningToUART_HUD = 1;
}


 /* after 3 seconds without update from HUD
    * data is considered old
    */
void tCCR_tick(void)
{
    if(count < 3 * 10)
        count++;
    else
    {
        data_old__lost_connection_to_HUD = 1;
        if(count < 20 * 10)
            count++;
        else
            tCCR_fallbackToFixedSetpoint();
    }
}


void tCCR_restart(void)
{
    HAL_UART_Receive_IT(&UartIR_HUD_Handle, receiveHUDraw, 15);/* 15*/
}


void tCCR_control(void)
{
    if((UartReadyHUD == RESET) && StartListeningToUART_HUD)
    {
            StartListeningToUART_HUD = 0;
            HAL_UART_Receive_IT(&UartIR_HUD_Handle, receiveHUDraw, 15);/* 15*/
    }

    if(UartReadyHUD == SET)
    {
            UartReadyHUD = RESET;

            memcpy(&receiveHUD[!boolHUDdata], receiveHUDraw, 11);
            receiveHUD[!boolHUDdata].battery_voltage_mV = receiveHUDraw[11] + (256 * receiveHUDraw[12]);
            receiveHUD[!boolHUDdata].checksum = receiveHUDraw[13] + (256 * receiveHUDraw[14]);

            uint16_t checksum = 0;

            for(int i=0;i<13;i++)
            {
                checksum += receiveHUDraw[i];
            }
            if(checksum == receiveHUD[!boolHUDdata].checksum)
            {
                boolHUDdata = !boolHUDdata;
                count = 0;
                data_old__lost_connection_to_HUD = 0;
            }
            StartListeningToUART_HUD = 1;
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
