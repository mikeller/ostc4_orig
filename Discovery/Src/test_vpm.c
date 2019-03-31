///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/test_vpm.c
/// \brief  test 101
/// \author Heinrichs Weikamp
/// \date   26-Oct-2014
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

#include <stdio.h>
#include <stdint.h>
//#include "LED.h"
//#include "Keyboard.h"
//#include "stm32f4xx_hal.h"
#include "buehlmann.h"
#include "calc_crush.h"
#include "vpm.h"
#include "display.h"
#include "test_vpm.h"
#include "math.h"
#include "data_central.h"
#include "decom.h"
#include "logbook.h"
#include "tInfoLog.h"

#define true 1
#define false 0
//#define uint8_t unsigned char

extern SSettings Settings;

_Bool simulate_descent(SDiveState* pInput, float ending_depth_meter, float rate_meter_per_minutes);
void init_buehlmann(SDiveState* pInput);

_Bool test1(void);
uint8_t test2_unapproved(void);
uint8_t test3_unapproved(void);

_Bool simulate_descent(SDiveState* pInput,
                       float ending_depth_meter,
                       float rate_meter_per_minutes)
{
    int i =0;
    static	float initial_helium_pressure[16];
    static	float initial_nitrogen_pressure[16];
    static	float initial_inspired_he_pressure;
    static	float initial_inspired_n2_pressure;
    static	float fraction_nitrogen_begin;
    static	float fraction_nitrogen_end;
    static	float fraction_helium_begin;
    static	float fraction_helium_end;
    static	float nitrogen_rate;
    static	float helium_rate;
    static	float time;

    extern const float WATER_VAPOR_PRESSURE;
    extern const float HELIUM_TIME_CONSTANT[];
    extern const float NITROGEN_TIME_CONSTANT[];

    float starting_ambient_pressure = pInput->lifeData.pressure_ambient_bar * 10;
    float ending_ambient_pressure = ending_depth_meter + pInput->lifeData.pressure_surface_bar * 10;

    if((rate_meter_per_minutes <= 0) || (starting_ambient_pressure >= ending_ambient_pressure))
        return 0;

    for(i=0; i<16; i++)
    {
        initial_helium_pressure[i] = pInput->lifeData.tissue_helium_bar[i] * 10.0f;
        initial_nitrogen_pressure[i] = pInput->lifeData.tissue_nitrogen_bar[i] * 10.0f;
    }

    //New
    time = (ending_ambient_pressure - starting_ambient_pressure) / rate_meter_per_minutes;
    decom_get_inert_gases(starting_ambient_pressure / 10, &pInput->lifeData.actualGas, &fraction_nitrogen_begin, &fraction_helium_begin );
    decom_get_inert_gases(ending_ambient_pressure   / 10, &pInput->lifeData.actualGas, &fraction_nitrogen_end, &fraction_helium_end );
    initial_inspired_he_pressure =	(starting_ambient_pressure - WATER_VAPOR_PRESSURE) * fraction_helium_begin;
    initial_inspired_n2_pressure =	(starting_ambient_pressure - WATER_VAPOR_PRESSURE) * fraction_nitrogen_begin;
    helium_rate = ((ending_ambient_pressure  - WATER_VAPOR_PRESSURE)* fraction_helium_end - initial_inspired_he_pressure)/time;
    nitrogen_rate = ((ending_ambient_pressure  - WATER_VAPOR_PRESSURE)* fraction_nitrogen_end - initial_inspired_n2_pressure)/time;
    pInput->lifeData.pressure_ambient_bar = ending_ambient_pressure/10;

    for( i = 0; i < 16; i++)
    {
        pInput->lifeData.tissue_helium_bar[i] = schreiner_equation__2(&initial_inspired_he_pressure,
        &helium_rate,
        &time,
        &HELIUM_TIME_CONSTANT[i],
        &initial_helium_pressure[i])/10.0f;

         pInput->lifeData.tissue_nitrogen_bar[i] = schreiner_equation__2(&initial_inspired_n2_pressure,
        &nitrogen_rate,
        &time,
        &NITROGEN_TIME_CONSTANT[i],
        &initial_nitrogen_pressure[i]) / 10.0f;
    }


    calc_crushing_pressure(&pInput->lifeData, &pInput->vpm,initial_helium_pressure,initial_nitrogen_pressure,starting_ambient_pressure, rate_meter_per_minutes);

    pInput->lifeData.dive_time_seconds += ((ending_ambient_pressure - starting_ambient_pressure)/rate_meter_per_minutes) * 60;
    return 1;
}
void init_buehlmann(SDiveState* pInput)
{
    pInput->diveSettings.internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero = 0;
    pInput->lifeData.dive_time_seconds = 0;
    for(int i=0;i<BUEHLMANN_STRUCT_MAX_GASES;i++)
    {
        pInput->diveSettings.decogaslist[i].change_during_ascent_depth_meter_otherwise_zero = 0;
        pInput->diveSettings.decogaslist[i].nitrogen_percentage = 79 - i;
        pInput->diveSettings.decogaslist[i].helium_percentage = i;
        pInput->diveSettings.decogaslist[i].setPoint_cbar = 0;
    }
    pInput->lifeData.actualGas = pInput->diveSettings.decogaslist[0];
    pInput->diveSettings.last_stop_depth_bar = 0.3f;
    pInput->diveSettings.input_next_stop_increment_depth_bar = 0.3f;

    pInput->decolistVPM.output_time_to_surface_seconds = 0;
    pInput->decolistFutureVPM.output_time_to_surface_seconds = 0;
    pInput->decolistBuehlmann.output_time_to_surface_seconds = 0;
    pInput->decolistFutureBuehlmann.output_time_to_surface_seconds = 0;
    for(int i=0;i<DECOINFO_STRUCT_MAX_STOPS;i++)
    {
        pInput->decolistVPM.output_stop_length_seconds[i] = 0;
        pInput->decolistFutureVPM.output_stop_length_seconds[i] = 0;

        pInput->decolistBuehlmann.output_stop_length_seconds[i] = 0;
        pInput->decolistFutureBuehlmann.output_stop_length_seconds[i] = 0;
    }
    for(int i=0;i<16;i++)
    {
        pInput->lifeData.tissue_nitrogen_bar[i] = 0.750927f;
        pInput->lifeData.tissue_helium_bar[i] = 0;
    }
    pInput->diveSettings.gf_high = 80;
    pInput->diveSettings.gf_low = 20;
    pInput->diveSettings.vpm_conservatism = 2;

    pInput->lifeData.pressure_surface_bar = 1.0f;
    pInput->lifeData.pressure_ambient_bar = 1.0f;

    pInput->warnings.decoMissed = 0;
    pInput->events.gasChange = 0;
    pInput->events.info_GasChange = 0;
    pInput->events.info_manuelGasSetO2 = 0;
    pInput->events.info_manuelGasSetHe = 0;
    pInput->events.manuelGasSet = 0;
    pInput->warnings.ppO2High = 0;
    pInput->warnings.ppO2Low = 0;
    pInput->warnings.slowWarning = 0;


    //pInput->decolistVPM.UNUSED_input_necessary_stop_length_seconds_otherwise_zero[i] = 0;
    /*for(i=0;i<BUEHLMANN_STRUCT_MAX_ASCENDRATES;i++)
    {
        pInput->lifeData.ascentrate[i].rate_bar_per_minute = 1.2f;
        pInput->lifeData.ascentrate[i].use_from_depth_bar = 0; // only one ascendrate at the moment
    }*/
    //pInput->diveSettings.input_second_stop_depth_bar = 0.6f;
    //pInput->lifeData.actual_gas_id = 0;
    //pInput->lifeData.actual_setpoint_bar_if_rebreather_otherwise_zero = 0;
    //pInput->lifeData.distance_used_below_stop_levels_bar = 0;
//	pInput->lifeData.pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero = 0;

}


