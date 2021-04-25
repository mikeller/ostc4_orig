///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/unit.c
/// \brief  input to meter/celsius or feet/farenheit
/// \author heinrichs weikamp gmbh
/// \date   24-Feb-2015
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
#include "unit.h"
#include "settings.h"

/* Exported variables --------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/
uint8_t  test;

/* Private variables ---------------------------------------------------------*/

/* Private variables with external access via get_xxx() function -------------*/

/* Private function prototypes -----------------------------------------------*/

/* Announced function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

char unit_depth_char1_T105(void)
{
    if(settingsGetPointer()->nonMetricalSystem)
        return 'f';
    else
        return 'm';
}

char unit_depth_char2_T105(void)
{
    if(settingsGetPointer()->nonMetricalSystem)
        return 't';
    else
        return '\004'; // 004 is nop
}

char unit_depth_char1(void)
{
    if(settingsGetPointer()->nonMetricalSystem)
        return 'f';
    else
        return 'm';
}

char unit_depth_char2(void)
{
    if(settingsGetPointer()->nonMetricalSystem)
        return 't';
    else
        return '\004'; // 004 is nop
}

float unit_depth_float(float input_meter)
{
    if(settingsGetPointer()->nonMetricalSystem == 0)
        return input_meter;
    else
    {
        return 3.2808f * input_meter;
    }
}

uint16_t unit_depth_integer(uint16_t input_meter)
{
    if(settingsGetPointer()->nonMetricalSystem == 0)
        return input_meter;
    else
    {
        return (input_meter * 10) / 3;
    }
}

float unit_temperature_float(float input_celsius)
{
    if(settingsGetPointer()->nonMetricalSystem == 0)
        return input_celsius;
    else
    {
        return input_celsius * (9.0f/5.0f) + 32;
    }
}

int16_t unit_temperature_integer(int16_t input_celsius)
{
    if(settingsGetPointer()->nonMetricalSystem == 0)
        return input_celsius;
    else
    {
        return ((input_celsius * 9 / 5) + 32);
    }
}

uint16_t unit_speed_integer(uint16_t input_meterPerMinute)
{
    if(settingsGetPointer()->nonMetricalSystem == 0)
        return input_meterPerMinute;
    else
    {
        return (input_meterPerMinute * 10) / 3;
    }
}

/* Quelle: https://de.wikipedia.org/wiki/Luftdruck */
/*
const float luftdruckStartMinus300[15] =
{
    1.0530f,
    1.0396f,
    1.0263f,
    1.01325f, // 0 m
    1.0003f,
    0.9876f,
    0.9750f,
    0.9625f,
    0.9503f,
    0.9381f,
    0.9262f,
    0.9144f,
    0.9027f,
    0.8912f, // 1000 m
    0.8358f
};
*/

const int luftdruckStartMinus300[15] =
{
    1053,
    1040,
    1026,
    1013, // 0 m
    1000,
     988,
     975,
     962,
     950,
     938,
     926,
     914,
     903,
     891, // 1000 m
     836
};


int unit_SeaLevelRelation_integer(int input_atmospheric_mbar)
{
    int i = 0;
    int distance1, distance2;
    for(i=0;i<15;i++)
    {
        if(input_atmospheric_mbar >= luftdruckStartMinus300[i])
            break;
    }

    if(i >= 14)
        return 1500;
    else if(i == 0)
        return -300;
    else
    {
        distance1 = input_atmospheric_mbar - luftdruckStartMinus300[i];
        distance2 = luftdruckStartMinus300[i-1] - input_atmospheric_mbar;
        if(distance2 < distance1)
            i -= 1;
        return (i*100) - 300;
    }
}
