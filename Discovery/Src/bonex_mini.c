///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/bonex_mini.c
/// \brief  voltage to battery percentage based on bonex.c for BIS PCB
/// \author Heinrichs Weikamp gmbh
/// \date   26-March-2017
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
/*
  ==============================================================================
              ##### CAN data #####
  ==============================================================================
  [..] 	is stored static in BONEX_CAN_Config
                see example CAN_Networking for STM32303C_EVAL

  */

/* Includes ------------------------------------------------------------------*/
#include "bonex_mini.h"

/* Private variables ---------------------------------------------------------*/

enum
{
    TYPE_ECOS = 0,
    TYPE_RS = 1,
    TYPE_MAX
};

const uint16_t 	loadVoltageInverted[TYPE_MAX][21] =
{
    {	// ECOS
        0
    },
    {	// RS
        38000,	// 0% >= index *5 ist Ergebnis Kapazitï¿½t
        38875,	// 5%
        39750,	// 10%
        40625,
        41500,
        42050,
        42600,
        43150,
        43700,
        44250,
        44800,
        45350,
        45900,
        46450,
        47000,	// 70%
        47550,	// 75%
        48100,
        48450,	// 85%
        48800,
        49150,
        49500,	//100% , index = 20
    }
};


uint8_t BONEX_mini_ResidualCapacityVoltageBased(float voltage_V, uint16_t ageInMilliSecondsSinceLast)
{
    static uint8_t capacityStorage = 0;
    static uint32_t voltage_mV_storage_32bit = 0;
    static uint16_t storageCounter = 0;

    uint16_t voltage_mV = (uint16_t)(1000 * voltage_V);

    uint8_t calcNow = 0;

    if(ageInMilliSecondsSinceLast < 2000)
    {
        voltage_mV_storage_32bit += voltage_mV;
        storageCounter++;
    }
    else
    {
        storageCounter = 0;
        voltage_mV_storage_32bit = 0;
    }


    if(storageCounter >= 600)
    {
        voltage_mV_storage_32bit /= storageCounter;
        voltage_mV = (uint16_t)voltage_mV_storage_32bit;
        storageCounter = 1;
        voltage_mV_storage_32bit = voltage_mV;
        calcNow = 1;
    }
    else if(storageCounter == 1) // value immediately but not called after 600 counter ;-)
    {
        voltage_mV = (uint16_t)voltage_mV_storage_32bit;
        calcNow = 1;
    }

    if(calcNow)
    {
        for(int i = 20; i>=0; i--)
        {
            if(voltage_mV >= loadVoltageInverted[1][i])
            {
                capacityStorage = i*5;
                break;
            }
        }
    }

    return capacityStorage;
}