void init_buehlmann2(SDiveState* pInput)
{
    pInput->diveSettings.internal__pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero = 0;
    pInput->lifeData.dive_time_seconds = 0;
    int i=0;
    for(i=0;i<BUEHLMANN_STRUCT_MAX_GASES;i++)
    {
        pInput->diveSettings.decogaslist[i].change_during_ascent_depth_meter_otherwise_zero = 0;
        pInput->diveSettings.decogaslist[i].nitrogen_percentage = 20;
        pInput->diveSettings.decogaslist[i].helium_percentage = 70;
        pInput->diveSettings.decogaslist[i].setPoint_cbar = 0;
    }
    pInput->lifeData.actualGas = pInput->diveSettings.decogaslist[0];
    /*for(i=0;i<BUEHLMANN_STRUCT_MAX_ASCENDRATES;i++)
    {
        pInput->lifeData.ascentrate[i].rate_bar_per_minute = 1.2f;
        pInput->lifeData.ascentrate[i].use_from_depth_bar = 0; // only one ascendrate at the moment
    }*/
    pInput->diveSettings.last_stop_depth_bar = 0.3f;
    //pInput->diveSettings.input_second_stop_depth_bar = 0.6f;
    pInput->diveSettings.input_next_stop_increment_depth_bar = 0.3f;
    pInput->decolistVPM.output_time_to_surface_seconds = 0;
    pInput->decolistFutureVPM.output_time_to_surface_seconds = 0;
    pInput->decolistBuehlmann.output_time_to_surface_seconds = 0;
    pInput->decolistFutureBuehlmann.output_time_to_surface_seconds = 0;
    for(int i=0;i<DECOINFO_STRUCT_MAX_STOPS;i++)
    {
        pInput->decolistVPM.output_stop_length_seconds[i] = 0;
        pInput->decolistFutureVPM.output_stop_length_seconds[i] = 0;

        pInput->decolistBuehlmann.output_stop_length_seconds[i] = 0;
        pInput->decolistFutureBuehlmann.output_stop_length_seconds[i] = 0;
    }
    for(i=0;i<16;i++)
    {
        pInput->lifeData.tissue_nitrogen_bar[i] = 0.750927f;
        pInput->lifeData.tissue_helium_bar[i] = 0;
    }

//	pInput->lifeData.distance_used_below_stop_levels_bar = 0;
    pInput->diveSettings.gf_high = 80;
    pInput->diveSettings.gf_low = 20;
    pInput->diveSettings.vpm_conservatism = 2;
//	pInput->lifeData.pressure_first_stop_ambient_bar_as_upper_limit_for_gf_low_otherwise_zero = 0;
    pInput->lifeData.pressure_surface_bar = 1.0f;
    pInput->lifeData.pressure_ambient_bar = 1.0f;
}


