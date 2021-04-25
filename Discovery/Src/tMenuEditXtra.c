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
#include "data_exchange_main.h"
#include "motion.h"


/* Private function prototypes -----------------------------------------------*/
void openEdit_CompassHeading(void);
void openEdit_ResetStopwatch(void);
void openEdit_SimFollowDecostops(void);
void openEdit_SetManualMarker(void);
void openEdit_SetEndDive(void);
void openEdit_CalibViewport(void);
static void openEdit_ScrubberTimer(uint8_t line);
static void openEdit_ScrubberReset(void);
static void openEdit_ScrubberTimerMode(void);

/* Announced function prototypes -----------------------------------------------*/
uint8_t OnAction_CompassHeading	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
static uint8_t OnAction_ScrubberTimer(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);

/* Exported functions --------------------------------------------------------*/

void openEdit_Xtra(uint8_t line)
{
    set_globalState_Menu_Line(line);
    resetMenuEdit(CLUT_MenuPageXtra);

    /* DIVE MODE */
    if(actual_menu_content != MENU_SURFACE)
    {
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
				openEdit_CalibViewport();
				break;
			case 5:
				if(is_stateUsedSetToSim())
				{
					 openEdit_SimFollowDecostops();
				}
				else
				{
					openEdit_SetEndDive();
				}
				break;
		}
    }
    else /* surface mode */
    {
		switch(line)
		{
			case 1:	openEdit_ScrubberTimer(line);
				break;
			case 2: openEdit_ScrubberReset();
				break;
			case 3:	openEdit_ScrubberTimerMode();
				break;
			default:
				break;
		}
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
    stateUsedWrite->events.manualMarker = 1;
    exitMenuEdit_to_Home();
}

void openEdit_SetEndDive(void)
{
	dataOutGetPointer()->setEndDive = 1;
    exitMenuEdit_to_Home();
}

void openEdit_SimFollowDecostops(void)
{
    simulation_set_heed_decostops(!simulation_get_heed_decostops());
    exitMenuEdit_to_Menu_with_Menu_Update();
}

void openEdit_CalibViewport(void)
{
	calibrateViewport(stateUsed->lifeData.compass_roll, stateUsed->lifeData.compass_pitch, stateUsed->lifeData.compass_heading);
	suspendMotionDetection(0);	/* exit to focus mode directly */
    exitMenuEdit_to_Home();
}



static void openEdit_ScrubberTimer(uint8_t line)
{
    uint16_t localScrubTimer;
    uint16_t y_line;

    char text[32];
    SSettings *pSettings = settingsGetPointer();
    localScrubTimer = pSettings->scrubTimerMax;

    y_line = ME_Y_LINE_BASE + (line * ME_Y_LINE_STEP);

    text[0] = '\001';
    text[1] = TXT_ScrubTime;
    text[2] = 0;
    write_topline(text);

    text[0] = '\002';
    strcpy(&text[1],"\016\016");
    text[3] = TXT_Minutes;
    text[4] = 0;
    write_label_fix(   20, 800, y_line, &FontT48, TXT_ScrubTime);
    write_label_var(  435, 780, y_line, &FontT48, text);
    write_field_udigit(StMXTRA_ScrubTimer_Max_Minutes, 600, 779, y_line, &FontT48, "###", (uint32_t)localScrubTimer, 0, 0, 0);

    write_buttonTextline(TXT2BYTE_ButtonMinus,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonPlus);

    setEvent(StMXTRA_ScrubTimer_Max_Minutes,	(uint32_t)OnAction_ScrubberTimer);
    startEdit();
}

static void openEdit_ScrubberReset(void)
{
	 SSettings *pSettings;
     pSettings = settingsGetPointer();
     pSettings->scrubTimerCur = pSettings->scrubTimerMax;
     exitMenuEdit_to_Menu_with_Menu_Update();
}

static void openEdit_ScrubberTimerMode(void)
{
	 uint8_t newMode;
	 SSettings *pSettings;
     pSettings = settingsGetPointer();
     newMode = pSettings->scrubTimerMode + 1;
     if(newMode >= SCRUB_TIMER_END)
     {
    	 newMode = SCRUB_TIMER_OFF;
     }
     pSettings->scrubTimerMode = newMode;
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
	stateUsedWrite->diveSettings.compassHeading = (uint16_t)stateUsed->lifeData.compass_heading;
    exitMenuEdit_to_Home_with_Menu_Update();
    return EXIT_TO_HOME;
}

static uint8_t OnAction_ScrubberTimer(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings;
    uint8_t digitContentNew = EXIT_TO_MENU;
    uint32_t newScrubberTime;

    if(action == ACTION_BUTTON_ENTER)
    {
        return digitContent;
    }
    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &newScrubberTime, 0, 0, 0);

        if(newScrubberTime > MAX_SCRUBBER_TIME)
        	newScrubberTime = MAX_SCRUBBER_TIME;

        pSettings = settingsGetPointer();
        pSettings->scrubTimerMax = newScrubberTime;
        if(pSettings->scrubTimerCur > newScrubberTime)
        {
        	pSettings->scrubTimerCur = newScrubberTime;
        }

        tMenuEdit_newInput(editId, newScrubberTime, 0, 0, 0);
        digitContentNew = UPDATE_AND_EXIT_TO_MENU;
    }
    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if(digitContentNew > '9')
            digitContentNew = '0';
    }
    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - 1;
        if(digitContentNew < '0')
            digitContentNew = '9';
    }
    return digitContentNew;
}