/*

uint8_t BONEX_mini_ResidualCapacityVoltageBased(float voltage_V, uint16_t ageInMilliSecondsSinceLast)
{
    static uint8_t capacityStorage = 0;
    static uint16_t voltage_mV_storage[5] = {0,0,0,0,0}; // number six is used directly from voltage_mV

    uint32_t voltage_mV = (uint32_t)(1000 * voltage_V);


    // if necessary reset container and return actual voltage_V as capacity
    if(ageInMilliSecondsSinceLast > 2000)
    {
        capacityStorage = 0;
        for(int i = 0; i<5; i++)
        {
            voltage_mV_storage[i] = 0;
        }
    }

    // find storage container or, if full, use it as number six and recalc voltage_mV based on those six values
    int ptr = -1;
    do
    {
        ptr++;
    } while ((ptr < 5) && voltage_mV_storage[ptr] != 0);

    if(ptr ==	5)
    {
        for(int i = 0; i<5; i++)
        {
            voltage_mV += voltage_mV_storage[i];
            voltage_mV_storage[i] = 0;
        }
        voltage_mV += 3;
        voltage_mV /= 6;
        capacityStorage = 0;
    }
    else
    {
        voltage_mV_storage[ptr] = voltage_mV;
    }

        // calc result if update necessary
    if(capacityStorage == 0)
    {
        for(int i = 20; i>=0; i--)
        {
            if(voltage_mV >= loadVoltageInverted[1][i])
            {
                capacityStorage = i*5;
                break;
            }
        }
    }
    return capacityStorage;
}

#define ECOS_VMAX 290
#define ECOS_VMIN 195
#define ECOS_STEP 5

#define RS_VMAX 500
#define RS_VMIN 360
#define RS_STEP 5

#define ECOS_LENGTH (((ECOS_VMAX - ECOS_VMIN) / ECOS_STEP) + 1)
#define RS_LENGTH 	(((RS_VMAX - RS_VMIN) / RS_STEP) + 1)
#define MAX_LENGTH (ECOS_LENGTH>RS_LENGTH? ECOS_LENGTH:RS_LENGTH)


typedef struct
{
    uint8_t load[3];
} load;


const int32_t 	currentMaxLoad[TYPE_MAX] 			= {  17000,14000};
const int32_t 	currentPartialLoad[TYPE_MAX] 	= {   1000, 1000};
const uint16_t 	voltageCharged[TYPE_MAX] 			= {    280,  480};
const uint16_t 	voltageMax[TYPE_MAX] 					= { ECOS_VMAX, RS_VMAX};
const uint16_t 	voltageMin[TYPE_MAX] 					= { ECOS_VMIN, RS_VMIN};
const uint8_t 	voltageSteps[TYPE_MAX] 				= { ECOS_STEP, RS_STEP};
const uint8_t 	length[TYPE_MAX] 							= { ECOS_LENGTH, RS_LENGTH};





const uint8_t 	loadVoltage[TYPE_MAX][MAX_LENGTH][3] =
{
    {
    // ECOS
    //  no,teil,voll
        {  0,  5,  0}, // voltageMin 19.5
        {  0,  5,  0}, // voltageMin +  0.5V
        {  0,  5,  0}, // 20.5
        {  5,  5,  5}, // 21
        {  5,  5,  5}, // 21.5
        {  5, 10, 10}, // 22
        {  5, 10, 15}, // 22.5
        { 10, 15, 30}, // 23
        { 20, 30, 45}, // 23.5
        { 30, 40, 60}, // 24
        { 40, 50, 65}, // 24.5
        { 50, 60, 75}, // 25
        { 60, 70, 80}, // 25.5
        { 70, 80, 85}, // 26
        { 80, 90, 85}, // 26.5
        { 85, 90, 90}, // 27
        { 90, 95, 90}, // 27.5
        { 95, 95, 95}, // 28
        {100,100,100}, // 28.5
        {100,100,100}, // voltageMax 29
    },
    {
    // RS
    //  no,teil,voll
        {  0,  0,  0}, // voltageMin 36 V
        {  2,  0,  2}, // voltageMin +  0.5V
        {  5,  0,  5}, // 37
        {  5,  2,  5}, //
        {  5,  5,  5}, // 38
        {  5,  5, 10}, //
        {  5,  5, 15}, // 39
        {  7,  7, 17}, //
        { 10, 10, 20}, // 40
        { 15, 12, 27}, //
        { 20, 15, 35}, // 41
        { 27, 22, 42}, //
        { 35, 30, 50}, // 42
        { 42, 37, 55}, //
        { 50, 45, 60}, // 43
        { 55, 50, 67}, //
        { 60, 55, 75}, // 44
        { 67, 57, 80}, //
        { 75, 60, 85}, // 45
        { 77, 65, 87}, //
        { 80, 70, 90}, // 46
        { 85, 75, 90}, //
        { 90, 80, 90}, // 47
        { 92, 85, 92}, //
        { 95, 90, 95}, // 48
        { 95, 92, 97}, //
        { 95, 95,100}, // 49
        { 97, 97,100}, //
        {100,100,100} // 50
    }
};


void BONEX_calc_new_ResidualCapacity(uint8_t *residualC, uint32_t voltage_mV, int32_t current_mA, uint8_t scooterType) // as in BIS
{
    uint8_t actualLoad = 0;
    uint8_t remainder = 0;
    uint32_t voltagePointer = 0;

    if(voltage_mV == 0)
        return;

    if(scooterType >= TYPE_MAX)
        return;

    if(voltage_mV < (voltageMin[scooterType] * 100))
    {
        *residualC = 0;
        return;
    }
    else
    if(voltage_mV >= (voltageMax[scooterType] * 100))
    {
        *residualC = 100;
        return;
    }
    else // check if charged and reset residualC for further calculation
    if(voltage_mV >= (voltageCharged[scooterType] * 100))
    {
        *residualC = 100;
        return;
    }

    // define the line we are working
    if(current_mA >= currentMaxLoad[scooterType])
        actualLoad = 2;
    else
    if(current_mA >= currentPartialLoad[scooterType])
        actualLoad = 1;
    else
        actualLoad = 0;

    voltagePointer = (voltage_mV - ((uint32_t)(voltageMin[scooterType])) * 100) / (voltageSteps[scooterType] * 100);

    // should be checked with if(... >= voltageMax) but for safety
    if(voltagePointer >= length[scooterType])
    {
        *residualC = 100;
        return;
    }

    if(loadVoltage[scooterType][voltagePointer][actualLoad] < *residualC)
        *residualC = loadVoltage[scooterType][voltagePointer][actualLoad];
    else if(loadVoltage[scooterType][voltagePointer][actualLoad] >= (*residualC + 20))
        *residualC = loadVoltage[scooterType][voltagePointer][actualLoad];

    // steps of 5
    remainder = (*residualC)%5;
    if(remainder)
        *residualC += (5 - remainder);

    // safety
    if(*residualC > 100)
        *residualC = 100;

    return;
}
*/