_Bool test1()
{
    /* debug code with watch */
    static int32_t output_time_to_surface_minutes;
    static int32_t counter = 0;
    //static float decotable_minutes[DECOINFO_STRUCT_MAX_STOPS];

    /* all the rest */
    SDiveState input;

    init_buehlmann(&input);
    //vpm conservatism = 0, repetitive = false,
    vpm_init(&input.vpm,0,false,0);

    //runter auf 70 meter mit 26 meter/minute
    simulate_descent(&input, 70.0f, 26.0f);
    //10 minuten settigung
    //buehlmann__test__saturate_tissues(&input,  10 * 60);
     decom_tissues_exposure(10 * 60, &input.lifeData);
    //buehlmann_calc_deco(&input);

    vpm_calc(&(input.lifeData),&(input.diveSettings),&(input.vpm),&(input.decolistVPM), DECOSTOPS);

    //Check time to surface 46 min +- 0.6
    // MultiDeco hw: 42 min

    output_time_to_surface_minutes = input.decolistVPM.output_time_to_surface_seconds / 60;
    if (output_time_to_surface_minutes != 46)
        counter = 0;
    else
        counter++;

    if(fabsf( ((float)input.decolistVPM.output_time_to_surface_seconds / 60.0f) - 46.0f) >= 0.6f)
        return false;
/*	for(i=0;i<DECOINFO_STRUCT_MAX_STOPS;i++)
    {

        if(decotable_minutes[i] != ((float)input.decolistVPM.output_stop_length_seconds[i]) / 60.0f)
        {
            counter2++;
            decotable_minutes[i] = ((float)input.decolistVPM.output_stop_length_seconds[i]) / 60.0f;
        }
    }
    i = i;*/

    vpm_saturation_after_ascent(&input.lifeData);
    input.vpm.decomode_vpm_plus_conservatism_last_dive = input.diveSettings.vpm_conservatism;

    //Pause 60 min
     decom_tissues_exposure(60 * 60, &input.lifeData );
    //buehlmann__test__saturate_tissues(&input,  60 * 60);
    vpm_init(&input.vpm,0,true, 60 * 60);
    //runter auf 70 meter mit 26 meter/minute
    simulate_descent(&input, 70.0f, 26.0f);
    //10 minuten settigung
    //buehlmann__test__saturate_tissues(&input,  10 * 60);
    decom_tissues_exposure(10 * 60, &input.lifeData);
    vpm_calc(&(input.lifeData),&(input.diveSettings),&(input.vpm),&(input.decolistVPM), DECOSTOPS);
    //Check time to surface 46 min +- 0.6
    // MultiDeco hw: 42 min

    output_time_to_surface_minutes = input.decolistVPM.output_time_to_surface_seconds / 60;

    if(fabsf( ((float)input.decolistVPM.output_time_to_surface_seconds / 60.0f) - 57.0f) >= 0.6f)
        return false;

    return true;
}


