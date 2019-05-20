///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/timer.c
/// \brief  Contains timer related functionality like stopwatch and security stop
/// \author Peter Ryser & heinrichs weikamp gmbh
/// \date   5. Feb.2015 (maybe)
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

#include "data_central.h"

static long stopWatchTime_Second = 0;
static _Bool bStopWatch = false;
static float stopWatchAverageDepth_Meter = 0.0f;
static long safetyStopCountDown_Second = 0;

void timer_init(void)
{
  stopWatchTime_Second = 0;
  stopWatchAverageDepth_Meter = 0.0f;
  bStopWatch = true;
  safetyStopCountDown_Second = 0;

}

void timer_UpdateSecond(_Bool checkOncePerSecond)
{
    static int last_second = -1;
    static _Bool bSafetyStop = false;
    static float last_depth_meter = 0;

    if(checkOncePerSecond)
    {
        int now =  current_second();
        if( last_second == now)
            return;
        last_second = now;
    }

    /** Stopwatch **/
    if(bStopWatch && !is_ambient_pressure_close_to_surface(&stateUsedWrite->lifeData))
    {
        if(stopWatchTime_Second == 0)
            stopWatchAverageDepth_Meter = stateUsed->lifeData.depth_meter;
        else
            stopWatchAverageDepth_Meter = (stopWatchAverageDepth_Meter * stopWatchTime_Second + stateUsed->lifeData.depth_meter)/ (stopWatchTime_Second + 1);

        stopWatchTime_Second++;
    }

    /** SafetyStop **/
    float depthToStopSafetyStopCount;
    if(settingsGetPointer()->safetystopDuration && (stateUsed->lifeData.max_depth_meter > 10.0f) && (stateUsed->lifeData.dive_time_seconds > 60))
    {

        //No deco when 10 meters are crossed from below => Activate SecurityStop
        if( last_depth_meter > 10.0f && stateUsed->lifeData.depth_meter <= 10.0f)
        {
            if(stateUsed->diveSettings.deco_type.ub.standard == GF_MODE)
            {
                if(stateUsed->decolistBuehlmann.output_ndl_seconds >  0)
                    bSafetyStop = true;
            }
            else
            {
                if(stateUsed->decolistVPM.output_ndl_seconds >  0)
                    bSafetyStop = true;
            }
        }

        //Countdown starts at 5 meters
        if(bSafetyStop && (stateUsed->lifeData.depth_meter - 0.0001f <= (settingsGetPointer()->safetystopDepth) ))
        {
            if(safetyStopCountDown_Second == 0)
            {
                safetyStopCountDown_Second = (settingsGetPointer()->safetystopDuration) * 60;
            }
            else
                safetyStopCountDown_Second--;
        }

        // after safetystopDuration minutes or below 3 (2) meter safetyStop is disabled
        if(settingsGetPointer()->safetystopDepth == 3)
            depthToStopSafetyStopCount = 1.999f; // instead of 2
        else
            depthToStopSafetyStopCount = 2.999f;// instead of 3

        if((safetyStopCountDown_Second == 1) || (stateUsed->lifeData.depth_meter <= depthToStopSafetyStopCount))
        {
            bSafetyStop = false;
            safetyStopCountDown_Second = 0;
        }
    }
    else
    {
        bSafetyStop = false;
        safetyStopCountDown_Second = 0;
    }
    last_depth_meter = stateUsed->lifeData.depth_meter;
}


void timer_Stopwatch_Restart(void)
{
  stopWatchTime_Second = 0;
  stopWatchAverageDepth_Meter = stateUsed->lifeData.depth_meter;
  bStopWatch = true;
}

void timer_Stopwatch_Stop(void)
{
   bStopWatch = false;
}

long timer_Stopwatch_GetTime(void)
{
  return stopWatchTime_Second;
}

float timer_Stopwatch_GetAvarageDepth_Meter(void)
{
  return stopWatchAverageDepth_Meter;
}

long timer_Safetystop_GetCountDown(void)
{
  return safetyStopCountDown_Second;
}

uint8_t timer_Safetystop_GetDepthUpperLimit(void)
{
    if(settingsGetPointer()->safetystopDepth == 3)
        return 2;
    else
        return 3;
}

