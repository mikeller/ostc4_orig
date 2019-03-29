///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tMenuEditXtra.c
/// \brief  Menu Edit Xtra - Specials in Divemode like Reset Stopwatch
/// \author heinrichs weikamp gmbh
/// \date   02-Mar-2015
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
#include "tMenuEditXtra.h"

#include "gfx_fonts.h"
#include "simulation.h"
#include "timer.h"
#include "tMenuEdit.h"

/* Private function prototypes -----------------------------------------------*/
void openEdit_CompassHeading(void);
void openEdit_ResetStopwatch(void);
void openEdit_SimFollowDecostops(void);
void openEdit_SetManualMarker(void);

/* Announced function prototypes -----------------------------------------------*/
uint8_t OnAction_CompassHeading	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);

/* Exported functions --------------------------------------------------------*/

void openEdit_Xtra(uint8_t line)
{
    set_globalState_Menu_Line(line);
    resetMenuEdit(CLUT_MenuPageXtra);

    switch(line)
    {
    case 1:
    default:
        openEdit_ResetStopwatch();
        break;
    case 2:
        openEdit_CompassHeading();
        break;
    case 3:
        openEdit_SetManualMarker();
        break;
    case 4:
        openEdit_SimFollowDecostops();
        break;
    }
}

/* Private functions ---------------------------------------------------------*/
void openEdit_ResetStopwatch(void)
{
    timer_Stopwatch_Restart();
    exitMenuEdit_to_Home();
}

void openEdit_SetManualMarker(void)
{
    if(stateUsed == stateRealGetPointer())
        stateRealGetPointerWrite()->events.manualMarker = 1;
    else
        stateSimGetPointerWrite()->events.manualMarker = 1;

    exitMenuEdit_to_Home();
}

void openEdit_SimFollowDecostops(void)
{
    simulation_set_heed_decostops(!simulation_get_heed_decostops());
    exitMenuEdit_to_Menu_with_Menu_Update();
}

void refresh_CompassHeading(void)
{
    uint16_t heading;
    char text[32];

    text[0] = '\001';
    text[1] = TXT_2BYTE;
    text[2] = TXT2BYTE_CompassHeading;
    text[3] = 0;
    write_topline(text);

    heading = (uint16_t)stateUsed->lifeData.compass_heading;
    snprintf(text,32,"\001%03i`",heading);
    write_label_var(   0, 800, ME_Y_LINE1, &FontT54, text);

    tMenuEdit_refresh_field(StMXTRA_CompassHeading);
}

void openEdit_CompassHeading(void)
{

    write_field_button(StMXTRA_CompassHeading,20, 800, ME_Y_LINE4, &FontT48, "Set");

    setEvent(StMXTRA_CompassHeading,  (uint32_t)OnAction_CompassHeading);
//	startEdit();
}


uint8_t OnAction_CompassHeading	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    if(is_stateUsedSetToSim())
        stateSimGetPointerWrite()->diveSettings.compassHeading = (uint16_t)stateUsed->lifeData.compass_heading;
    else
        stateRealGetPointerWrite()->diveSettings.compassHeading = (uint16_t)stateUsed->lifeData.compass_heading;
    exitMenuEdit_to_Home_with_Menu_Update();
    return EXIT_TO_HOME;
}


/*
    uint8_t digitContentNew;
    uint32_t newHeading;

    if(action == ACTION_BUTTON_ENTER)
    {
        return digitContent;
    }
    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &newHeading, 0, 0, 0);
        if(newHeading > 359)
            newHeading = 0;

        if(is_stateUsedSetToSim())
            stateSimGetPointerWrite()->diveSettings.compassHeading = newHeading;
        else
            stateRealGetPointerWrite()->diveSettings.compassHeading = newHeading;
        exitMenuEdit_to_Home_with_Menu_Update();
        return EXIT_TO_HOME;
    }
    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if((digitNumber == 0) && (digitContentNew > '3'))
            digitContentNew = '0';
        else
        if(digitContentNew > '9')
            digitContentNew = '0';
        return digitContentNew;
    }
    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - 1;
        if((digitNumber == 0) && (digitContentNew < '0'))
            digitContentNew = '3';
        else
        if(digitContentNew < '0')
            digitContentNew = '9';
        return digitContentNew;
    }

    return EXIT_TO_MENU;
*/