uint8_t test2_unapproved(void)
{
    /* debug code with watch */
    static int32_t output_time_to_surface_minutes;
    static int32_t counter = 0;
    static float decotable_minutes[DECOINFO_STRUCT_MAX_STOPS];
    static int32_t counter2 = 0;

    /* all the rest */
    SDiveState input;
    int i;

    init_buehlmann(&input);
    //vpm conservatism = 3, repetitive = false,
    vpm_init(&(input.vpm),3,false,0);

    //runter auf 70 meter mit 26 meter/minute
    simulate_descent(&input, 70.0f, 26.0f);
    //30 minuten saetigung
    //buehlmann__test__saturate_tissues(&input,  30 * 60);
     decom_tissues_exposure(30 * 60, &input.lifeData );
    //buehlmann_calc_deco(&input);

        vpm_calc(&(input.lifeData),&(input.diveSettings),&(input.vpm),&(input.decolistVPM), DECOSTOPS);
    //Check time to surface 179.833 min (Peter Version 140415) +- 0.6, MultiDeco is 195 min

    output_time_to_surface_minutes = input.decolistVPM.output_time_to_surface_seconds / 60;
    if (output_time_to_surface_minutes != 180)
        counter = 0;
    else
        counter++;

    if(fabsf( ((float)input.decolistVPM.output_time_to_surface_seconds / 60.0f) - 180.0f) >= 0.6f)
        return false;
    for(i=0;i<DECOINFO_STRUCT_MAX_STOPS;i++)
    {
        if(decotable_minutes[i] != ((float)input.decolistVPM.output_stop_length_seconds[i]) / 60.0f)
        {
            counter2++;
            decotable_minutes[i] = ((float)input.decolistVPM.output_stop_length_seconds[i]) / 60.0f;
        }
    }
    return true;
}

/**
  ******************************************************************************
  * @brief		test 3
  *						Trimix 10/70
    *						everything else identical to test1 by Peter Ryser
  * @version 	V0.0.1
  * @date   	19-April-2014
  * @retval 	1 for result is similar to DRx code, 0 otherwise
  ******************************************************************************
  */
uint8_t test3_unapproved(void)
{

    /* debug code with watch */
    static int32_t output_time_to_surface_minutes;
    static int32_t counter = 0;
    static float decotable_minutes[DECOINFO_STRUCT_MAX_STOPS];
    static int32_t counter2 = 0;

    /* all the rest */
    SDiveState input;
    int i;

    init_buehlmann2(&input);
    //vpm conservatism = 0, repetitive = false,
    vpm_init(&(input.vpm),0,false,0);

    //runter auf 70 meter mit 26 meter/minute
    simulate_descent(&input, 70.0f, 26.0f);
    //10 minuten settigung
     decom_tissues_exposure(10 * 60, &input.lifeData);
    //buehlmann__test__saturate_tissues(&input,  10 * 60);
    //buehlmann_calc_deco(&input);

       vpm_calc(&(input.lifeData),&(input.diveSettings),&(input.vpm),&(input.decolistVPM), DECOSTOPS);

    //Check time to surface 46 min +- 0.6

    output_time_to_surface_minutes = input.decolistVPM.output_time_to_surface_seconds / 60;
    if (output_time_to_surface_minutes != 46)
        counter = 0;
    else
        counter++;

    if(fabsf( ((float)input.decolistVPM.output_time_to_surface_seconds / 60.0f) - 46.0f) >= 0.6f)
        return false;
    for(i=0;i<DECOINFO_STRUCT_MAX_STOPS;i++)
    {
        if(decotable_minutes[i] != ((float)input.decolistVPM.output_stop_length_seconds[i]) / 60.0f)
        {
            counter2++;
            decotable_minutes[i] = ((float)input.decolistVPM.output_stop_length_seconds[i]) / 60.0f;
        }
    }
    return true;
}

