///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tInfoCompass.c
/// \brief  there is only compass_DX_f, compass_DY_f, compass_DZ_f output during this mode
/// \author heinrichs weikamp gmbh
/// \date   23-Feb-2015
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
#include "tInfoCompass.h"

#include "gfx_fonts.h"
#include "tInfo.h"

#include <string.h>

/* Private variables ---------------------------------------------------------*/

uint16_t tInfoCompassTimeout = 0;
int16_t minMaxCompassDX[3][2] = { 0 };

/* Exported functions --------------------------------------------------------*/
void openInfo_Compass(void)
{
    set_globalState(StICOMPASS);
    tInfoCompassTimeout = settingsGetPointer()->timeoutInfoCompass;
    tInfoCompassTimeout *= 10;

    for(int i = 0; i<3;i ++)
    {
            minMaxCompassDX[i][0] = 999;
            minMaxCompassDX[i][1] = -999;
    }
}


//  ===============================================================================
//	refreshInfo_Compass
/// @brief	there is only compass_DX_f, compass_DY_f, compass_DZ_f output during this mode
///					the accel is not called during this process
//  ===============================================================================
void refreshInfo_Compass(void)
{
    tInfoCompassTimeout--;
    if(tInfoCompassTimeout == 0)
    {
        exitInfo();
        return;
    }

    char text[80];

    int16_t compassValues[3];

    compassValues[0] = stateUsed->lifeData.compass_DX_f;
    compassValues[1] = stateUsed->lifeData.compass_DY_f;
    compassValues[2] = stateUsed->lifeData.compass_DZ_f;

    for(int i = 0; i<3;i ++)
    {
        // do not accept zero
        if(minMaxCompassDX[i][0] == 0)
            minMaxCompassDX[i][0] = compassValues[i];

        // do not accept zero
        if(minMaxCompassDX[i][1] == 0)
            minMaxCompassDX[i][1] = compassValues[i];

        if(compassValues[i] < minMaxCompassDX[i][0])
            minMaxCompassDX[i][0] = compassValues[i];

        if(compassValues[i] > minMaxCompassDX[i][1])
            minMaxCompassDX[i][1] = compassValues[i];
    }

    snprintf(text,80,"Time left: %u s",(tInfoCompassTimeout+9)/10);
    tInfo_write_content_simple(  20,800,  20, &FontT42, text, CLUT_InfoCompass);

    for(int i = 0; i<3;i ++)
    {
        snprintf(text,80,"%c: %i" "\t(%i, %i)",
            'X'+i,
            compassValues[i],
            minMaxCompassDX[i][0],
            minMaxCompassDX[i][1]);
        tInfo_write_content_simple(  20,800, 96 + (i*96), &FontT48, text, CLUT_InfoCompass);
    }

    snprintf(text,80,"roll %.1f" "\tpitch %.1f",
        stateUsed->lifeData.compass_roll,
        stateUsed->lifeData.compass_pitch);
    tInfo_write_content_simple(  20,800, 96 * 4, &FontT42, text, CLUT_InfoCompass);
}