/**
  ******************************************************************************
  * @brief		test 4 - find the limit
  *						Trimix 10/70
    *						200 Meter, 30 Minuten
  * @version 	V0.0.1
  * @date   	19-April-2014
  * @retval 	1 for result is similar to DRx code, 0 otherwise
  ******************************************************************************
  */
uint8_t test4_unapproved(void)
{

    /* debug code with watch */
    static int32_t output_time_to_surface_minutes;
    static int32_t counter = 0;
    static float decotable_minutes[DECOINFO_STRUCT_MAX_STOPS];
    static int32_t counter2 = 0;

    /* all the rest */
    SDiveState input;
    int i;

    init_buehlmann2(&input);
    //vpm conservatism = 0, repetitive = false,
    vpm_init(&input.vpm,0,false,0);

    //runter auf 70 meter mit 26 meter/minute
    simulate_descent(&input, 200.0f, 26.0f);
    //10 minuten settigung
     decom_tissues_exposure(10 * 60, &input.lifeData );
    //buehlmann__test__saturate_tissues(&input,  30 * 60);
    //buehlmann_calc_deco(&input);

    vpm_calc(&(input.lifeData),&(input.diveSettings),&(input.vpm),&(input.decolistVPM), DECOSTOPS);

    //Check time to surface 1270 min
    // Multi Deco 1270 Minuten

    output_time_to_surface_minutes = input.decolistVPM.output_time_to_surface_seconds / 60;
    if (output_time_to_surface_minutes != 1270)
        counter = 0;
    else
        counter++;

    if(fabsf( ((float)input.decolistVPM.output_time_to_surface_seconds / 60.0f) - 1270.0f) >= 0.6f)
        return false;
    for(i=0;i<DECOINFO_STRUCT_MAX_STOPS;i++)
    {
        if(decotable_minutes[i] != ((float)input.decolistVPM.output_stop_length_seconds[i]) / 60.0f)
        {
            counter2++;
            decotable_minutes[i] = ((float)input.decolistVPM.output_stop_length_seconds[i]) / 60.0f;
        }
    }
    return true;
}

/*uint8_t test5_unapproved(uint32_t frame1, uint32_t frame2, uint32_t frame3, uint32_t frame4)*/
uint8_t test5_unapproved(void)
{
    /* debug code with watch */
    static int32_t output_time_to_surface_minutes;
    static int32_t counter = 0;

//	static int32_t counter2 = 0;
    /* all the rest */
    SDiveState input;

    //uint32_t frame[5];

    uint8_t vpm_count;

    /*
    frame[0] = frame1;
    frame[1] = frame2;
    frame[2] = frame3;
    frame[3] = frame4;
    frame[4] = frame[0];
*/
    init_buehlmann(&input);
    vpm_init(&input.vpm,0,false,0);
    logbook_initNewdiveProfile(&input,&Settings);
    setSimulationValues(12, 26 , 70, 30);

    long time = 60 * 70 / 26 + 10 *60;

    vpm_count = 0;
    while(input.lifeData.dive_time_seconds < time )
    {
/*				frame[4] = frame[0];
                frame[0] = frame[1];
                frame[1] = frame[2];
                frame[2] = frame[3];
                frame[3] = frame[4];*/
        UpdateLifeDataTest(&input);

                vpm_count++;
                if(vpm_count > 20)
                {
                    vpm_calc(&input.lifeData, &(input.diveSettings),&input.vpm, &input.decolistVPM, DECOSTOPS);
                    vpm_count = 0;
                }

/*
#ifdef VGAOUT
                tVGA_refresh(frame[1], &input);
                GFX_VGA_transform(frame[1],frame[0]);
                GFX_SetFrameBuffer(frame[0], TOP_LAYER);
                GFX_clear_buffer(frame[3]); // frame[3] is the previous frame[0]
#endif
*/
            if(input.lifeData.dive_time_seconds == 60 *5)
            {
                input.events.gasChange = 1;
                input.events.info_GasChange = 2;
            }
            else
            {
                input.events.gasChange = 0;
                input.events.info_GasChange = 0;
            }
            logbook_writeSample(input);
    }
        volatile SLogbookHeader* logbookHeader = logbook_getCurrentHeader();

        logbookHeader->total_diveTime_seconds = input.lifeData.dive_time_seconds;
        logbookHeader->maxDepth = input.lifeData.max_depth_meter * 100;
         logbook_EndDive();

    output_time_to_surface_minutes = input.decolistVPM.output_time_to_surface_seconds / 60;
    if (output_time_to_surface_minutes != 46)
        counter = 0;
    else
        counter++;

    if(fabsf( ((float)input.decolistVPM.output_time_to_surface_seconds / 60.0f) - 46.0f) >= 0.6f)
        return false;

    return true;
}


uint8_t test6_unapproved(void)
{
    /* debug code with watch */
    static int32_t output_time_to_surface_minutes;
    static int32_t counter = 0;

//	static int32_t counter2 = 0;
    /* all the rest */
    SDiveState input;

    //uint32_t frame[5];

    uint8_t vpm_count;

    init_buehlmann(&input);
    vpm_init(&input.vpm,0,false,0);
    logbook_initNewdiveProfile(&input,&Settings);
    setSimulationValues(12, 26 , 65, 15);

    long time = 60 * 70 / 26 + 10 *60;

    vpm_count = 0;
    while(input.lifeData.dive_time_seconds < time )
    {
        UpdateLifeDataTest(&input);

                vpm_count++;
                if(vpm_count > 20)
                {
                    vpm_calc(&input.lifeData, &(input.diveSettings),&input.vpm, &input.decolistVPM, DECOSTOPS);
                    vpm_count = 0;
                }
                if(input.lifeData.dive_time_seconds == 60 *5)
                {
                        input.events.gasChange = 1;
                        input.events.info_GasChange = 2;
                }
                else
                {
                        input.events.gasChange = 0;
                        input.events.info_GasChange = 0;
                }
            logbook_writeSample(input);
    }
    volatile SLogbookHeader* logbookHeader = logbook_getCurrentHeader();

    logbookHeader->total_diveTime_seconds = input.lifeData.dive_time_seconds;
    logbookHeader->maxDepth = input.lifeData.max_depth_meter * 100;
    logbook_EndDive();

    output_time_to_surface_minutes = input.decolistVPM.output_time_to_surface_seconds / 60;
    if (output_time_to_surface_minutes != 46)
        counter = 0;
    else
        counter++;

    if(fabsf( ((float)input.decolistVPM.output_time_to_surface_seconds / 60.0f) - 46.0f) >= 0.6f)
        return false;

    return true;
}

uint8_t test7_unapproved(void)
{
    /* debug code with watch */
    static int32_t output_time_to_surface_minutes;
    static int32_t counter = 0;

//	static int32_t counter2 = 0;
    /* all the rest */
    SDiveState input;

    //uint32_t frame[5];

    uint8_t vpm_count;

    init_buehlmann(&input);
    vpm_init(&input.vpm,0,false,0);
    logbook_initNewdiveProfile(&input,&Settings);
    setSimulationValues(12, 26 , 40, 45);

    long time = 60 * 70 / 26 + 10 *60;

    vpm_count = 0;
    while(input.lifeData.dive_time_seconds < time )
    {
        UpdateLifeDataTest(&input);

                vpm_count++;
                if(vpm_count > 20)
                {
                    vpm_calc(&input.lifeData,&input.diveSettings, &input.vpm, &input.decolistVPM, DECOSTOPS);
                    vpm_count = 0;
                }
                if(input.lifeData.dive_time_seconds == 60 *5)
                {
                        input.events.gasChange = 1;
                        input.events.info_GasChange = 2;
                }
                else
                {
                        input.events.gasChange = 0;
                        input.events.info_GasChange = 0;
                }
            logbook_writeSample(input);
    }
    volatile SLogbookHeader* logbookHeader = logbook_getCurrentHeader();

    logbookHeader->total_diveTime_seconds = input.lifeData.dive_time_seconds;
    logbookHeader->maxDepth = input.lifeData.max_depth_meter * 100;
    logbook_EndDive();

    output_time_to_surface_minutes = input.decolistVPM.output_time_to_surface_seconds / 60;
    if (output_time_to_surface_minutes != 46)
        counter = 0;
    else
        counter++;

    if(fabsf( ((float)input.decolistVPM.output_time_to_surface_seconds / 60.0f) - 46.0f) >= 0.6f)
        return false;

    return true;
}

void test_log_only(uint8_t max_depth_meter, uint16_t divetime_minutes)
{
    SDiveState input;
    float ascendrate_seconds;
    float descendrate_seconds;
    uint32_t divetime_seconds;
    uint32_t divetime_start_ascend;

    init_buehlmann(&input);
    input.lifeData.max_depth_meter = 0.0;
    input.lifeData.depth_meter = 0.0;
    input.lifeData.temperature_celsius = 22.7;

    ascendrate_seconds = 12.0 / 60.0;
    descendrate_seconds = 20.0 / 60.0;
    divetime_seconds = divetime_minutes * 60;
    divetime_start_ascend = divetime_seconds - (uint32_t)(max_depth_meter / ascendrate_seconds);

    logbook_initNewdiveProfile(&input,&Settings);

    while(input.lifeData.dive_time_seconds < divetime_seconds )
    {
            input.lifeData.dive_time_seconds += 1;

            if(input.lifeData.max_depth_meter < (float)max_depth_meter)
            {
                input.lifeData.depth_meter += descendrate_seconds;
                input.lifeData.max_depth_meter = input.lifeData.depth_meter;
            }
            else if((input.lifeData.dive_time_seconds >= divetime_start_ascend) && (input.lifeData.depth_meter > 0))
            {
                input.lifeData.depth_meter -= ascendrate_seconds;
                if(input.lifeData.depth_meter < 0)
                    input.lifeData.depth_meter = 0;
            }

            logbook_writeSample(input);
    }
    volatile SLogbookHeader* logbookHeader = logbook_getCurrentHeader();
    logbookHeader->total_diveTime_seconds = input.lifeData.dive_time_seconds;
    logbookHeader->maxDepth = input.lifeData.max_depth_meter * 100;
    logbook_EndDive();
}


/**
  ******************************************************************************
  * @brief		test 101
  *						a) for air
    *						b) air + oxygen
    * 					c) Trimix 10/70 + oxygen
    *						65 Meter, 42 Minuten with descent
  * @version 	V0.0.1
  * @date   	26-Oct-2014
    * @retval 	ToDo: 1 for result is similar to MultiDeco
  ******************************************************************************
  */

uint8_t test101_buehlmann_unapproved(void)
{
    /* all the rest */
    SDiveState input;

    init_buehlmann(&input);

    //Gas Change at 6 meter to oxygin
    //input.diveSettings.decogaslist[1].change_during_ascent_depth_bar_otherwise_zero = 0.6f;
    //input.diveSettings.decogaslist[1].nitrogen_percentage = 0;
    //input.diveSettings.decogaslist[1].helium_percentage = 0;
    input.diveSettings.gf_high = 100;
    input.diveSettings.gf_low = 100;
    input.diveSettings.ascentRate_meterperminute = 10.0f;
    input.diveSettings.last_stop_depth_bar = 0.3f; //	input.diveSettings.last_stop_depth_bar = 0.6f; /* ist egal bei oxygen */

    //runter auf 65 meter mit 20 meter/minute
    simulate_descent(&input, 65.0f, 20.0f);
    //38min 45sec saettigung == 2325 sec
     decom_tissues_exposure(30*60, &input.lifeData );
//	 decom_tissues_exposure(2325, &input.lifeData );

  //vpm_calc(&(input.lifeData),&(input.diveSettings),&(input.vpm),&(input.decolistVPM));
    buehlmann_calc_deco(&input.lifeData,&input.diveSettings,&input.decolistBuehlmann);


    //Check time to surface MultiDeco 4.04
    // 308 min with Air
    // 190,5 min with Air + 6m last stop with oxygen
    // 538 min with Trimix 10/70 and oxygen at 6m

    // ...

    return true;
}
