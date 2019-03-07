///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/t7.c
/// \brief  Main Template file for dive mode 7x
/// \author Heinrichs Weikamp gmbh
/// \date   23-April-2014
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
#include "t7.h"

#include "data_exchange_main.h"
#include "decom.h"
#include "gfx_fonts.h"
#include "logbook_miniLive.h"
#include "math.h"
#include "tHome.h"
#include "simulation.h"
#include "timer.h"
#include "unit.h"

/* Private function prototypes -----------------------------------------------*/

void t7_refresh_surface(void);
void t7_refresh_surface_debugmode(void);
void t7_refresh_divemode(void);
void t7_refresh_sleep_design_fun(void);
void t7_refresh_divemode_userselected_left_lower_corner(void);
void t7_refresh_customview(void);

void draw_frame(_Bool PluginBoxHeader, _Bool LinesOnTheSides, uint8_t colorBox, uint8_t colorLinesOnTheSide);

void t7_tissues(const SDiveState * pState);
void t7_compass(uint16_t ActualHeading, uint16_t UserSetHeading);
void t7_SummaryOfLeftCorner(void);
void t7_debug(void);

void t7_miniLiveLogProfile(void);
//void t7_clock(void);
void t7_logo_OSTC(void);
void t7_colorscheme_mod(char *text);

uint8_t t7_test_customview_warnings(void);
void t7_show_customview_warnings(void);

uint8_t t7_test_customview_warnings_surface_mode(void);
void t7_show_customview_warnings_surface_mode(void);

uint8_t t7_customtextPrepare(char * text);

/* Importend function prototypes ---------------------------------------------*/
extern uint8_t write_gas(char *text, uint8_t oxygen, uint8_t helium);

/* Exported variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
float depthLastCall[9] = { 0,0,0,0,0,0,0,0,0};
uint8_t idDepthLastCall = 0;
float temperatureLastCall[3] = { 0,0,0};
uint8_t idTemperatureLastCall = 0;

GFX_DrawCfgScreen	t7screen;
GFX_DrawCfgScreen	t7screenCompass;

/* left 3 fields
 * right 3 fields
 * centered one field on top of customview, one below
 * customview header + customview + warning
 */
GFX_DrawCfgWindow	t7l1, t7l2, t7l3;
GFX_DrawCfgWindow	t7r1, t7r2, t7r3;
GFX_DrawCfgWindow	t7c1, t7batt, t7c2, t7charge, t7voltage;
GFX_DrawCfgWindow	t7cH, t7cC, t7cW, t7cY0free;
GFX_DrawCfgWindow	t7pCompass;
GFX_DrawCfgWindow	t7surfaceL, t7surfaceR;

uint8_t selection_custom_field = 1;
uint8_t selection_customview = 1;

uint8_t updateNecessary = 0;

typedef struct{
uint32_t pointer;
uint32_t x0;
uint32_t y0;
uint32_t width;
uint32_t height;
} SBackground;

SBackground background =
{
    .pointer = 0,
};


/* Private types -------------------------------------------------------------*/
const uint8_t customviewsDiveStandard[] =
{
    CVIEW_sensors,
    CVIEW_Compass,
    CVIEW_Decolist,
    CVIEW_Tissues,
    CVIEW_Profile,
    CVIEW_Gaslist,
    CVIEW_sensors_mV,
    CVIEW_EADTime,
    CVIEW_SummaryOfLeftCorner,
    CVIEW_noneOrDebug,
    CVIEW_END,
    CVIEW_END
};

const uint8_t customviewsSurfaceStandard[] =
{
//  CVIEW_CompassDebug,
    CVIEW_Hello,
    CVIEW_sensors,
    CVIEW_Compass,
    CVIEW_Tissues,
    CVIEW_sensors_mV,
    CVIEW_END,
    CVIEW_END
};

const uint8_t *customviewsDive		= customviewsDiveStandard;
const uint8_t *customviewsSurface	= customviewsSurfaceStandard;

#define TEXTSIZE 16
/* offset includes line: 2 = line +1
 * box (line) is 300 px
 * inside is 296 px
 * left of box are 249 px ( 0..248)
 * right of box are 249 px (551 .. 799)
 */

#define CUSTOMBOX_LINE_LEFT (250)
#define CUSTOMBOX_LINE_RIGHT (549)
#define CUSTOMBOX_LINE_TOP	  (0)
#define CUSTOMBOX_LINE_MIDDLE  (142)
#define CUSTOMBOX_LINE_BOTTOM  (318)
#define CUSTOMBOX_INSIDE_OFFSET (2)
#define CUSTOMBOX_OUTSIDE_OFFSET (2)
#define CUSTOMBOX_SPACE_INSIDE (CUSTOMBOX_LINE_RIGHT + 1 - (CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET + CUSTOMBOX_INSIDE_OFFSET))
#define TOP_LINE_HIGHT (25)

/* Exported functions --------------------------------------------------------*/

void t7_init(void)
{

	SSettings* pSettings;
	pSettings = settingsGetPointer();

    selection_custom_field = 1;
    selection_customview = customviewsSurface[0];

    t7screen.FBStartAdress = 0;
    t7screen.ImageHeight = 480;
    t7screen.ImageWidth = 800;
    t7screen.LayerIndex = 1;

    t7screenCompass.FBStartAdress = 0;
    t7screenCompass.ImageHeight = 240;
    t7screenCompass.ImageWidth = 1600;
    t7screenCompass.LayerIndex = 0;

    if(!pSettings->FlipDisplay)
    {
		t7l1.Image = &t7screen;
		t7l1.WindowNumberOfTextLines = 2;
		t7l1.WindowLineSpacing = 19; // Abstand von Y0
		t7l1.WindowTab = 100; // vermtl. ohne Verwendung in diesem Fenster
		t7l1.WindowX0 = 0;
		t7l1.WindowX1 = CUSTOMBOX_LINE_LEFT - CUSTOMBOX_OUTSIDE_OFFSET;
		t7l1.WindowY0 = 318;
		t7l1.WindowY1 = 479;

		t7l2.Image = &t7screen;
		t7l2.WindowNumberOfTextLines = 2;
		t7l2.WindowLineSpacing = 22; // Abstand von Y0
		t7l2.WindowTab = 100; // vermtl. ohne Verwendung in diesem Fenster
		t7l2.WindowX0 = 0;
		t7l2.WindowX1 = t7l1.WindowX1;
		t7l2.WindowY0 = 142;
		t7l2.WindowY1 = t7l1.WindowY0 - 5;

		t7l3.Image = &t7screen;
		t7l3.WindowNumberOfTextLines = 2;
		t7l3.WindowLineSpacing = 58; // Abstand von Y0
		t7l3.WindowTab = 100; // vermtl. ohne Verwendung in diesem Fenster
		t7l3.WindowX0 = 0;
		t7l3.WindowX1 = t7l1.WindowX1;
		t7l3.WindowY0 = 0;
		t7l3.WindowY1 = t7l2.WindowY0 - 5;

		t7r1.Image = &t7screen;
		t7r1.WindowNumberOfTextLines = 2;
		t7r1.WindowLineSpacing = t7l1.WindowLineSpacing;
		t7r1.WindowTab = 100;
		t7r1.WindowX0 = 550;
		t7r1.WindowX1 = 799;
		t7r1.WindowY0 = t7l1.WindowY0;
		t7r1.WindowY1 = 479;

		t7r2.Image = &t7screen;
		t7r2.WindowNumberOfTextLines = 2;
		t7r2.WindowLineSpacing = t7l2.WindowLineSpacing;
		t7r2.WindowTab = 100;
		t7r2.WindowX0 = 550;
		t7r2.WindowX1 = 799;
		t7r2.WindowY0 = t7l2.WindowY0;
		t7r2.WindowY1 = t7l2.WindowY1;

		t7r3.Image = &t7screen;
		t7r3.WindowNumberOfTextLines = 2;
		t7r3.WindowLineSpacing = 0;//t7l3.WindowLineSpacing;
		t7r3.WindowTab = 100;
		t7r3.WindowX0 = CUSTOMBOX_LINE_RIGHT + CUSTOMBOX_OUTSIDE_OFFSET;
		t7r3.WindowX1 = 799;
		t7r3.WindowY0 = t7l3.WindowY0;
		t7r3.WindowY1 = t7l3.WindowY1;

		t7cC.Image = &t7screen;
		t7cC.WindowNumberOfTextLines = 3;
		t7cC.WindowLineSpacing = 95; // Abstand von Y0
		t7cC.WindowTab = 100;
		t7cC.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
		t7cC.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
		t7cC.WindowY0 = 90;
		t7cC.WindowY1 = 434 - 95;

		t7cH.Image = &t7screen;
		t7cH.WindowNumberOfTextLines = 1;
		t7cH.WindowLineSpacing = 95; // Abstand von Y0
		t7cH.WindowTab = 100;
		t7cH.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
		t7cH.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
		t7cH.WindowY0 = 434 - 94;
		t7cH.WindowY1 = 434;

		t7cW.Image = &t7screen;
		t7cW.WindowNumberOfTextLines = 3;
		t7cW.WindowLineSpacing = 95; // Abstand von Y0
		t7cW.WindowTab = 100;
		t7cW.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
		t7cW.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
		t7cW.WindowY0 = 90;
		t7cW.WindowY1 = 434 - 95;


		t7surfaceL.Image = &t7screen;
		t7surfaceL.WindowNumberOfTextLines = 9;
		t7surfaceL.WindowLineSpacing = 53;
		t7surfaceL.WindowTab = 100;
		t7surfaceL.WindowX0 = 0;
		t7surfaceL.WindowX1 = CUSTOMBOX_LINE_LEFT - CUSTOMBOX_OUTSIDE_OFFSET;
		t7surfaceL.WindowY0 = 0;
		t7surfaceL.WindowY1 = 480;

		t7surfaceR.Image = &t7screen;
		t7surfaceR.WindowNumberOfTextLines = 9;
		t7surfaceR.WindowLineSpacing = 53;
		t7surfaceR.WindowTab = 100;
		t7surfaceR.WindowX0 = CUSTOMBOX_LINE_RIGHT + CUSTOMBOX_OUTSIDE_OFFSET;
		t7surfaceR.WindowX1 = 800;
		t7surfaceR.WindowY0 = 0;
		t7surfaceR.WindowY1 = 480;

		t7cY0free.Image = &t7screen;
		t7cY0free.WindowNumberOfTextLines = 1;
		t7cY0free.WindowLineSpacing = 95;
		t7cY0free.WindowTab = 100;
		t7cY0free.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
		t7cY0free.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
		t7cY0free.WindowY0 = 90;
		t7cY0free.WindowY1 = 434 - 95;

		t7batt.Image = &t7screen;
		t7batt.WindowNumberOfTextLines = 1;
		t7batt.WindowLineSpacing = 10;
		t7batt.WindowTab = 100;
		t7batt.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
		t7batt.WindowX0 = t7batt.WindowX1 - (52+52);
		t7batt.WindowY1 = 479;
		t7batt.WindowY0 = t7batt.WindowY1 - 25;

		t7charge.Image = &t7screen;
		t7charge.WindowNumberOfTextLines = 1;
		t7charge.WindowLineSpacing = 10;
		t7charge.WindowTab = 100;
		t7charge.WindowX1 = t7batt.WindowX1 - 18;
		t7charge.WindowX0 = t7charge.WindowX1 - 14;
		t7charge.WindowY1 = 479;
		t7charge.WindowY0 = t7batt.WindowY1 - 25;

		t7voltage.Image = &t7screen;
		t7voltage.WindowNumberOfTextLines = 1;
		t7voltage.WindowLineSpacing = 10;
		t7voltage.WindowTab = 100;
		t7voltage.WindowX0 = t7charge.WindowX0 - 10;
		t7voltage.WindowX1 = t7voltage.WindowX0 + (18*3)+ 9;
		t7voltage.WindowY1 = 479;
		t7voltage.WindowY0 = t7batt.WindowY1 - 25;

		t7c1.Image = &t7screen;
		t7c1.WindowNumberOfTextLines = 1;
		t7c1.WindowLineSpacing = 10;
		t7c1.WindowTab = 100;
		t7c1.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
		t7c1.WindowX1 = t7batt.WindowX0 - 18;
		t7c1.WindowY0 = 435;
		t7c1.WindowY1 = 479;

		t7c2.Image = &t7screen;
		t7c2.WindowNumberOfTextLines = 1;
		t7c2.WindowLineSpacing = 0; // Abstand von Y0
		t7c2.WindowTab = 100;
		t7c2.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
		t7c2.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
		t7c2.WindowY0 = 0;
		t7c2.WindowY1 = 69;

		t7pCompass.Image = &t7screenCompass;
		t7pCompass.WindowNumberOfTextLines = 1;
		t7pCompass.WindowLineSpacing = 100; // Abstand von Y0
		t7pCompass.WindowTab = 100;
		t7pCompass.WindowX0 = 0;
		t7pCompass.WindowX1 = 1600-1;
		t7pCompass.WindowY0 = 0;
		t7pCompass.WindowY1 = 100-1;
    }
    else
    {
/* 6 segments (left / right) used to show data during dive */

    t7l1.Image = &t7screen;
    t7l1.WindowNumberOfTextLines = 2;
    t7l1.WindowLineSpacing = 19; // Abstand von Y0
    t7l1.WindowTab = 100; // vermtl. ohne Verwendung in diesem Fenster
    t7l1.WindowX0 = CUSTOMBOX_LINE_RIGHT + CUSTOMBOX_OUTSIDE_OFFSET;
    t7l1.WindowX1 = 799;
    t7l1.WindowY0 = CUSTOMBOX_LINE_TOP;
    t7l1.WindowY1 = 150 + TOP_LINE_HIGHT;

    t7l2.Image = &t7screen;
    t7l2.WindowNumberOfTextLines = 2;
    t7l2.WindowLineSpacing = 22; // Abstand von Y0
    t7l2.WindowTab = 100; // vermtl. ohne Verwendung in diesem Fenster
    t7l2.WindowX0 = t7l1.WindowX0;
    t7l2.WindowX1 = t7l1.WindowX1;
    t7l2.WindowY0 = t7l1.WindowY1 + 5;
    t7l2.WindowY1 = t7l2.WindowY0 + 146;

    t7l3.Image = &t7screen;
    t7l3.WindowNumberOfTextLines = 2;
    t7l3.WindowLineSpacing = 58; // Abstand von Y0
    t7l3.WindowTab = 100; // vermtl. ohne Verwendung in diesem Fenster
    t7l3.WindowX0 = t7l1.WindowX0;
    t7l3.WindowX1 = t7l1.WindowX1;;
    t7l3.WindowY0 = 479 - 150;
    t7l3.WindowY1 = 479;

    t7r1.Image = &t7screen;
    t7r1.WindowNumberOfTextLines = 2;
    t7r1.WindowLineSpacing = t7l1.WindowLineSpacing;
    t7r1.WindowTab = 100;
    t7r1.WindowX0 = 0;
    t7r1.WindowX1 = CUSTOMBOX_LINE_LEFT - CUSTOMBOX_OUTSIDE_OFFSET;
    t7r1.WindowY0 = t7l1.WindowY0;
    t7r1.WindowY1 = t7l1.WindowY1;

    t7r2.Image = &t7screen;
    t7r2.WindowNumberOfTextLines = 2;
    t7r2.WindowLineSpacing = t7l2.WindowLineSpacing;
    t7r2.WindowTab = 100;
    t7r2.WindowX0 = t7r1.WindowX0;
    t7r2.WindowX1 = t7r1.WindowX1;
    t7r2.WindowY0 = t7l2.WindowY0;
    t7r2.WindowY1 = t7l2.WindowY1;

    t7r3.Image = &t7screen;
    t7r3.WindowNumberOfTextLines = 2;
    t7r3.WindowLineSpacing = 0;//t7l3.WindowLineSpacing;
    t7r3.WindowTab = 100;
    t7r3.WindowX0 = t7r1.WindowX0;
    t7r3.WindowX1 = t7r1.WindowX1;
    t7r3.WindowY0 = t7l3.WindowY0;
    t7r3.WindowY1 = t7l3.WindowY1;

/* screen for CustomText / serial number */
    t7cC.Image = &t7screen;
    t7cC.WindowNumberOfTextLines = 3;
    t7cC.WindowLineSpacing = 95; // Abstand von Y0
    t7cC.WindowTab = 100;
    t7cC.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t7cC.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
    t7cC.WindowY0 = 165; //90;
    t7cC.WindowY1 = 415;

    /* used by warning message box */
    t7cH.Image = &t7screen;
    t7cH.WindowNumberOfTextLines = 1;
    t7cH.WindowLineSpacing = 95; // Abstand von Y0
    t7cH.WindowTab = 100;
    t7cH.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t7cH.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
    t7cH.WindowY0 = 46; //480 - 434;
    t7cH.WindowY1 = 390 - 46;// - 90; //46 + 390; //480 - (434 - 94); //434;

    /* used by warning custom box */
    t7cW.Image = &t7screen;
    t7cW.WindowNumberOfTextLines = 3;
    t7cW.WindowLineSpacing = 95; // Abstand von Y0
    t7cW.WindowTab = 100;
    t7cW.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t7cW.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
    t7cW.WindowY0 = 480 - (434 - 90);
    t7cW.WindowY1 = 480 - 90; //434 - 95;

/* time and environment */
    t7surfaceL.Image = &t7screen;
    t7surfaceL.WindowNumberOfTextLines = 9;
    t7surfaceL.WindowLineSpacing = 53;
    t7surfaceL.WindowTab = 100;
    t7surfaceL.WindowX0 = CUSTOMBOX_LINE_RIGHT + CUSTOMBOX_OUTSIDE_OFFSET;
    t7surfaceL.WindowX1 = 799;
    t7surfaceL.WindowY0 = 0;
    t7surfaceL.WindowY1 = 479;

    t7surfaceR.Image = &t7screen;
    t7surfaceR.WindowNumberOfTextLines = 9;
    t7surfaceR.WindowLineSpacing = 53;
    t7surfaceR.WindowTab = 100;
    t7surfaceR.WindowX0 = 0;
    t7surfaceR.WindowX1 = CUSTOMBOX_LINE_LEFT - CUSTOMBOX_OUTSIDE_OFFSET;
    t7surfaceR.WindowY0 = 0;
    t7surfaceR.WindowY1 = 479;

/* info screen in the middle */
    t7cY0free.Image = &t7screen;
    t7cY0free.WindowNumberOfTextLines = 1;
    t7cY0free.WindowLineSpacing = 95;
    t7cY0free.WindowTab = 100;
    t7cY0free.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t7cY0free.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
    t7cY0free.WindowY0 = 115;
    t7cY0free.WindowY1 = 365;

/* voltage value (V or %) */
    t7voltage.Image = &t7screen;
    t7voltage.WindowNumberOfTextLines = 1;
    t7voltage.WindowLineSpacing = 10;
    t7voltage.WindowTab = 100;
    t7voltage.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t7voltage.WindowX1 = t7voltage.WindowX0 + (18*3) +9;
    t7voltage.WindowY1 = TOP_LINE_HIGHT;
    t7voltage.WindowY0 = 0;

/* battery symbol */
    t7batt.Image = &t7screen;
    t7batt.WindowNumberOfTextLines = 1;
    t7batt.WindowLineSpacing = 10;
    t7batt.WindowTab = 100;
    t7batt.WindowX0 = t7voltage.WindowX1;
    t7batt.WindowX1 = t7batt.WindowX0 + (52);
    t7batt.WindowY1 = TOP_LINE_HIGHT;
    t7batt.WindowY0 = 0;

/* charger symbol */
    t7charge.Image = &t7screen;
    t7charge.WindowNumberOfTextLines = 1;
    t7charge.WindowLineSpacing = 10;
    t7charge.WindowTab = 100;
    t7charge.WindowX1 = t7batt.WindowX0 - 18;
    t7charge.WindowX0 = t7charge.WindowX1 - 14;

    t7charge.WindowY1 = TOP_LINE_HIGHT;
    t7charge.WindowY0 = 0;

/* show dive mode OC / CC */
    t7c1.Image = &t7screen;
    t7c1.WindowNumberOfTextLines = 1;
    t7c1.WindowLineSpacing = 10;
    t7c1.WindowTab = 100;
    t7c1.WindowX0 = t7batt.WindowX1 + 18; //CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t7c1.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET; //t7batt.WindowX1 + 18;
    t7c1.WindowY0 = 0;
    t7c1.WindowY1 = 479 - 435;

/* Gas warnings and exit Sim*/
    t7c2.Image = &t7screen;
    t7c2.WindowNumberOfTextLines = 1;
    t7c2.WindowLineSpacing = 0; // Abstand von Y0
    t7c2.WindowTab = 100;
    t7c2.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t7c2.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
    t7c2.WindowY0 = 480 - 69;
    t7c2.WindowY1 = 479;

/* Rotating compass */
    t7pCompass.Image = &t7screenCompass;
    t7pCompass.WindowNumberOfTextLines = 1;
    t7pCompass.WindowLineSpacing = 100; // Abstand von Y0
    t7pCompass.WindowTab = 100;
    t7pCompass.WindowX0 = 0;
    t7pCompass.WindowX1 = 1600-1;
    t7pCompass.WindowY0 = 479 - 75;
    t7pCompass.WindowY1 = 479;

    }

    init_t7_compass();
}


void t7_refresh_sleepmode_fun(void)
{
    uint32_t oldScreen;

    oldScreen = t7screen.FBStartAdress;
    t7screen.FBStartAdress = getFrame(22);

    t7_refresh_sleep_design_fun();

    if(get_globalState() == StStop)
    {
        GFX_SetFramesTopBottom(t7screen.FBStartAdress, 0,480);
    }
    releaseFrame(22,oldScreen);
}


void t7_refresh(void)
{
    static uint8_t last_mode = MODE_SURFACE;

//	uint32_t oldScreen;//, oldPlugin;
    SStateList status;
    get_globalStateList(&status);

    t7screen.FBStartAdress = getFrame(22);

    background.pointer = 0;

    if(stateUsed->mode == MODE_DIVE)
    {
        if(last_mode != MODE_DIVE)
        {
            last_mode = MODE_DIVE;
            /* lower left corner primary */
            selection_custom_field = settingsGetPointer()->tX_userselectedLeftLowerCornerPrimary;
            /* custom view primary OR debug if automatic return is off */
            if((settingsGetPointer()->tX_customViewTimeout == 0) && (settingsGetPointer()->showDebugInfo))
                selection_customview = CVIEW_noneOrDebug;
            else
                selection_customview = settingsGetPointer()->tX_customViewPrimary;
        }

        if(status.page == PageSurface)
            set_globalState(StD);

        if(stateUsed->diveSettings.diveMode == DIVEMODE_Gauge)
        {
            settingsGetPointer()->design = 5;
            releaseAllFramesExcept(22,t7screen.FBStartAdress);
            releaseFrame(22,t7screen.FBStartAdress);
            return;
        }
        else if(stateUsed->diveSettings.diveMode == DIVEMODE_Apnea)
        {
            settingsGetPointer()->design = 6;
            releaseAllFramesExcept(22,t7screen.FBStartAdress);
            releaseFrame(22,t7screen.FBStartAdress);
            return;
        }
        else
        {
            t7_refresh_divemode();
        }
    }
    else // from if(stateUsed->mode == MODE_DIVE)
    {
        if(last_mode != MODE_SURFACE)
        {
            last_mode = MODE_SURFACE;
            selection_customview = customviewsSurface[0];
        }
        if(status.page == PageDive)
            set_globalState(StS);

        if(settingsGetPointer()->showDebugInfo)
            t7_refresh_surface_debugmode();
        else
            t7_refresh_surface();
    }

    tHome_show_lost_connection_count(&t7screen);

    if(status.base == BaseHome)
    {
        if(background.pointer)
        {
            GFX_SetFrameTop(t7screen.FBStartAdress);
            GFX_SetFrameBottom(background.pointer,background.x0 , background.y0, background.width, background.height);
        }
        else
            GFX_SetFramesTopBottom(t7screen.FBStartAdress, 0,480);
    }

    releaseAllFramesExcept(22,t7screen.FBStartAdress);
}

/* Private functions ---------------------------------------------------------*/

void t7_fill_surfacetime_helper(SSurfacetime *outArray, uint32_t inputMinutes, uint32_t inputSeconds)
{
    inputSeconds += inputMinutes * 60;

    outArray->Total = inputSeconds;

    outArray->Days = inputSeconds / 86400;// (24*60*60);
    inputSeconds -= 86400 * (uint32_t)outArray->Days;

    outArray->Hours = inputSeconds / 3600;// (60*60);
    inputSeconds -= 3600 * (uint32_t)outArray->Hours;

    outArray->Minutes = inputSeconds / 60;;
    inputSeconds -= 60 * (uint32_t)outArray->Minutes;

    outArray->Seconds = inputSeconds;
}

void t7_refresh_sleep_design_fun(void)
{
    static uint16_t state = 0;
    uint16_t ytop = 0;

    state +=1;
    if(state > 800)
        state = 1;

    if(state > 400)
        ytop = 800 - state;
    else
        ytop = 0 + state;
    Gfx_write_label_var(&t7screen,  300,800, ytop,&FontT48,CLUT_Font020,"SLEEP SLEEP SLEEP");
}

void t7_refresh_surface(void)
{
    char text[256];
    uint8_t date[3], year,month,day;
    uint32_t color;
    uint8_t  customview_warnings = 0;

    RTC_DateTypeDef Sdate;
    RTC_TimeTypeDef Stime;
    RTC_DateTypeDef SdateFirmware;

    uint8_t dateNotSet = 0;

    uint8_t oxygen_percentage, gasOffset, actualGasID;
//	uint16_t bottleFirstGas_bar;
    point_t start, stop;//, other;

	SSettings* pSettings;
	pSettings = settingsGetPointer();


    // update in all customview modes
    if(DataEX_check_RTE_version__needs_update() || font_update_required())
        updateNecessary = 1;
    else
        updateNecessary = 0;

    /* buttons */
    text[0] = TXT_2BYTE;
    text[1] = TXT2BYTE_ButtonLogbook;
    text[2] = 0;
    write_content_simple(&t7screen, 0, 799, 479-TOP_LINE_HIGHT, &FontT24,text,CLUT_ButtonSurfaceScreen);

    text[0] = '\001';
    text[1] = TXT_2BYTE;
    text[2] = TXT2BYTE_ButtonView;
    text[3] = 0;
    write_content_simple(&t7screen, 0, 799, 479-TOP_LINE_HIGHT, &FontT24,text,CLUT_ButtonSurfaceScreen);

    text[0] = '\002';
    text[1] = TXT_2BYTE;
    text[2] = TXT2BYTE_ButtonMenu;
    text[3] = 0;
    write_content_simple(&t7screen, 0, 799, 479-TOP_LINE_HIGHT, &FontT24,text,CLUT_ButtonSurfaceScreen);

    /* was power on reset */
//.....
/* removed hw 160802 in V1.1.1
    if(errorsInSettings)
    {
        sprintf(text,"Settings: %u",errorsInSettings);
        GFX_write_string_color(&FontT42,&t7surfaceR,text,4,CLUT_WarningRed);
    }
    else
*/
    if(DataEX_was_power_on())
        GFX_write_string_color(&FontT42,&t7surfaceR,"cold start",4,CLUT_WarningRed);

    /* time and date */
    translateDate(stateUsed->lifeData.dateBinaryFormat, &Sdate);
    translateTime(stateUsed->lifeData.timeBinaryFormat, &Stime);

    firmwareGetDate(&SdateFirmware);
    if(tHome_DateCode(&Sdate) < tHome_DateCode(&SdateFirmware))
        dateNotSet = 1;
    else
        dateNotSet = 0;
/*
    if(Stime.Seconds % 2)
        snprintf(text,255,"\001%02d:%02d",Stime.Hours,Stime.Minutes);
    else
        snprintf(text,255,"\001%02d\021:\020%02d",Stime.Hours,Stime.Minutes);
    GFX_write_string(&FontT54,&t7surfaceR,text,3);
*/
// debug version:
    if(Stime.Seconds % 2)
        snprintf(text,255,"\001%02d:%02d:%02d",Stime.Hours,Stime.Minutes,Stime.Seconds);
    else if(dateNotSet)
        snprintf(text,255,"\001\021%02d:%02d:%02d\020",Stime.Hours,Stime.Minutes,Stime.Seconds);
    else
        snprintf(text,255,"\001%02d\021:\020%02d:%02d",Stime.Hours,Stime.Minutes,Stime.Seconds);
    GFX_write_string(&FontT54,&t7surfaceR,text,3);

    if(settingsGetPointer()->date_format == DDMMYY)
    {
        day = 0;
        month = 1;
        year = 2;
    }
    else
    if(settingsGetPointer()->date_format == MMDDYY)
    {
        day = 1;
        month = 0;
        year = 2;
    }
    else
    {
        day = 2;
        month = 1;
        year = 0;
    }
    date[day] = Sdate.Date;
    date[month] = Sdate.Month;
    date[year] = Sdate.Year;

    if((Stime.Seconds % 2) || (dateNotSet == 0))
        snprintf(text,255,"\001%02d.%02d.%02d",date[0],date[1],date[2]);
    else
        snprintf(text,255,"\001\021%02d.%02d.%02d",date[0],date[1],date[2]);

    GFX_write_string(&FontT54,&t7surfaceR,text,5);

    if(!DataEX_was_power_on() && !errorsInSettings)
    {
        text[0] = '\001';
        text[1] = '\004';
        text[2] = TXT_2BYTE;
        text[3] = TXT2BYTE_Sunday;
        text[4] = 0;
        if(Sdate.WeekDay != RTC_WEEKDAY_SUNDAY)
            text[3] += Sdate.WeekDay;

        if(!(Stime.Seconds % 2) && (dateNotSet == 1))
            text[1] = '\021';

        GFX_write_string(&FontT24,&t7surfaceR,text,4);
    }

    /* DEBUG uTick Pressure and Compass */
/*
    snprintf(text,255,"\001%u",stateRealGetPointer()->pressure_uTick_new - stateRealGetPointer()->pressure_uTick_old);
    GFX_write_string(&FontT42,&t7surfaceR,text,1);
    snprintf(text,255,"\001%u",HAL_GetTick() - stateRealGetPointer()->pressure_uTick_local_new);
    GFX_write_string(&FontT42,&t7surfaceR,text,2);

    snprintf(text,255,"\001%u",stateRealGetPointer()->compass_uTick_new - stateRealGetPointer()->compass_uTick_old);
    GFX_write_string(&FontT42,&t7surfaceR,text,6);
    snprintf(text,255,"\001%u",HAL_GetTick() - stateRealGetPointer()->compass_uTick_local_new);
    GFX_write_string(&FontT42,&t7surfaceR,text,7);

    static uint32_t bildschirmRefresh = 0;
    snprintf(text,255,"\001%u",HAL_GetTick() - bildschirmRefresh);
    GFX_write_string(&FontT42,&t7surfaceR,text,8);
    bildschirmRefresh = HAL_GetTick();

    static uint16_t bildschirmRefreshCount = 1;
    if(bildschirmRefreshCount>10)
        bildschirmRefreshCount = 1;
    for(int i=0;i<bildschirmRefreshCount;i++)
        text[i] = '.';
        text[bildschirmRefreshCount++] = 0;
    GFX_write_string(&FontT42,&t7surfaceR,text,4);
*/

    /* noFlyTime or DesaturationTime */

    if((!display_count_high_time) && (stateUsed->lifeData.no_fly_time_minutes))
    {
        SSurfacetime NoFlyTime = {0,0,0,0};
        t7_fill_surfacetime_helper(&NoFlyTime,stateUsed->lifeData.no_fly_time_minutes, 0);

        if(NoFlyTime.Days)
        {
            snprintf(text,30,"\001%02d\016\016d\017 %02d\016\016h\017",NoFlyTime.Days, NoFlyTime.Hours);
        }
        else
        {
            snprintf(text,20,"\001%02d:%02d",NoFlyTime.Hours, NoFlyTime.Minutes);
        }

        GFX_write_string(&FontT54,&t7surfaceR,text,7);

        text[0] = '\001';
        text[1] = '\022';
        text[2] = '\016';
        text[3] = '\016';
        text[4] = TXT_2BYTE;
        text[5] = TXT2BYTE_noFly;
        text[6] = 0;
        GFX_write_string(&FontT48,&t7surfaceR,text,6);
    }
    else
    if(stateUsed->lifeData.desaturation_time_minutes)
    {
        SSurfacetime DesatTime = {0,0,0,0};
        t7_fill_surfacetime_helper(&DesatTime,stateUsed->lifeData.desaturation_time_minutes, 0);

        if(DesatTime.Days)
        {
            snprintf(text,30,"\001%02d\016\016d\017 %02d\016\016h\017",DesatTime.Days, DesatTime.Hours);
        }
        else
        {
            snprintf(text,20,"\001%02d:%02d",DesatTime.Hours, DesatTime.Minutes);
        }
        GFX_write_string(&FontT54,&t7surfaceR,text,7);

            text[0] = '\001';
            text[1] = '\022';
            text[2] = '\016';
            text[3] = '\016';
            text[4] = TXT_2BYTE;
            text[5] = TXT2BYTE_Desaturation;
            text[6] = 0;
            GFX_write_string(&FontT48,&t7surfaceR,text,6);
    }

    /* Time since last dive */
    if(stateUsed->lifeData.surface_time_seconds)
    {
        SSurfacetime SurfTime = {0,0,0,0};
        t7_fill_surfacetime_helper(&SurfTime, 0, stateUsed->lifeData.surface_time_seconds);

        if(SurfTime.Days == 0)
        {
            snprintf(text,20,"\001\022%02d:%02d",SurfTime.Hours, SurfTime.Minutes);
        }
        else
        {
            snprintf(text,30,"\001\022%02d\016\016d\017 %02d\016\016h\017",SurfTime.Days, SurfTime.Hours);
        }

        GFX_write_string(&FontT54,&t7surfaceR,text,2);


        text[0] = '\001';
        text[1] = '\022';
        text[2] = '\016';
        text[3] = '\016';
        text[4] = TXT_2BYTE;
        text[5] = TXT2BYTE_TimeSinceLastDive;
        text[6] = 0;
        GFX_write_string(&FontT48,&t7surfaceR,text,1);
    }

    /* beta version */
    if( firmwareDataGetPointer()->versionBeta )
    {
        snprintf(text,255,"\025 BETA");
        GFX_write_string(&FontT48,&t7surfaceL,text,2);
    }

    /* surface pressure  and temperature */
    if(stateUsed->sensorErrorsRTE == 0)
    {
        snprintf(text,30,"%01.0f\022\016\016 %s", stateUsed->lifeData.pressure_surface_bar * 1000.0f,TEXT_PRESSURE_UNIT);
        GFX_write_string(&FontT48,&t7surfaceL,text,3);

        if(settingsGetPointer()->nonMetricalSystem)
            snprintf(text,40,"%01.0f\140\022\016\016 fahrenheit",unit_temperature_float(stateUsed->lifeData.temperature_celsius));
        else
            snprintf(text,30,"%01.0f\140\022\016\016 celsius",stateUsed->lifeData.temperature_celsius);
        GFX_write_string(&FontT48,&t7surfaceL,text,4);
    }
    else
    {
        snprintf(text,30,"ERR\022\016\016 %s",TEXT_PRESSURE_UNIT);
        GFX_write_string(&FontT48,&t7surfaceL,text,3);

        if(settingsGetPointer()->nonMetricalSystem)
            snprintf(text,40,"ERR\022\016\016 fahrenheit");
        else
            snprintf(text,30,"ERR\022\016\016 celsius");
        GFX_write_string(&FontT48,&t7surfaceL,text,4);
    }


    /* gas mix and selection */
    if((stateUsed->diveSettings.diveMode == DIVEMODE_Gauge) || (stateUsed->diveSettings.diveMode == DIVEMODE_Apnea))
    {
        if(stateUsed->diveSettings.diveMode == DIVEMODE_Gauge)
            text[0] = TXT_Gauge;
        else
            text[0] = TXT_Apnoe;

        text[1] = 0;
        GFX_write_string(&FontT48,&t7surfaceL,text,6);
    }
    else
    {
        text[0] = '\021';
        text[1] = '1';
        text[2] = '\177';
        text[3] = '\177';
        text[4] = 10;
        text[5] = '\021';
        text[6] = '2';
        text[7] = '\177';
        text[8] = '\177';
        text[9] = 10;
        text[10] = '\021';
        text[11] = '3';
        text[12] = '\177';
        text[13] = '\177';
        text[14] = 10;
        text[15] = '\021';
        text[16] = '4';
        text[17] = '\177';
        text[18] = '\177';
        text[19] = 10;
        text[20] = '\021';
        text[21] = '5';
        text[22] = 0;

        if(stateUsed->diveSettings.diveMode == DIVEMODE_CCR)
            gasOffset = NUM_OFFSET_DILUENT;
        else
            gasOffset = 0;
        for(int i=1;i<=5;i++)
        {
            if(stateUsed->diveSettings.gas[i+gasOffset].note.ub.active)
                text[(i-1)*5] -= 1;
        }
        GFX_write_string(&FontT48,&t7surfaceL,text,6);


        oxygen_percentage = 100;
        oxygen_percentage -= stateUsed->lifeData.actualGas.nitrogen_percentage;
        oxygen_percentage -= stateUsed->lifeData.actualGas.helium_percentage;

        tHome_gas_writer(oxygen_percentage,stateUsed->lifeData.actualGas.helium_percentage,&text[0]);
        GFX_write_string(&FontT48,&t7surfaceL,text,7);

        actualGasID = stateUsed->lifeData.actualGas.GasIdInSettings;
    /*
        bottleFirstGas_bar = stateUsed->lifeData.bottle_bar[actualGasID];
        if(bottleFirstGas_bar)
        {
            snprintf(text,255,"%3u\022\016\016 bar",bottleFirstGas_bar);
            GFX_write_string(&FontT48,&t7surfaceL,text,8);
        }
    */
        // after gas name :-)
        if(actualGasID > gasOffset) // security
        {
        	if(!pSettings->FlipDisplay)
        	{
        		start.y = t7surfaceL.WindowY0 + (3 * t7surfaceL.WindowLineSpacing);
        		start.x = t7surfaceL.WindowX0 + ((stateUsed->lifeData.actualGas.GasIdInSettings - gasOffset - 1) * 35);
        	}
			else
			{
				start.y = t7surfaceR.WindowY0 + (3 * t7surfaceR.WindowLineSpacing);
				start.x = t7surfaceR.WindowX0 + ((stateUsed->lifeData.actualGas.GasIdInSettings - gasOffset - 1) * 35);
			}

            stop.x = start.x + 25;
            stop.y = start.y + 52;
            GFX_draw_box2(&t7screen, start, stop, CLUT_Font020, 1);
        }
    }

    /* dive mode */
    if(stateUsed->diveSettings.diveMode == DIVEMODE_CCR)
        GFX_write_string(&FontT24,&t7c1,"\f\002" "CCR",0);
    else
        GFX_write_string(&FontT24,&t7c1,"\f\002" "OC",0);
//		GFX_write_string(&FontT24,&t7c1,"\f\177\177\x80" "CCR",0);

    /*battery */

    text[0] = '3';
    text[1] = '1';
    text[2] = '1';
    text[3] = '1';
    text[4] = '1';
    text[5] = '1';
    text[6] = '1';
    text[7] = '1';
    text[8] = '1';
    text[9] = '1';
    text[10] = '1';
    text[11] = '0';
    text[12] = 0;

    for(int i=1;i<=10;i++)
    {
        if(	stateUsed->lifeData.battery_charge > (9 * i))
            text[i] += 1;
    }

    if(stateUsed->chargeStatus == CHARGER_off)
    {
        if(stateUsed->warnings.lowBattery)
        {
            if(warning_count_high_time)
            {
                for(int i=1;i<=10;i++)
                    text[i] = '1';
            }
            else
            {
                text[1] = '2';
            }
            GFX_write_string_color(&Batt24,&t7batt,text,0,CLUT_WarningRed);
            if((stateUsed->lifeData.battery_charge > 0) && (stateUsed->lifeData.battery_charge < 140))
            {
                snprintf(text,16,"\004\025\f\002%u%%",(uint8_t)stateUsed->lifeData.battery_charge);
                if(warning_count_high_time)
                    text[0] = '\a';
                GFX_write_string(&FontT24,&t7voltage,text,0);
            }
            else
            {
                snprintf(text,6,"\f%.1fV",stateUsed->lifeData.battery_voltage);
                GFX_write_string(&FontT24,&t7voltage,text,0);
            }
        }
        else
        {
            GFX_write_string_color(&Batt24,&t7batt,text,0,CLUT_BatteryStandard);

            if((stateUsed->lifeData.battery_charge > 0) && (stateUsed->lifeData.battery_charge < 140))
            {
                snprintf(text,16,"\f\002%u%%",(uint8_t)stateUsed->lifeData.battery_charge);
        //        GFX_write_string(&FontT24,&t7batt,text,0);
                GFX_write_string(&FontT24,&t7voltage,text,0);
            }
            else
            {
                snprintf(text,6,"\f%.1fV",stateUsed->lifeData.battery_voltage);
                GFX_write_string(&FontT24,&t7voltage,text,0);
            }
        }
    }
    else
    {
        GFX_write_string_color(&Batt24,&t7batt,text,0,CLUT_BatteryCharging);

        switch(stateUsed->chargeStatus)
        {
        case CHARGER_running:
        default:
            color = CLUT_BatteryStandard;
            break;
        case CHARGER_complete:
            color = CLUT_BatteryCharging;
            break;
        case CHARGER_lostConnection:
            color = CLUT_BatteryProblem;
            break;
        }
        text[0] = '4';
        text[1] = 0;
        GFX_write_string_color(&Batt24,&t7charge,text,0,color);
    }



    customview_warnings = t7_test_customview_warnings_surface_mode();
    if(customview_warnings && warning_count_high_time)
        t7_show_customview_warnings_surface_mode();
    else
        t7_refresh_customview();
    draw_frame(0,0, CLUT_pluginboxSurface, CLUT_Font020);
}


void t7_refresh_surface_debugmode_wireless_info(void)
{
    char text[400];
    uint8_t colorDataLost = 0;
    int txtPointer = 0;
    uint8_t numberOfBytes = 0;

    GFX_DrawCfgWindow textWindow =
    {
        .Image = &t7screen,
        .WindowNumberOfTextLines = 5,
        .WindowLineSpacing = 70,
        .WindowTab = 220,
        .WindowX0 = 10,
        .WindowX1 = 790,
        .WindowY0 = 10,
        .WindowY1 = 380
    };

    Gfx_write_label_var(&t7screen,  10,600,  10,&FontT42,CLUT_DiveMainLabel,"Wireless Data");

    if(stateUsed->data_old__lost_connection_to_slave)
    {
        Gfx_write_label_var(&t7screen, 600,800,10,&FontT42,CLUT_Font020,"CPU2?");
        colorDataLost = 1;
    }

    txtPointer = 0;
    for(int i=0;i<4;i++)
    {
        if((!stateUsed->lifeData.wireless_data[i].ageInMilliSeconds) || colorDataLost)
            text[txtPointer++] = '\021';

        numberOfBytes = stateUsed->lifeData.wireless_data[i].numberOfBytes;
        if((numberOfBytes > 0) && (numberOfBytes <= 10))
        {
            txtPointer += snprintf(&text[txtPointer],20,"%02u s  %02u\t"
                ,(stateUsed->lifeData.wireless_data[i].ageInMilliSeconds)/1000
                ,stateUsed->lifeData.wireless_data[i].status
            );
            if(numberOfBytes > 8) ///< lifeData.wireless_data[i].data[j] has only size of 8
                numberOfBytes = 8;
            for(int j=0;j<numberOfBytes;j++)
            {
                txtPointer += snprintf(&text[txtPointer],4," %02X"
                    ,stateUsed->lifeData.wireless_data[i].data[j]
                );
            }
        }
        text[txtPointer++] = '\n';
        text[txtPointer++] = '\r';
        text[txtPointer++] = '\020';
        text[txtPointer] = 0;
    }
    GFX_write_string(&FontT48,&textWindow,text,1);

}


void t7_refresh_surface_debugmode(void)
{
    if(selection_customview%2 == 1)
    {
        t7_refresh_surface_debugmode_wireless_info();
        return;
    }

    // could be warning, now just to set RTE variables
    DataEX_check_RTE_version__needs_update();


    char TextL1[4*TEXTSIZE];
    uint32_t color;
//	uint8_t gasIdFirst;
    SSettings* pSettings = settingsGetPointer();
    extern SDataExchangeSlaveToMaster dataIn;

    SWindowGimpStyle windowGimp;

    RTC_DateTypeDef Sdate;
    RTC_TimeTypeDef Stime;

    translateDate(stateUsed->lifeData.dateBinaryFormat, &Sdate);
    translateTime(stateUsed->lifeData.timeBinaryFormat, &Stime);


    if(stateUsed->data_old__lost_connection_to_slave)
    {
        Gfx_write_label_var(&t7screen,  500,800,  0,&FontT42,CLUT_DiveMainLabel,"old");
        snprintf(TextL1,TEXTSIZE,"%X %X %X %X",dataIn.header.checkCode[0],dataIn.header.checkCode[1],dataIn.header.checkCode[2],dataIn.header.checkCode[3]);
        Gfx_write_label_var(&t7screen,  500,800, 45,&FontT48,CLUT_Font020,TextL1);
    }
    else
    if(DataEX_lost_connection_count())
    {
        snprintf(TextL1,TEXTSIZE,"\002%i",DataEX_lost_connection_count());
        Gfx_write_label_var(&t7screen,  600,800, 45,&FontT48,CLUT_Font020,TextL1);
    }

    snprintf(TextL1,TEXTSIZE,"\002%i",blockedFramesCount());
    Gfx_write_label_var(&t7screen,  600,800, 0,&FontT48,CLUT_Font020,TextL1);

    if(stateUsed->lifeData.compass_DX_f | stateUsed->lifeData.compass_DY_f | stateUsed->lifeData.compass_DZ_f)
    {
        snprintf(TextL1,TEXTSIZE,"X %i",stateUsed->lifeData.compass_DX_f);
        Gfx_write_label_var(&t7screen,  0,400, 45,&FontT48,CLUT_Font020,TextL1);
        snprintf(TextL1,TEXTSIZE,"Y %i",stateUsed->lifeData.compass_DY_f);
        Gfx_write_label_var(&t7screen,  0,400,145,&FontT48,CLUT_Font020,TextL1);
        snprintf(TextL1,TEXTSIZE,"Z %i",stateUsed->lifeData.compass_DZ_f);
        Gfx_write_label_var(&t7screen,  0,400,255,&FontT48,CLUT_Font020,TextL1);
        return;
    }
    snprintf(TextL1,TEXTSIZE,"%01.0f %s",stateUsed->lifeData.pressure_ambient_bar * 1000.0f,TEXT_PRESSURE_UNIT);
    Gfx_write_label_var(&t7screen,  0,400,  0,&FontT42,CLUT_DiveMainLabel,"Ambient Pressure");
    Gfx_write_label_var(&t7screen,  0,400, 45,&FontT48,CLUT_Font020,TextL1);

    snprintf(TextL1,TEXTSIZE,"%01.2f C",stateUsed->lifeData.temperature_celsius);
    Gfx_write_label_var(&t7screen,  0,400,100,&FontT42,CLUT_DiveMainLabel,"Temperature");
    Gfx_write_label_var(&t7screen,  0,400,145,&FontT48,CLUT_Font020,TextL1);

    snprintf(TextL1,TEXTSIZE,"%03.0f %03.0f %03.0f",stateUsed->lifeData.compass_heading,stateUsed->lifeData.compass_roll,stateUsed->lifeData.compass_pitch);
    Gfx_write_label_var(&t7screen,  0,400,200,&FontT42,CLUT_DiveMainLabel,"Heading Roll Pitch");
    Gfx_write_label_var(&t7screen,  0,400,255,&FontT48,CLUT_Font020,TextL1);

    snprintf(TextL1,TEXTSIZE,"%01.0f %s",stateUsed->lifeData.pressure_surface_bar * 1000.0f,TEXT_PRESSURE_UNIT);
    Gfx_write_label_var(&t7screen,  0,400,310,&FontT42,CLUT_DiveMainLabel,"Surface Pressure");
    Gfx_write_label_var(&t7screen,  0,400,355,&FontT48,CLUT_Font020,TextL1);

//	gasIdFirst = stateUsed->lifeData.actualGas.GasIdInSettings;
    snprintf(TextL1,TEXTSIZE,"%u.%u",dataIn.RTE_VERSION_high,dataIn.RTE_VERSION_low);
    Gfx_write_label_var(&t7screen,  320,500,100,&FontT42,CLUT_DiveMainLabel,"RTE");
    Gfx_write_label_var(&t7screen,  320,500,145,&FontT48,CLUT_Font020,TextL1);

    Gfx_write_label_var(&t7screen,  500,800,100,&FontT42,CLUT_DiveMainLabel,"Battery");
    snprintf(TextL1,TEXTSIZE,"%01.4f V",stateUsed->lifeData.battery_voltage);
    Gfx_write_label_var(&t7screen,  500,800,145,&FontT48,CLUT_Font020,TextL1);
    snprintf(TextL1,TEXTSIZE,"%03.1f %%",stateUsed->lifeData.battery_charge);
    Gfx_write_label_var(&t7screen,  500,800,200,&FontT48,CLUT_Font020,TextL1);
    if(stateUsed->chargeStatus != CHARGER_off)
    {
        switch(stateUsed->chargeStatus)
        {
        case CHARGER_running:
        default:
            color = CLUT_BatteryStandard;
            break;
        case CHARGER_complete:
            color = CLUT_BatteryCharging;
            break;
        case CHARGER_lostConnection:
            color = CLUT_BatteryProblem;
            break;
        }
        TextL1[0] = '4';
        TextL1[1] = 0;
        Gfx_write_label_var(&t7screen,  660,800,200,&Batt24,color,TextL1);
    }

extern uint32_t base_tempLightLevel;

    snprintf(TextL1,TEXTSIZE,"# %u (%u)",stateUsed->lifeData.ambient_light_level, base_tempLightLevel);
    Gfx_write_label_var(&t7screen,  401,600,310,&FontT42,CLUT_DiveMainLabel,"Light");
    Gfx_write_label_var(&t7screen,  401,800,355,&FontT48,CLUT_Font020,TextL1);

//	snprintf(TextL1,TEXTSIZE,"# %u",stateUsed->lifeData.ambient_light_level);
//	Gfx_write_label_var(&t7screen,  601,800,310,&FontT42,CLUT_DiveMainLabel,"Light");
//	Gfx_write_label_var(&t7screen,  601,800,355,&FontT48,CLUT_Font020,TextL1);



    if(Sdate.Year < 15)
    {
        if(warning_count_high_time)
        {
            snprintf(TextL1,4*TEXTSIZE,"\017 %02d-%02d-%02d  %02d:%02d:%02d", Sdate.Date, Sdate.Month, 2000 + Sdate.Year,Stime.Hours, Stime.Minutes, Stime.Seconds);
            Gfx_write_label_var(&t7screen,  0,800,420,&FontT48,CLUT_Font020,TextL1);
        }
    }
    else
    {
        if(pSettings->customtext[0])
        {
            if(pSettings->customtext[59])
                pSettings->customtext[59] = 0;
            Gfx_write_label_var(&t7screen,  0,400,420,&FontT24,CLUT_Font020,pSettings->customtext);
        }
        else
        {
            snprintf(TextL1,4*TEXTSIZE,"\017 %02d-%02d-%02d  %02d:%02d:%02d  Dives: %u", Sdate.Date, Sdate.Month, 2000 + Sdate.Year,Stime.Hours, Stime.Minutes, Stime.Seconds,pSettings->totalDiveCounter			);
            Gfx_write_label_var(&t7screen,  0,800,420,&FontT48,CLUT_Font020,TextL1);
        }
    }

    windowGimp.left = 400;
    windowGimp.top = 0;
    GFX_draw_image_monochrome(&t7screen, windowGimp, &ImgOSTC, 0);
}

/* CUSTOMVIEW
 * in the middle of the screen
 */

uint8_t t7_test_customview_warnings(void)
{
    uint8_t count = 0;

    count = 0;
    count += stateUsed->warnings.decoMissed;
    count += stateUsed->warnings.ppO2Low;
    count += stateUsed->warnings.ppO2High;
    //count += stateUsed->warnings.lowBattery;
    count += stateUsed->warnings.sensorLinkLost;
    count += stateUsed->warnings.fallback;
    return count;
}


uint8_t t7_test_customview_warnings_surface_mode(void)
{
    uint8_t count = 0;
    count = 0;
    count += stateUsed->cnsHigh_at_the_end_of_dive;
    count += stateUsed->decoMissed_at_the_end_of_dive;
    return count;
}


void t7_show_customview_warnings_surface_mode(void)
{
    char text[256];
    uint8_t textpointer, lineFree;

    text[0] = '\025';
    text[1] = '\f';
    text[2] = '\001';
    text[3] = TXT_Warning;
    text[4] = 0;
    GFX_write_string(&FontT42,&t7cH,text,0);

    textpointer = 0;
    lineFree = 5;

    if(stateUsed->decoMissed_at_the_end_of_dive)
    {
        text[textpointer++] = TXT_2BYTE;
        text[textpointer++] = TXT2BYTE_WarnDecoMissed;
        text[textpointer++] = '\n';
        text[textpointer++] = '\r';
        text[textpointer] = 0;
        lineFree--;
    }

    if(stateUsed->cnsHigh_at_the_end_of_dive)
    {
        text[textpointer++] = TXT_2BYTE;
        text[textpointer++] = TXT2BYTE_WarnCnsHigh;
        text[textpointer++] = '\n';
        text[textpointer++] = '\r';
        text[textpointer] = 0;
        lineFree--;
    }
    if(textpointer != 0)
        GFX_write_string(&FontT48,&t7cW,text,1);
}


void t7_show_customview_warnings(void)
{
    char text[256];
    uint8_t textpointer, lineFree;

    text[0] = '\025';
    text[1] = '\f';
    text[2] = '\001';
    text[3] = TXT_Warning;
    text[4] = 0;
    GFX_write_string(&FontT42,&t7cH,text,0);

    textpointer = 0;
    lineFree = 5;

    if(lineFree && stateUsed->warnings.decoMissed)
    {
        text[textpointer++] = TXT_2BYTE;
        text[textpointer++] = TXT2BYTE_WarnDecoMissed;
        text[textpointer++] = '\n';
        text[textpointer++] = '\r';
        text[textpointer] = 0;
        lineFree--;
    }

    if(lineFree && stateUsed->warnings.fallback)
    {
        text[textpointer++] = TXT_2BYTE;
        text[textpointer++] = TXT2BYTE_WarnFallback;
        text[textpointer++] = '\n';
        text[textpointer++] = '\r';
        text[textpointer] = 0;
        lineFree--;
    }

    if(lineFree && stateUsed->warnings.ppO2Low)
    {
        text[textpointer++] = TXT_2BYTE;
        text[textpointer++] = TXT2BYTE_WarnPPO2Low;
        text[textpointer++] = '\n';
        text[textpointer++] = '\r';
        text[textpointer] = 0;
        lineFree--;
    }

    if(lineFree && stateUsed->warnings.ppO2High)
    {
        text[textpointer++] = TXT_2BYTE;
        text[textpointer++] = TXT2BYTE_WarnPPO2High;
        text[textpointer++] = '\n';
        text[textpointer++] = '\r';
        text[textpointer] = 0;
        lineFree--;
    }

    if(lineFree && stateUsed->warnings.sensorLinkLost)
    {
        text[textpointer++] = TXT_2BYTE;
        text[textpointer++] = TXT2BYTE_WarnSensorLinkLost;
        text[textpointer++] = '\n';
        text[textpointer++] = '\r';
        text[textpointer] = 0;
        lineFree--;
    }
/*
    if(lineFree && stateUsed->warnings.lowBattery)
    {
        text[textpointer++] = TXT_2BYTE;
        text[textpointer++] = TXT2BYTE_WarnBatteryLow;
        text[textpointer++] = '\n';
        text[textpointer++] = '\r';
        text[textpointer] = 0;
        lineFree--;
    }
*/
    GFX_write_string(&FontT48,&t7cW,text,1);
}


void t7_set_customview_to_primary(void)
{
    if(stateUsed->mode == MODE_DIVE)
            selection_customview = settingsGetPointer()->tX_customViewPrimary;
}


// for CVIEW_END is none_or_debug
void t7_change_customview(void)
{
    const uint8_t *pViews;
    _Bool cv_disabled = 0;

    if(stateUsed->mode == MODE_DIVE)
        pViews = customviewsDive;
    else
        pViews = customviewsSurface;

    while((*pViews != CVIEW_END) && (*pViews != selection_customview))
        {pViews++;}

    if(*pViews < CVIEW_END)
        pViews++;


    if(*pViews == CVIEW_END)
    {
        if(stateUsed->mode == MODE_DIVE)
            pViews = customviewsDive;
        else
            pViews = customviewsSurface;
    }

    if(stateUsed->mode == MODE_DIVE)
    {
        do
        {
            cv_disabled = 0;
            for(int i=0;i<6;i++)
            {
                if((*pViews == cv_changelist[i]) && !CHECK_BIT_THOME(cv_configuration, cv_changelist[i]))
                {
                    cv_disabled = 1;
                    break;
                }
            }
            if(cv_disabled)
            {
                if(*pViews < CVIEW_END)
                {
                    pViews++;
                }
                else
                {
                    pViews = customviewsDive;
                }
            }
        } while(cv_disabled);
    }
    selection_customview = *pViews;
}


uint8_t t7_get_length_of_customtext(void)
{
    uint8_t i = 0;
    settingsGetPointer()->customtext[60-1] = 0;
    while(settingsGetPointer()->customtext[i] > 0)
        i++;
    return i;
}


void t7_refresh_customview(void)
{

    char text[256];
    uint16_t textpointer = 0;
    int16_t start;
    uint8_t lineCountCustomtext = 0;
    int16_t shiftWindowY0;
    RTC_DateTypeDef Sdate;
    RTC_TimeTypeDef Stime;
    float fPpO2limitHigh, fPpO2limitLow, fPpO2ofGasAtThisDepth; // CVIEW_Gaslist
    const SGasLine * pGasLine; // CVIEW_Gaslist
    uint8_t oxygen, helium; // CVIEW_Gaslist
    float depth, surface, fraction_nitrogen, fraction_helium, ead, end; // CVIEW_EADTime

	SSettings* pSettings;
	pSettings = settingsGetPointer();

    if((selection_customview == CVIEW_sensors) &&(stateUsed->diveSettings.ccrOption == 0))
        t7_change_customview();
    if((selection_customview == CVIEW_sensors_mV) &&(stateUsed->diveSettings.ccrOption == 0))
        t7_change_customview();
    if((selection_customview == CVIEW_sensors) &&(stateUsed->diveSettings.ccrOption == 0))
        t7_change_customview();

    switch(selection_customview)
    {
    case CVIEW_noneOrDebug:
        if(settingsGetPointer()->showDebugInfo)
        {
            // header
            strcpy(text,"\032\f\001Debug");
            GFX_write_string(&FontT42,&t7cH,text,0);
            // content
            t7_debug();
        }
        break;

    case CVIEW_SummaryOfLeftCorner:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_Summary);
        GFX_write_string(&FontT42,&t7cH,text,0);
        // content
        t7_SummaryOfLeftCorner();
        break;

    case CVIEW_CompassDebug:
        snprintf(text,100,"\032\f\001Compass raw");
        GFX_write_string(&FontT42,&t7cH,text,0);
/*
        pStateReal->lifeData.compass_heading = dataIn.data[dataIn.boolCompassData].compass_heading;
        pStateReal->lifeData.compass_roll = dataIn.data[dataIn.boolCompassData].compass_roll;
        pStateReal->lifeData.compass_pitch = dataIn.data[dataIn.boolCompassData].compass_pitch;

        pStateReal->lifeData.compass_DX_f = dataIn.data[dataIn.boolCompassData].compass_DX_f;
        pStateReal->lifeData.compass_DY_f = dataIn.data[dataIn.boolCompassData].compass_DY_f;
        pStateReal->lifeData.compass_DZ_f = dataIn.data[dataIn.boolCompassData].compass_DZ_f;
*/
        snprintf(text,255,"%1.1f\n\r%1.1f\n\r%1.1f\n\r%i\n\r%i\n\r%i"
                    ,stateUsed->lifeData.compass_heading
                    ,stateUsed->lifeData.compass_roll
                    ,stateUsed->lifeData.compass_pitch
                    ,stateUsed->lifeData.compass_DX_f
                    ,stateUsed->lifeData.compass_DY_f
                    ,stateUsed->lifeData.compass_DZ_f
            );

        t7cY0free.WindowY0 = t7cC.WindowY0 - 10;
        t7cY0free.WindowLineSpacing = 48;
        t7cY0free.WindowNumberOfTextLines = 6;
        GFX_write_string(&FontT42, &t7cY0free, text, 1);
        break;

    case CVIEW_Hello:
        t7_logo_OSTC();
        t7cC.WindowLineSpacing = 53;
        t7cC.WindowNumberOfTextLines = 5;
        shiftWindowY0 = 18;

        if(updateNecessary)//if(DataEX_check_RTE_version__needs_update() || font_update_required())
        {
            if(warning_count_high_time)
            {
                shiftWindowY0 += 20;
                t7cC.WindowY0 -= shiftWindowY0;
                textpointer = 0;
                text[textpointer++] = TXT_2BYTE;
                text[textpointer++] = TXT2BYTE_PleaseUpdate;
                text[textpointer++] = '\n';
                text[textpointer++] = '\r';
                if(DataEX_check_RTE_version__needs_update())
                {
                    text[textpointer++] = TXT_2BYTE;
                    text[textpointer++] = TXT2BYTE_RTE;
                    text[textpointer++] = '\n';
                    text[textpointer++] = '\r';
                }
                if(font_update_required())
                {
                    text[textpointer++] = TXT_2BYTE;
                    text[textpointer++] = TXT2BYTE_Fonts;
                }
                text[textpointer++] = 0;
                GFX_write_string_color(&FontT42,&t7cC,text,1, CLUT_WarningRed);
                t7cC.WindowY0 += shiftWindowY0;
            }
            t7cC.WindowNumberOfTextLines = 3;
        }
        else // customtext
        {
            lineCountCustomtext = t7_customtextPrepare(text);
            if(lineCountCustomtext <= 2)
                shiftWindowY0 += 20+26; // nach unten
            else
            if(lineCountCustomtext <= 3)
                shiftWindowY0 += 20; // nach unten
            t7cC.WindowY0 -= shiftWindowY0;

            GFX_write_string(&FontT42,&t7cC,text,1);
            t7cC.WindowNumberOfTextLines = 3;
            t7cC.WindowY0 += shiftWindowY0;
        }
        if(lineCountCustomtext <= 4)
        {
            snprintf(text,100,"\001#%0u V%01u.%01u.%01u",
            settingsGetPointer()->serialLow + (256 * settingsGetPointer()->serialHigh),
            firmwareDataGetPointer()->versionFirst,
            firmwareDataGetPointer()->versionSecond,
            firmwareDataGetPointer()->versionThird
            );
            GFX_write_string(&FontT24,&t7cC,text,0);
        }
        break;

    case CVIEW_Gaslist:
        // a lot of code taken from tMenuGas.c
        // header
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_Gaslist);
        GFX_write_string(&FontT42,&t7cH,text,0);
        // content
        textpointer = 0;

        if(!pSettings->FlipDisplay)
        {
        	t7cY0free.WindowY0 = t7cC.WindowY0 - 10;
        }
        else
        {
        	t7cY0free.WindowY1 = 400;
        }
        t7cY0free.WindowLineSpacing = 48+9;
        t7cY0free.WindowNumberOfTextLines = 5; // NUM_GASES == 5
        t7cY0free.WindowTab = 420;

        pGasLine = settingsGetPointer()->gas;
        if(actualLeftMaxDepth(stateUsed))
            fPpO2limitHigh = (float)(settingsGetPointer()->ppO2_max_deco) / 100;
        else
            fPpO2limitHigh = (float)(settingsGetPointer()->ppO2_max_std) / 100;
        fPpO2limitLow = (float)(settingsGetPointer()->ppO2_min) / 100;
        for(int gasId=1;gasId<=NUM_GASES;gasId++)
        {
            textpointer = 0;
            fPpO2ofGasAtThisDepth = (stateUsed->lifeData.pressure_ambient_bar - WATER_VAPOUR_PRESSURE) * pGasLine[gasId].oxygen_percentage / 100;
            if(pGasLine[gasId].note.ub.active == 0)
                strcpy(&text[textpointer++],"\021");
            else if((fPpO2ofGasAtThisDepth > fPpO2limitHigh) || (fPpO2ofGasAtThisDepth < fPpO2limitLow))
                strcpy(&text[textpointer++],"\025");
            else
                strcpy(&text[textpointer++],"\030");

            text[textpointer++] = ' ';
            oxygen = pGasLine[gasId].oxygen_percentage;
            helium = pGasLine[gasId].helium_percentage;
            textpointer += write_gas(&text[textpointer], oxygen, helium);
            // Wechseltiefe
            if(pGasLine[gasId].depth_meter)
            {
                textpointer += snprintf(&text[textpointer],7,"\t%u %c%c",unit_depth_integer(pGasLine[gasId].depth_meter), unit_depth_char1(), unit_depth_char2());
            }
            GFX_write_string(&FontT42, &t7cY0free, text, gasId);
        }
        break;

    case CVIEW_EADTime:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_Info );
        GFX_write_string(&FontT42,&t7cH,text,0);
        textpointer = 0;

        t7cY0free.WindowY0 = t7cC.WindowY0 - 10;
        if(pSettings->FlipDisplay)
        {
        	t7cY0free.WindowY1 = 400;
        }
        t7cY0free.WindowLineSpacing = 48;
        t7cY0free.WindowNumberOfTextLines = 6;

    // time
        snprintf(text,100,"\032\001%c%c",TXT_2BYTE,TXT2BYTE_Clock );
        GFX_write_string(&FontT42, &t7cY0free, text, 1);

        translateDate(stateRealGetPointer()->lifeData.dateBinaryFormat, &Sdate);
        translateTime(stateRealGetPointer()->lifeData.timeBinaryFormat, &Stime);
        if(Stime.Seconds % 2)
            textpointer += snprintf(&text[textpointer],100,"\030\001%02d:%02d",Stime.Hours,Stime.Minutes);
        else
            textpointer += snprintf(&text[textpointer],100,"\030\001%02d\031:\030%02d",Stime.Hours,Stime.Minutes);
        GFX_write_string(&FontT42, &t7cY0free, text, 2);

        // EAD / END
        // The equivalent air depth can be calculated for depths in metres as follows:
        // EAD = (Depth + 10) ï¿½ Fraction of N2 / 0.79 - 10   (wikipedia)
        // The equivalent narcotic depth can be calculated for depths in metres as follows:
        // END = (Depth + 10) ï¿½ (1 - Fraction of helium) - 10  (wikipedia)
        decom_get_inert_gases((float)stateUsed->lifeData.pressure_ambient_bar,&(stateUsed->lifeData.actualGas),&fraction_nitrogen,&fraction_helium);
        depth = stateUsed->lifeData.pressure_ambient_bar;
        surface = stateUsed->lifeData.pressure_surface_bar;
        ead = 10.f * ((depth * fraction_nitrogen/0.79f) - surface);
        end = 10.0f * ((depth * (1.f - fraction_helium)) - surface);
        if(ead < 0)
            ead = 0;
        if(end < 0)
            end = 0;

        snprintf(text,100,"\032\001EAD");
        GFX_write_string(&FontT42, &t7cY0free, text, 3);
        snprintf(text,100,"\030\001%01.1f %c%c"
        , unit_depth_float(ead)
        , unit_depth_char1()
        , unit_depth_char2()
        );
        GFX_write_string(&FontT42, &t7cY0free, text, 4);

        snprintf(text,100,"\032\001END");
        GFX_write_string(&FontT42, &t7cY0free, text, 5);
        snprintf(text,100,"\030\001%01.1f %c%c"
        , unit_depth_float(ead)
        , unit_depth_char1()
        , unit_depth_char2()
        );
        GFX_write_string(&FontT42, &t7cY0free, text, 6);
        break;

    case CVIEW_Profile:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_Profile);
        GFX_write_string(&FontT42,&t7cH,text,0);
        textpointer = 0;
        t7_miniLiveLogProfile();
        break;

    case CVIEW_Tissues:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_Tissues);
        GFX_write_string(&FontT42,&t7cH,text,0);
        textpointer = 0;
        t7_tissues(stateUsed);
        break;

    case CVIEW_sensors:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_O2monitor);
        GFX_write_string(&FontT42,&t7cH,text,0);
        textpointer = 0;
        text[textpointer++] = '\030'; // main color
        for(int i=0;i<3;i++)
        {
            if(stateUsed->diveSettings.ppo2sensors_deactivated & (1<<i))
            {
                text[textpointer++] = '\031'; // labelcolor
                text[textpointer++] = '\001';
                text[textpointer++] = '-';
                text[textpointer++] = '\n';
                text[textpointer++] = '\r';
                text[textpointer++] = '\030'; // main color
                text[textpointer] = 0;
            }
            else
            {
                if(stateUsed->warnings.sensorOutOfBounds[i])
                    text[textpointer++] = '\025'; // Warning Red
                textpointer += snprintf(&text[textpointer],100,"\001%01.2f\n\r\030",stateUsed->lifeData.ppO2Sensor_bar[i]);
            }
        }
        t7cC.WindowLineSpacing = 95;
        t7cC.WindowNumberOfTextLines = 3;
        text[textpointer] = 0;
        GFX_write_string(&FontT105,&t7cC,text,1);
        break;

    case CVIEW_sensors_mV:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_O2voltage);
        GFX_write_string(&FontT42,&t7cH,text,0);
        textpointer = 0;
        text[textpointer++] = '\030';
        for(int i=0;i<3;i++)
        {
            if(stateUsed->diveSettings.ppo2sensors_deactivated & (1<<i))
            {
                text[textpointer++] = '\031';
                text[textpointer++] = '\001';
                text[textpointer++] = '-';
                text[textpointer++] = '\n';
                text[textpointer++] = '\r';
                text[textpointer++] = '\030';
                text[textpointer] = 0;
            }
            else
            {
                if(stateUsed->warnings.sensorOutOfBounds[i])
                    text[textpointer++] = '\025';
                textpointer += snprintf(&text[textpointer],100,"\001%01.1f mV\n\r\030",(stateUsed->lifeData.sensorVoltage_mV[i]));
            }
        }
        t7cC.WindowLineSpacing = 95;
        t7cC.WindowNumberOfTextLines = 3;
        text[textpointer] = 0;
        GFX_write_string(&FontT48,&t7cC,text,1);
        break;

    case CVIEW_Compass:
    default:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE, TXT2BYTE_Compass);
        GFX_write_string(&FontT42,&t7cH,text,0);
        t7_compass((uint16_t)stateUsed->lifeData.compass_heading, stateUsed->diveSettings.compassHeading);

        if(!pSettings->FlipDisplay)
        {
        	t7cY0free.WindowX0 += 15;
        	t7cY0free.WindowY0 = 230;
        }
        else
        {
        	t7cY0free.WindowX0 -= 15;
        	t7cY0free.WindowY0 = 0;
        	t7cY0free.WindowY1 = 250;
        }
        snprintf(text,100,"\030\001%03i`",(uint16_t)stateUsed->lifeData.compass_heading);
        GFX_write_string(&FontT54,&t7cY0free,text,0);
        if(!pSettings->FlipDisplay)
        {
        	t7cY0free.WindowX0 -= 15;
        }
        else
        {
        	t7cY0free.WindowX0 += 15;
        }
        break;

    case CVIEW_Decolist:
        snprintf(text,100,"\032\f\001 %c%c", TXT_2BYTE, TXT2BYTE_Decolist);
        GFX_write_string(&FontT42,&t7cH,text,0);

        const SDecoinfo * pDecoinfo;
        uint8_t depthNext, depthLast, depthSecond, depthInc;

        if(stateUsed->diveSettings.deco_type.ub.standard == GF_MODE)
            pDecoinfo = &stateUsed->decolistBuehlmann;
        else
            pDecoinfo = &stateUsed->decolistVPM;

        depthLast 		= (uint8_t)(stateUsed->diveSettings.last_stop_depth_bar * 10);
        depthSecond 	= (uint8_t)(stateUsed->diveSettings.input_second_to_last_stop_depth_bar * 10);
        depthInc 			= (uint8_t)(stateUsed->diveSettings.input_next_stop_increment_depth_bar * 10);

        if(settingsGetPointer()->nonMetricalSystem)
        {
            depthLast		= (uint8_t)unit_depth_integer(depthLast);
            depthSecond	= (uint8_t)unit_depth_integer(depthSecond);
            depthInc 		= (uint8_t)unit_depth_integer(depthInc);
        }

        for(start=DECOINFO_STRUCT_MAX_STOPS-1; start>0; start--)
            if(pDecoinfo->output_stop_length_seconds[start]) break;
        start -= 6;
        if(start < 0) start = 0;

        textpointer = 0;
        for(int i=start;i<6+start;i++)
        {
            if(i == 0)
                depthNext = depthLast;
            else
                depthNext = depthSecond + (( i - 1 )* depthInc);

            if(pDecoinfo->output_stop_length_seconds[i])
                textpointer += snprintf(&text[textpointer],20,"\030\034   %2u\016\016%c%c\017%3i'\n\r",depthNext, unit_depth_char1(), unit_depth_char2(), (pDecoinfo->output_stop_length_seconds[i]+59)/60);
            else
                textpointer += snprintf(&text[textpointer],20,"\031\034   %2u\016\016%c%c\017\n\r",depthNext, unit_depth_char1(), unit_depth_char2());
            if(textpointer > 200) break;
        }
        if(!pSettings->FlipDisplay)
        {
        	t7cY0free.WindowY0 = t7cC.WindowY0 - 10;
        }
        else
        {
        	t7cY0free.WindowY0 = t7cC.WindowY0 - 10;
        	t7cY0free.WindowY1 = 400;
        }

        t7cY0free.WindowLineSpacing = 48;
        t7cY0free.WindowNumberOfTextLines = 6;
        GFX_write_string(&FontT42, &t7cY0free, text, 1);
        break;
    }
}



/* DIVE MODE
 */
void t7_refresh_divemode(void)
{
    char TextL1[TEXTSIZE];
    char TextL2[TEXTSIZE];

    char TextR1[TEXTSIZE];
    char TextR2[TEXTSIZE];
    char TextR3[TEXTSIZE];

    char TextC1[2*TEXTSIZE];
    char TextC2[TEXTSIZE];
    uint8_t textPointer;

    point_t start, stop;
    uint8_t color;
    int textlength;

    uint16_t 	nextstopLengthSeconds = 0;
    uint8_t 	nextstopDepthMeter = 0;
    uint8_t oxygen_percentage = 0;
    SDivetime Divetime = {0,0,0, 0};
    SDivetime SafetyStopTime = {0,0,0,0};
    SDivetime TimeoutTime = {0,0,0,0};
    uint8_t  customview_warnings = 0;
    const SDecoinfo * pDecoinfo;

	SSettings* pSettings;
	pSettings = settingsGetPointer();

    Divetime.Total = stateUsed->lifeData.dive_time_seconds_without_surface_time;
    Divetime.Minutes = Divetime.Total / 60;
    Divetime.Seconds = Divetime.Total - ( Divetime.Minutes * 60 );

    SafetyStopTime.Total = timer_Safetystop_GetCountDown();
    SafetyStopTime.Minutes = SafetyStopTime.Total / 60;
    SafetyStopTime.Seconds = SafetyStopTime.Total - (SafetyStopTime.Minutes * 60);

    TimeoutTime.Total = settingsGetPointer()->timeoutDiveReachedZeroDepth - stateUsed->lifeData.counterSecondsShallowDepth;
    if(TimeoutTime.Total > settingsGetPointer()->timeoutDiveReachedZeroDepth)
    {
        TimeoutTime.Total = 0;
    }
    TimeoutTime.Minutes = TimeoutTime.Total / 60;
    TimeoutTime.Seconds = TimeoutTime.Total - (TimeoutTime.Minutes * 60);

    if(stateUsed->diveSettings.deco_type.ub.standard == GF_MODE)
        pDecoinfo = &stateUsed->decolistBuehlmann;
    else
        pDecoinfo = &stateUsed->decolistVPM;

    if(pDecoinfo->output_time_to_surface_seconds)
    {
        tHome_findNextStop(pDecoinfo->output_stop_length_seconds, &nextstopDepthMeter, &nextstopLengthSeconds);
    }
    else
    {
        nextstopDepthMeter = 0;
        nextstopLengthSeconds = 0;
    }

    /* depth */
    float depth = 0;
    float depthThisCall = unit_depth_float(stateUsed->lifeData.depth_meter);
    if(is_stateUsedSetToSim())
    {
        depth = (depthThisCall + depthLastCall[0] + depthLastCall[1] + depthLastCall[2] + depthLastCall[3] + depthLastCall[4] + depthLastCall[5] + depthLastCall[6] + depthLastCall[7] + depthLastCall[8]) / 10.0f;

        idDepthLastCall++;
        if(idDepthLastCall >= 9)
            idDepthLastCall = 0;
        depthLastCall[idDepthLastCall] = depthThisCall;
    }
    else
    {
        depth = (depthThisCall + depthLastCall[0] + depthLastCall[1] + depthLastCall[2]) / 4.0f;

        idDepthLastCall++;
        if(idDepthLastCall >= 3)
            idDepthLastCall = 0;
        depthLastCall[idDepthLastCall] = depthThisCall;
    }

    if(depth <= 0.3f)
        depth = 0;

    if(settingsGetPointer()->nonMetricalSystem)
        snprintf(TextL1,TEXTSIZE,"\032\f[feet]");
    else
        snprintf(TextL1,TEXTSIZE,"\032\f%c",TXT_Depth);
    GFX_write_string(&FontT24,&t7l1,TextL1,0);

    if((stateUsed->lifeData.ascent_rate_meter_per_min > 8) || (stateUsed->lifeData.ascent_rate_meter_per_min < -10))
    {
        snprintf(TextL1,TEXTSIZE,"\f\002%.0f %c%c/min "
            , unit_depth_float(stateUsed->lifeData.ascent_rate_meter_per_min)
            , unit_depth_char1()
            , unit_depth_char2()
        );
        GFX_write_string(&FontT24,&t7l1,TextL1,0);
    }

    if( depth < 100)
        snprintf(TextL1,TEXTSIZE,"\020%01.1f",depth);
    else
        snprintf(TextL1,TEXTSIZE,"\020%01.0f",depth);

    t7_colorscheme_mod(TextL1);
    GFX_write_string(&FontT144,&t7l1,TextL1,1);

    /* max depth */
    snprintf(TextL2,TEXTSIZE,"\032\f%c",TXT_MaxDepth);
    GFX_write_string(&FontT42,&t7l2,TextL2,0);

    if(unit_depth_float(stateUsed->lifeData.max_depth_meter) < 100)
        snprintf(TextL2,TEXTSIZE,"\020%01.1f",unit_depth_float(stateUsed->lifeData.max_depth_meter));
    else
        snprintf(TextL2,TEXTSIZE,"\020%01.0f",unit_depth_float(stateUsed->lifeData.max_depth_meter));

    t7_colorscheme_mod(TextL2);
    GFX_write_string(&FontT105,&t7l2,TextL2,1);

    /* ascentrate graph */
    if(stateUsed->lifeData.ascent_rate_meter_per_min > 0)
    {
    	if(!pSettings->FlipDisplay)
    	{
    		start.y = t7l1.WindowY0 - 1;
    	}
    	else
    	{
    		start.y = t7l3.WindowY0 - 25;
    	}

        for(int i = 0; i<4;i++)
        {
            start.y += 5*6;
            stop.y = start.y;
            start.x = CUSTOMBOX_LINE_LEFT - 1;
            stop.x = start.x - 17;
            GFX_draw_line(&t7screen, start, stop, 0);
//			start.x = CUSTOMBOX_LINE_RIGHT + 2; old right too
//			stop.x = start.x + 17;
//			GFX_draw_line(&t7screen, start, stop, 0);
        }
        // new thick bar design Sept. 2015
        start.x = CUSTOMBOX_LINE_LEFT - CUSTOMBOX_OUTSIDE_OFFSET - 3 - 5;
        stop.x = start.x;
    	if(!pSettings->FlipDisplay)
    	{
    		start.y = t7l1.WindowY0 - 1;
    	}
    	else
    	{
    		start.y = t7l3.WindowY0 - 25;
    	}
        stop.y = start.y + (uint16_t)(stateUsed->lifeData.ascent_rate_meter_per_min * 6);
        stop.y -= 3; // wegen der Liniendicke von 12 anstelle von 9
        if(stop.y >= 470)
            stop.y = 470;
        start.y += 7; // starte etwas weiter oben
        if(stateUsed->lifeData.ascent_rate_meter_per_min <= 10)
            color = CLUT_EverythingOkayGreen;
        else
        if(stateUsed->lifeData.ascent_rate_meter_per_min <= 15)
            color = CLUT_WarningYellow;
        else
            color = CLUT_WarningRed;

        GFX_draw_thick_line(12,&t7screen, start, stop, color);
    }
    //snprintf(TextL2,TEXTSIZE,"\f%.1f m/min",stateUsed->lifeData.ascent_rate_meter_per_min);

    /* divetime */
    if(stateUsed->lifeData.counterSecondsShallowDepth)
    {
        snprintf(TextR1,TEXTSIZE,"\f\002\136 %u:%02u",TimeoutTime.Minutes, TimeoutTime.Seconds);
        GFX_write_string(&FontT42,&t7r1,TextR1,0);
    }
    else
    {
        snprintf(TextR1,TEXTSIZE,"\032\f\002%c",TXT_Divetime);
        GFX_write_string(&FontT42,&t7r1,TextR1,0);
    }

    if(Divetime.Minutes < 1000)
        snprintf(TextR1,TEXTSIZE,"\020\016\002%u:%02u",Divetime.Minutes, Divetime.Seconds);
    else
        snprintf(TextR1,TEXTSIZE,"\020\016\002%u'",Divetime.Minutes);
    t7_colorscheme_mod(TextR1);
    GFX_write_string(&FontT105,&t7r1,TextR1,1);

    /* next deco stop */
    if(nextstopDepthMeter)
    {
        snprintf(TextR2,TEXTSIZE,"\032\f\002%c",TXT_Decostop);
        GFX_write_string(&FontT42,&t7r2,TextR2,0);
        textlength = snprintf(TextR2,TEXTSIZE,"\020\002%u%c%c %u'"
            , unit_depth_integer(nextstopDepthMeter)
            , unit_depth_char1_T105()
            , unit_depth_char2_T105()
            , (nextstopLengthSeconds+59)/60);
        t7_colorscheme_mod(TextR2);
        if(time_elapsed_ms(pDecoinfo->tickstamp, HAL_GetTick()) > MAX_AGE_DECOINFO_MS)
            TextR2[0] = '\021';
        if(textlength <= 9)
            GFX_write_string(&FontT105,&t7r2,TextR2,1);
        else
            GFX_write_string(&FontT54,&t7r2,TextR2,1);
    }
    else if(SafetyStopTime.Total && (depth > timer_Safetystop_GetDepthUpperLimit()))
    {
        snprintf(TextR2,TEXTSIZE,"\032\f\002%c%c",TXT_2BYTE,TXT2BYTE_SafetyStop2);
        GFX_write_string(&FontT42,&t7r2,TextR2,0);
        snprintf(TextR2,TEXTSIZE,"\020\016\002%u:%02u",SafetyStopTime.Minutes,SafetyStopTime.Seconds);
        t7_colorscheme_mod(TextR2);
        GFX_write_string(&FontT105,&t7r2,TextR2,1);
    }

    /* tts - option 1
     * ndl - option 2
     * empty - option 3 */
    if(pDecoinfo->output_time_to_surface_seconds)
    {
        snprintf(TextR3,TEXTSIZE,"\032\f\002%c",TXT_TTS);
        GFX_write_string(&FontT42,&t7r3,TextR3,0);
        if(pDecoinfo->output_time_to_surface_seconds < 1000 * 60)
            snprintf(TextR3,TEXTSIZE,"\020\002%i'",(pDecoinfo->output_time_to_surface_seconds + 30)/ 60);
        else
            snprintf(TextR3,TEXTSIZE,"\020\002%ih",pDecoinfo->output_time_to_surface_seconds / 3600);
        t7_colorscheme_mod(TextR3);
        if(time_elapsed_ms(pDecoinfo->tickstamp, HAL_GetTick()) > MAX_AGE_DECOINFO_MS)
            TextR2[0] = '\021';
        GFX_write_string(&FontT105,&t7r3,TextR3,1);
    }
    else if(pDecoinfo->output_ndl_seconds)
    {
        snprintf(TextR3,TEXTSIZE,"\032\f\002%c",TXT_Nullzeit);
        GFX_write_string(&FontT42,&t7r3,TextR3,0);
        if(pDecoinfo->output_ndl_seconds < 1000 * 60)
            snprintf(TextR3,TEXTSIZE,"\020\002%i'",pDecoinfo->output_ndl_seconds/60);
        else
            snprintf(TextR3,TEXTSIZE,"\020\002%ih",pDecoinfo->output_ndl_seconds/3600);
        t7_colorscheme_mod(TextR3);
        if(time_elapsed_ms(pDecoinfo->tickstamp, HAL_GetTick()) > MAX_AGE_DECOINFO_MS)
            TextR2[0] = '\021';
        GFX_write_string(&FontT105,&t7r3,TextR3,1);
    }

    /* Menu Selection (and gas mix) */
    if(get_globalState() == StDMGAS)
    {
        textPointer = 0;
        TextR1[textPointer++] = '\a';
//		TextR1[textPointer++] = '\f';
        TextR1[textPointer++] = '\001';
        TextR1[textPointer++] = ' ';
        textPointer += tHome_gas_writer(stateUsed->diveSettings.gas[actualBetterGasId()].oxygen_percentage,stateUsed->diveSettings.gas[actualBetterGasId()].helium_percentage,&TextR1[textPointer]);
        TextR1[textPointer++] = '?';
        TextR1[textPointer++] = ' ';
        TextR1[textPointer++] = 0;
        GFX_write_string_color(&FontT48,&t7c2,TextR1,0,CLUT_WarningYellow);
    }
    else if(get_globalState() == StDMSPT)
    {
        textPointer = 0;
        TextR1[textPointer++] = '\a';
        TextR1[textPointer++] = '\001';
        TextR1[textPointer++] = ' ';
        textPointer += snprintf(&TextR1[textPointer],5,"%f01.2",((float)(stateUsed->diveSettings.setpoint[actualBetterSetpointId()].setpoint_cbar))/100);
        TextR1[textPointer++] = '?';
        TextR1[textPointer++] = ' ';
        TextR1[textPointer++] = 0;
        GFX_write_string_color(&FontT48,&t7c2,TextR1,0,CLUT_WarningYellow);
    }
    else if(get_globalState() == StDMENU)
    {
        snprintf(TextR1,TEXTSIZE,"\a\001%c%c", TXT_2BYTE, TXT2BYTE_DiveMenuQ);
        GFX_write_string_color(&FontT48,&t7c2,TextR1,0,CLUT_WarningYellow);
    }
    else if(get_globalState() == StDSIM1)
    {
        snprintf(TextR1,TEXTSIZE,"\a\001%c%c", TXT_2BYTE, TXT2BYTE_DiveQuitQ);
        GFX_write_string_color(&FontT48,&t7c2,TextR1,0,CLUT_WarningYellow);
    }
    else if(get_globalState() == StDSIM2)
    {
        if(settingsGetPointer()->nonMetricalSystem)
            snprintf(TextR1,TEXTSIZE,"\a\001" " Sim:-3.33ft ");
        else
            snprintf(TextR1,TEXTSIZE,"\a\001" " Sim:-1m ");
        GFX_write_string_color(&FontT48,&t7c2,TextR1,0,CLUT_WarningYellow);

        snprintf(TextR1,TEXTSIZE,"\a\f %u %c%c"
            , unit_depth_integer(simulation_get_aim_depth())
            , unit_depth_char1()
            , unit_depth_char2()
            );
        GFX_write_string_color(&FontT42,&t7l1,TextR1,0,CLUT_WarningYellow);

    }
    else if(get_globalState() == StDSIM3)
    {
        if(settingsGetPointer()->nonMetricalSystem)
            snprintf(TextR1,TEXTSIZE,"\a\001" " Sim:+3.33ft ");
        else
            snprintf(TextR1,TEXTSIZE,"\a\001" " Sim:+1m ");
        GFX_write_string_color(&FontT48,&t7c2,TextR1,0,CLUT_WarningYellow);
        snprintf(TextR1,TEXTSIZE,"\a\f %u %c%c"
            , unit_depth_integer(simulation_get_aim_depth())
            , unit_depth_char1()
            , unit_depth_char2()
            );
        GFX_write_string_color(&FontT42,&t7l1,TextR1,0,CLUT_WarningYellow);
    }
    else if(get_globalState() == StDSIM4)
    {
        snprintf(TextR1,TEXTSIZE,"\a\001" " Sim:+5' ");
        GFX_write_string_color(&FontT48,&t7c2,TextR1,0,CLUT_WarningYellow);
        snprintf(TextR1,TEXTSIZE,"\a\f %u %c%c"
            , unit_depth_integer(simulation_get_aim_depth())
            , unit_depth_char1()
            , unit_depth_char2()
            );
        GFX_write_string_color(&FontT42,&t7l1,TextR1,0,CLUT_WarningYellow);
    }
    else
    {
        /* gas mix */
        oxygen_percentage = 100;
        oxygen_percentage -= stateUsed->lifeData.actualGas.nitrogen_percentage;
        oxygen_percentage -= stateUsed->lifeData.actualGas.helium_percentage;

        textPointer = 0;
        TextC2[textPointer++] = '\020';
        if(stateUsed->warnings.betterGas && warning_count_high_time)
        {
            TextC2[textPointer++] = '\a';
        }
        else
        {
            float fPpO2limitHigh, fPpO2now;

            if(actualLeftMaxDepth(stateUsed))
                fPpO2limitHigh = settingsGetPointer()->ppO2_max_deco;
            else
                fPpO2limitHigh = settingsGetPointer()->ppO2_max_std;

            fPpO2now = (stateUsed->lifeData.pressure_ambient_bar - WATER_VAPOUR_PRESSURE) * oxygen_percentage;

            if((fPpO2now > fPpO2limitHigh) || (fPpO2now < (float)(settingsGetPointer()->ppO2_min)))
                TextC2[textPointer++] = '\025';
        }
        TextC2[textPointer++] = '\002';
        textPointer += tHome_gas_writer(oxygen_percentage,stateUsed->lifeData.actualGas.helium_percentage,&TextC2[textPointer]);

        if(stateUsed->warnings.betterGas && warning_count_high_time)
        {
            if(TextC2[0] == '\020')
            {
                TextC2[0] = '\004'; // NOP
            }
            GFX_write_string_color(&FontT48,&t7c2,TextC2,0,CLUT_WarningYellow);
        }
        else
        {
            t7_colorscheme_mod(TextC2);
            GFX_write_string(&FontT48,&t7c2,TextC2,0); // T54 has only numbers
        }

        if(stateUsed->diveSettings.ccrOption)
        {
            if(stateUsed->diveSettings.diveMode == DIVEMODE_CCR)
            {
                snprintf(TextC2,TEXTSIZE,"\020%01.2f",stateUsed->lifeData.ppO2);
                if(stateUsed->warnings.betterSetpoint && warning_count_high_time && (stateUsed->diveSettings.diveMode == DIVEMODE_CCR))
                {
                    TextC2[0] = '\a'; // inverse instead of color \020
                    GFX_write_string_color(&FontT48,&t7c2,TextC2,0,CLUT_WarningYellow);
                }
                else
                {
                    t7_colorscheme_mod(TextC2);
                    GFX_write_string(&FontT48,&t7c2,TextC2,0);
                }
            }
        }
        else if(settingsGetPointer()->alwaysShowPPO2)
        {
            snprintf(TextC2,TEXTSIZE,"\020%01.2f",stateUsed->lifeData.ppO2);
            t7_colorscheme_mod(TextC2);
            GFX_write_string(&FontT48,&t7c2,TextC2,0);
        }
    }

    /* algorithm, ccr, bailout and battery */
    /* and permanent warnings (CNS) */

    if((stateUsed->warnings.cnsHigh) && display_count_high_time)
    {
        TextC2[0] = '\f';
        TextC2[1] = TXT_2BYTE;
        TextC2[2] = TXT2BYTE_WarnCnsHigh;
        TextC2[3] = 0;
        GFX_write_string_color(&FontT48,&t7c1,TextC2,0,CLUT_WarningRed);
    }
    else
    {
        if(stateUsed->warnings.aGf)
        {
            GFX_write_string_color(&FontT48,&t7c1,"\f" "aGF",0,CLUT_WarningYellow);
        }
        else if(stateUsed->diveSettings.deco_type.ub.standard == GF_MODE)
        {
            GFX_write_string(&FontT48,&t7c1,"\027\f" "GF",0);
        }
        else
        {
            GFX_write_string(&FontT48,&t7c1,"\027\f" "VPM",0);
        }

        if(stateUsed->diveSettings.diveMode == DIVEMODE_CCR)
            GFX_write_string(&FontT24,&t7c1,"\027\f\002" "CCR",0);
        //  GFX_write_string(&FontT24,&t7c1,"\f\177\177\x80" "CCR",0);
        else
        if(stateUsed->diveSettings.ccrOption)
            GFX_write_string(&FontT24,&t7c1,"\f\002\024" "Bailout",0);
        //  GFX_write_string(&FontT24,&t7c1,"\f\177\177\x80\024" "Bailout",0);
    }
    TextC1[0] = '\020';
    TextC1[1] = '3';
    TextC1[2] = '1';
    TextC1[3] = '1';
    TextC1[4] = '1';
    TextC1[5] = '1';
    TextC1[6] = '1';
    TextC1[7] = '1';
    TextC1[8] = '1';
    TextC1[9] = '1';
    TextC1[10] = '1';
    TextC1[11] = '1';
    TextC1[12] = '0';
    TextC1[13] = 0;

    for(int i=1;i<=10;i++)
    {
        if(	stateUsed->lifeData.battery_charge > (9 * i))
            TextC1[i+1] += 1;
    }

    if(stateUsed->warnings.lowBattery)
    {
        TextC1[0] = '\025';
        if(warning_count_high_time)
        {
            for(int i=2;i<=11;i++)
                TextC1[i] = '1';
        }
        else
        {
            TextC1[2] = '2';
        }
        GFX_write_string(&Batt24,&t7batt,TextC1,0);

        if((stateUsed->lifeData.battery_charge > 0) && (stateUsed->lifeData.battery_charge < 140))
        {
            snprintf(TextC1,16,"\004\025\f\002%u%%",(uint8_t)stateUsed->lifeData.battery_charge);
            if(warning_count_high_time)
                TextC1[0] = '\a';
            GFX_write_string(&FontT24,&t7voltage,TextC1,0);
        }
    }
    else
    {
        t7_colorscheme_mod(TextC1);
        GFX_write_string(&Batt24,&t7batt,TextC1,0);

        if((stateUsed->lifeData.battery_charge > 0) && (stateUsed->lifeData.battery_charge < 140))
        {
            snprintf(TextC1,16,"\020\f\002%u%%",(uint8_t)stateUsed->lifeData.battery_charge);
            t7_colorscheme_mod(TextC1);
            GFX_write_string(&FontT24,&t7voltage,TextC1,0); // t7batt
        }
    }

    /* customizable left lower corner */
    t7_refresh_divemode_userselected_left_lower_corner();


    /* customview - option 1
     * warning - option 2 */
    if(stateUsed->warnings.numWarnings)
        customview_warnings = t7_test_customview_warnings();

    background.pointer = 0;
    if(customview_warnings && warning_count_high_time)
        t7_show_customview_warnings();
    else
        t7_refresh_customview();

    /* the frame */
    draw_frame(1,1, CLUT_DIVE_pluginbox, CLUT_DIVE_FieldSeperatorLines);
}

void t7_set_field_to_primary(void)
{
    if(stateUsed->mode == MODE_DIVE)
            selection_custom_field = settingsGetPointer()->tX_userselectedLeftLowerCornerPrimary;
}

void t7_change_field(void)
{
    const uint8_t minVal = 0;
    const uint8_t maxValGF 	= 8;
    const uint8_t maxValVPM = 7;
    uint8_t maxNow = maxValGF;

    selection_custom_field++;

    if(stateUsed->diveSettings.deco_type.ub.standard == VPM_MODE)
        maxNow = maxValVPM;

    if(selection_custom_field > maxNow)
        selection_custom_field = minVal;
}


void t7_refresh_divemode_userselected_left_lower_corner(void)
{
    if(!selection_custom_field)
        return;

    char  headerText[10];
    char  text[TEXTSIZE];
    uint8_t textpointer = 0;
    _Bool tinyHeaderFont = 0;
    uint8_t line = 0;

    SDivetime Stopwatch = {0,0,0,0};
    float fAverageDepth, fAverageDepthAbsolute;
    const SDecoinfo * pDecoinfoStandard;
    const SDecoinfo * pDecoinfoFuture;
    float fCNS;

    float temperatureThisCall;
    float temperature;

    if(stateUsed->diveSettings.deco_type.ub.standard == GF_MODE)
    {
        pDecoinfoStandard = &stateUsed->decolistBuehlmann;
        pDecoinfoFuture = &stateUsed->decolistFutureBuehlmann;
    }
    else
    {
        pDecoinfoStandard = &stateUsed->decolistVPM;
        pDecoinfoFuture = &stateUsed->decolistFutureVPM;
    }

    Stopwatch.Total = timer_Stopwatch_GetTime();
    Stopwatch.Minutes = Stopwatch.Total / 60;
    Stopwatch.Seconds = Stopwatch.Total - ( Stopwatch.Minutes * 60 );
    fAverageDepth = timer_Stopwatch_GetAvarageDepth_Meter();
    fAverageDepthAbsolute = stateUsed->lifeData.average_depth_meter;

    headerText[0] = '\032';
    headerText[1] = '\f';

    switch(selection_custom_field)
    {
    /* Temperature */
    case 1:
    default:
        // mean value
        temperatureThisCall = unit_temperature_float(stateUsed->lifeData.temperature_celsius);
        temperature = (temperatureThisCall + temperatureLastCall[0] + temperatureLastCall[1] + temperatureLastCall[2]) / 4.0f;
        idTemperatureLastCall++;
        if(idTemperatureLastCall >= 3)
            idTemperatureLastCall = 0;
        temperatureLastCall[idTemperatureLastCall] = temperatureThisCall;
        // output
        headerText[2] = TXT_Temperature;
        textpointer = snprintf(text,TEXTSIZE,"\020\016%01.1f \140",temperature); // "\016\016%01.1f `" + C or F
        if(settingsGetPointer()->nonMetricalSystem == 0)
            text[textpointer++] = 'C';
        else
            text[textpointer++] = 'F';
        text[textpointer++] = 0;
        tinyHeaderFont = 0;
        break;

    /* Average Depth */
    case 2:
        headerText[2] = TXT_AvgDepth;
        if(settingsGetPointer()->nonMetricalSystem)
            snprintf(text,TEXTSIZE,"\020%01.0f",unit_depth_float(fAverageDepthAbsolute));
        else
            snprintf(text,TEXTSIZE,"\020%01.1f",fAverageDepthAbsolute);
        break;

    /* ppO2 */
    case 3:
        headerText[2] = TXT_ppO2;
        snprintf(text,TEXTSIZE,"\020%01.2f",stateUsed->lifeData.ppO2);
        break;

    /* Stop Uhr */
    case 4:
        headerText[2] = TXT_Stopwatch;
        if(settingsGetPointer()->nonMetricalSystem)
            snprintf(text,TEXTSIZE,"\020\016\016%u:%02u\n\r%01.0f",Stopwatch.Minutes, Stopwatch.Seconds,unit_depth_float(fAverageDepth));
        else
            snprintf(text,TEXTSIZE,"\020\016\016%u:%02u\n\r%01.1f",Stopwatch.Minutes, Stopwatch.Seconds,fAverageDepth);
        tinyHeaderFont = 1;
        line = 1;
        break;

    /* Ceiling */
    case 5:
        headerText[2] = TXT_Ceiling;
        if((pDecoinfoStandard->output_ceiling_meter > 99.9f) || (settingsGetPointer()->nonMetricalSystem))
            snprintf(text,TEXTSIZE,"\020%01.0f",unit_depth_float(pDecoinfoStandard->output_ceiling_meter));
        else
            snprintf(text,TEXTSIZE,"\020%01.1f",pDecoinfoStandard->output_ceiling_meter);
        break;

    /* Future TTS */
    case 6:
        headerText[2] = TXT_FutureTTS;
        snprintf(text,TEXTSIZE,"\020\016\016@+%u'\n\r" "%i' TTS",settingsGetPointer()->future_TTS, pDecoinfoFuture->output_time_to_surface_seconds / 60);
        tinyHeaderFont = 1;
        line = 1;
        break;

    /* CNS */
    case 7:
        headerText[2] = TXT_CNS;
        fCNS = stateUsed->lifeData .cns;
        if(fCNS > 999)
            fCNS = 999;
        snprintf(text,TEXTSIZE,"\020%.0f\016\016%%\017",fCNS);
        break;

    /* actual GF */
    case 8:
        headerText[2] = TXT_ActualGradient;
        snprintf(text,TEXTSIZE,"\020%.0f",100 * pDecoinfoStandard->output_relative_gradient);
        break;
    }
    headerText[3] = 0;

    if(tinyHeaderFont)
        GFX_write_string(&FontT24,&t7l3,headerText,0);
    else
        GFX_write_string(&FontT42,&t7l3,headerText,0);

    t7_colorscheme_mod(text);
    GFX_write_string(&FontT105,&t7l3,text,line);
}

/* Private functions ---------------------------------------------------------*/

uint8_t t7_customtextPrepare(char * text)
{
    uint8_t i, j, textptr, lineCount;
    char nextChar;

    textptr = 0;
    lineCount = 0;

    text[textptr++] = TXT_MINIMAL;

    j = 0;
    i = 0;
    do
    {
        j += i;
        i = 0;
        do
        {
            nextChar = settingsGetPointer()->customtext[i+j];
            i++;
            if((!nextChar) || (nextChar =='\n')  || (nextChar =='\r'))
                break;
            text[textptr++] = nextChar;
        } while (i < 12);

        if(!nextChar)
            break;

        if(lineCount < 3)
        {
            text[textptr++] = '\n';
            text[textptr++] = '\r';
        }
        lineCount++;
        for(uint8_t k=0;k<2;k++)
        {
            nextChar = settingsGetPointer()->customtext[i+j+k];
            if((nextChar =='\n')  || (nextChar =='\r'))
                i++;
            else
                break;
        }

    } while (lineCount < 4);

    text[textptr] = 0;
    return lineCount;
}

/* could be extended to search for \020 inside
 */
void t7_colorscheme_mod(char *text)
{
    if((text[0] == '\020') && !GFX_is_colorschemeDiveStandard())
    {
        text[0] = '\027';
    }
}


void draw_frame(_Bool PluginBoxHeader, _Bool LinesOnTheSides, uint8_t colorBox, uint8_t colorLinesOnTheSide)
{
    point_t LeftLow, WidthHeight;
    point_t start, stop;

	SSettings* pSettings;
	pSettings = settingsGetPointer();

    // plugin box
    LeftLow.x = CUSTOMBOX_LINE_LEFT;
    WidthHeight.x = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_LINE_LEFT;
    LeftLow.y = 60;
    WidthHeight.y = 440 - LeftLow.y;
    GFX_draw_box(&t7screen, LeftLow, WidthHeight, 1, colorBox);

    if(PluginBoxHeader)
    {
        // plugin box - header
        start.x = CUSTOMBOX_LINE_LEFT;
        stop.x = CUSTOMBOX_LINE_RIGHT;
        stop.y = start.y = 440 - 60;
        GFX_draw_line(&t7screen, start, stop, colorBox);
    }

    if(LinesOnTheSides)
    {
        // aufteilung links
        start.x = 0;
        stop.x = CUSTOMBOX_LINE_LEFT;
        if(!pSettings->FlipDisplay)
        {
        	stop.y = start.y = t7l1.WindowY0 - 1;
        }
        else
        {
        	stop.y = start.y = 480 - t7l1.WindowY1 - 1;
        }

        GFX_draw_line(&t7screen, start, stop, colorLinesOnTheSide);
        if(!pSettings->FlipDisplay)
        {
        	stop.y = start.y = t7l2.WindowY0 -1;
        }
        else
        {
        	stop.y = start.y = 480 - t7l2.WindowY1 -1;
        }
        GFX_draw_line(&t7screen, start, stop, colorLinesOnTheSide);

        // aufteilung rechts
        start.x = CUSTOMBOX_LINE_RIGHT;
        stop.x = 799;
        if(!pSettings->FlipDisplay)
        {
        	stop.y = start.y = t7l1.WindowY0 - 1;
        }
        else
        {
        	stop.y = start.y = 480 - t7l1.WindowY1 - 1;
        }

        GFX_draw_line(&t7screen, start, stop, colorLinesOnTheSide);
        if(!pSettings->FlipDisplay)
        {
        	stop.y = start.y = t7l2.WindowY0 - 1;
        }
        else
        {
        	stop.y = start.y = 480 - t7l2.WindowY1 - 1;
        }
        GFX_draw_line(&t7screen, start, stop, colorLinesOnTheSide);
    }
}


/* Compass like TCOS shellfish
 * input is 0 to 359
 * 2 px / 1 degree
 * Range is 148 degree with CUSTOMBOX_SPACE_INSIDE = 296
 * one side is 74 degree (less than 90 degree)
 * internal 360 + 180 degree of freedom
 * use positive values only, shift by 360 below 90 mid position
 */


point_t t7_compass_circle(uint8_t id, uint16_t degree)
{
    float fCos, fSin;
    const float piMult =  ((2 * 3.14159) / 360);
//	const int radius[4] = {95,105,115,60};
    const int radius[4] = {95,105,115,100};
    const point_t offset = {.x = 400, .y = 250};

    static point_t r[4][360] = { 0 };

    if(r[0][0].y == 0)		/* calc table at first call only */
    {
        for(int i=0;i<360;i++)
        {
            fCos = cos(i * piMult);
            fSin = sin(i * piMult);
            for(int j=0;j<4;j++)
            {
                r[j][i].x = offset.x + (int)(fSin * radius[j]);
                r[j][i].y = offset.y + (int)(fCos * radius[j]);
            }
        }
    }
    if(id > 3) id = 0;
    if(degree > 359) degree = 0;
    return r[id][degree];
}

/* range should be 0 to 30 bar if 300 meter with 100% of nitrogen or helium
 * T24 is 28 high
*/
void t7_tissues(const SDiveState * pState)
{
    point_t start, change, stop;
    float value;
    uint16_t front, cns100pixel;
    char text[256];
    uint8_t textpointer = 0;
    uint8_t color;

    float percent_N2;
    float percent_He;
    float partial_pressure_N2;
    float partial_pressure_He;

	SSettings* pSettings;
	pSettings = settingsGetPointer();


    /* N2 */
    t7cY0free.WindowLineSpacing = 28 + 48 + 14;
    if(!pSettings->FlipDisplay)
    {
    	t7cY0free.WindowY0 = t7cH.WindowY0 - 5 - 2 * t7cY0free.WindowLineSpacing;
    }
    else
    {
    	t7cY0free.WindowY0 = t7cH.WindowY0 + 15;
    	t7cY0free.WindowY1 = t7cY0free.WindowY0 + 250;
    }
    t7cY0free.WindowNumberOfTextLines = 3;

    text[textpointer++] = '\030';
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_Nitrogen;
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_Helium;
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_CNS;
    text[textpointer++] = 0;

    GFX_write_string(&FontT24, &t7cY0free, text, 1);

    if(!pSettings->FlipDisplay)
    {
    	start.y = t7cH.WindowY0 - 5;
    }
    else
    {
    	start.y = t7cH.WindowY1 - 5;
    }
    start.x = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    stop.x = start.x + CUSTOMBOX_SPACE_INSIDE;

    for(int i=0;i<16;i++)
    {
        stop.y = start.y;
        change.y = start.y;

        value = pState->lifeData.tissue_nitrogen_bar[i] - 0.7512f;
        value *= 80;//20

        if(value < 0)
            front = 0;
        else
        if(value > CUSTOMBOX_SPACE_INSIDE)
            front = CUSTOMBOX_SPACE_INSIDE;
        else
                front = (uint16_t)value;

        change.x = start.x + front;
        if(change.x != start.x)
            GFX_draw_thick_line(1,&t7screen, start, change, CLUT_Font030);
        if(change.x != stop.x)
            GFX_draw_thick_line(1,&t7screen, change, stop, CLUT_Font031);

        start.y -= 3;
    }

    /* He */
    start.y -= 28 + 14;
    for(int i=0;i<16;i++)
    {
        stop.y = start.y;
        change.y = start.y;

        value = pState->lifeData.tissue_helium_bar[i];
        value *= 80;//20

        if(value < 0)
            front = 0;
        else if(value > CUSTOMBOX_SPACE_INSIDE)
            front = CUSTOMBOX_SPACE_INSIDE;
        else
                front = (uint16_t)value;

        change.x = start.x + front;
        if(change.x != start.x)
            GFX_draw_thick_line(1,&t7screen, start, change, CLUT_Font030);
        if(change.x != stop.x)
            GFX_draw_thick_line(1,&t7screen, change, stop, CLUT_Font031);

        start.y -= 3;
    }

    /* CNS == Oxygen */
    start.y -= 28 + 14;

    cns100pixel = (8 * CUSTOMBOX_SPACE_INSIDE) / 10;
    value = pState->lifeData.cns;
    value *= cns100pixel;
    value /= 100;

    if(value < 0)
        front = 0;
    else if(value > CUSTOMBOX_SPACE_INSIDE)
        front = CUSTOMBOX_SPACE_INSIDE;
    else
            front = (uint16_t)value;

    if(pState->lifeData.cns < 95)
        color = CLUT_Font030;
    else if(pState->lifeData.cns < 100)
        color =  CLUT_WarningYellow;
    else
        color = CLUT_WarningRed;

    for(int i=0;i<16;i++)
    {
        stop.y = start.y;
        change.y = start.y;

        change.x = start.x + front;
        if(change.x != start.x)
            GFX_draw_thick_line(1,&t7screen, start, change, color);
        if(change.x != stop.x)
            GFX_draw_thick_line(1,&t7screen, change, stop, CLUT_Font031);

        start.y -= 3;
    }

    /* where is the onload/offload limit for N2 and He */
    decom_get_inert_gases(pState->lifeData.pressure_ambient_bar, &pState->lifeData.actualGas, &percent_N2, &percent_He);
    partial_pressure_N2 =  (pState->lifeData.pressure_ambient_bar - WATER_VAPOUR_PRESSURE) * percent_N2;
    partial_pressure_He = (pState->lifeData.pressure_ambient_bar - WATER_VAPOUR_PRESSURE) * percent_He;

    // Nitrogen vertical bar
    if(!pSettings->FlipDisplay)
    {
    	start.y = t7cH.WindowY0 + 1 - 5;
    }
    else
    {
    	start.y = t7cH.WindowY1 - 5;
    }

    stop.y = start.y - (3 * 15) - 1;
    if((percent_N2 > 0) && (partial_pressure_N2 > 0.8f))//(0.8f + 0.5f)))
    {
        value = partial_pressure_N2;
        value *= 80;//20

        if(value < 0)
            front = 3;
        else if(value + 5 > CUSTOMBOX_SPACE_INSIDE)
            front = CUSTOMBOX_SPACE_INSIDE - 3;
        else
                front = (uint16_t)value;
    }
    else
    {
        front = 1;
    }
    start.x = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET + front;
    stop.x = start.x;
    GFX_draw_thick_line(2,&t7screen, start, stop, CLUT_EverythingOkayGreen);


    // Helium vertical bar
    if(!pSettings->FlipDisplay)
    {
    	start.y = t7cH.WindowY0 + 1 - 5 - 3*16 - 28 - 14;
    }
    else
    {
    	start.y = t7cH.WindowY1 - 5 - 3*16 - 28 - 14;
    }

    stop.y = start.y - (3 * 15) - 1;
    if((percent_He > 0) && (partial_pressure_He > 0.01f)) // 0.5f
    {

        value = partial_pressure_He;
        value *= 80;//20

        if(value < 0)
            front = 3;
        else if(value + 5 > CUSTOMBOX_SPACE_INSIDE)
            front = CUSTOMBOX_SPACE_INSIDE - 3;
        else
                front = (uint16_t)value;
    }
    else
    {
        front = 1;
    }

    start.x = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET + front;
    stop.x = start.x;
    GFX_draw_thick_line(2,&t7screen, start, stop, CLUT_EverythingOkayGreen);

    // Oxygen vertical bar
    if(!pSettings->FlipDisplay)
    {
    	start.y = t7cH.WindowY0 + 1 - 5 - 6*16 - 2*28 - 2*14;
    }
    else
    {
    	start.y = t7cH.WindowY1 - 5 - 6*16 - 2*28 - 2*14;
    }

    stop.y = start.y - (3 * 15) - 1;

    start.x = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET + cns100pixel;
    stop.x = start.x;
    GFX_draw_thick_line(2, &t7screen, start, stop, CLUT_WarningRed);

}


void t7_debug(void)
{
    char text[256+50];
    uint8_t textpointer = 0;

    t7cY0free.WindowLineSpacing = 28 + 48 + 14;
    t7cY0free.WindowY0 = t7cH.WindowY0 - 5 - 2 * t7cY0free.WindowLineSpacing;
    t7cY0free.WindowNumberOfTextLines = 3;

    textpointer += snprintf(&text[textpointer],50,"Ambient [bar]\n\r");
    textpointer += snprintf(&text[textpointer],50,"Surface [bar] + salt\n\r");
//	textpointer += snprintf(&text[textpointer],50,"Difference [mbar]\n\r");
    textpointer += snprintf(&text[textpointer],50,"ShallowCounter [s]\n\r");
    GFX_write_string(&FontT24, &t7cY0free, text, 1);

    t7cY0free.WindowY0 -= 52;
//		snprintf(text,60,"%0.2f\n\r%0.2f       %u%%\n\r%0.0f",stateUsed->lifeData.pressure_ambient_bar, stateUsed->lifeData.pressure_surface_bar, settingsGetPointer()->salinity, 1000 * (stateUsed->lifeData.pressure_ambient_bar-stateUsed->lifeData.pressure_surface_bar));
    snprintf(text,60,
        "%0.2f\n\r"
        "%0.2f       %u%%\n\r"
        "%u"
        ,stateUsed->lifeData.pressure_ambient_bar
        ,stateUsed->lifeData.pressure_surface_bar
        ,settingsGetPointer()->salinity
        ,stateUsed->lifeData.counterSecondsShallowDepth);
    GFX_write_string(&FontT42, &t7cY0free, text, 1);
}


void t7_SummaryOfLeftCorner(void)
{
    char text[256+60];
    uint8_t textpointer = 0;

    const SDecoinfo * pDecoinfoStandard;
    const SDecoinfo * pDecoinfoFuture;
    float fCNS;

    if(stateUsed->diveSettings.deco_type.ub.standard == GF_MODE)
    {
        pDecoinfoStandard = &stateUsed->decolistBuehlmann;
        pDecoinfoFuture = &stateUsed->decolistFutureBuehlmann;
    }
    else
    {
        pDecoinfoStandard = &stateUsed->decolistVPM;
        pDecoinfoFuture = &stateUsed->decolistFutureVPM;
    }

    fCNS = stateUsed->lifeData .cns;
    if(fCNS > 999)
        fCNS = 999;

    t7cY0free.WindowY0 = t7cC.WindowY0 - 10;
    t7cY0free.WindowLineSpacing = 48;
    t7cY0free.WindowNumberOfTextLines = 6;
    t7cY0free.WindowTab = 420;

    // header
    textpointer = 0;
    text[textpointer++] = '\032';
    text[textpointer++] = '\016';
    text[textpointer++] = '\016';
    text[textpointer++] = TXT_ppO2;
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = TXT_Ceiling;
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = TXT_ActualGradient;
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = TXT_CNS;
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = TXT_FutureTTS;
    text[textpointer++] = '\017';
    text[textpointer++] = 0;
    t7cY0free.WindowX0 += 10;
    t7cY0free.WindowY0 += 10;
    GFX_write_string(&FontT24, &t7cY0free, text, 1);
    t7cY0free.WindowX0 -= 10;
    t7cY0free.WindowY0 -= 10;

    textpointer = 0;
    text[textpointer++] = '\t';
    textpointer += snprintf(&text[textpointer],10,"\020%01.2f",	stateUsed->lifeData.ppO2);
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = '\t';
    if((pDecoinfoStandard->output_ceiling_meter > 99.9f) || (settingsGetPointer()->nonMetricalSystem))
        textpointer += snprintf(&text[textpointer],10,"\020%01.1f",unit_depth_float(pDecoinfoStandard->output_ceiling_meter));
    else
        textpointer += snprintf(&text[textpointer],10,"\020%01.0f",pDecoinfoStandard->output_ceiling_meter);
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = '\t';
    textpointer += snprintf(&text[textpointer],10,"\020%.0f",		100 * pDecoinfoStandard->output_relative_gradient);
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = '\t';
    textpointer += snprintf(&text[textpointer],10,"\020%.0f\016\016%%\017",fCNS);
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = '\t';
    textpointer += snprintf(&text[textpointer],10,"\020%i'",		pDecoinfoFuture->output_time_to_surface_seconds / 60);
    text[textpointer++] = 0;
    GFX_write_string(&FontT42, &t7cY0free, text, 1);
}




/*
    point_t start, change, stop;
    float value;
    uint16_t front;
    uint8_t color;


    start.y = t7cH.WindowY0 - 5;
    start.x = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    stop.x = start.x + CUSTOMBOX_SPACE_INSIDE;


    for(int i=0;i<16;i++)
    {
        stop.y = start.y;
        change.y = start.y;

        value = pState->lifeData.tissue_nitrogen_bar[i] - 0.7512f;
        value *= 20;

        if(value < 0)
            front = 0;
        else
        if(value > CUSTOMBOX_SPACE_INSIDE)
            front = CUSTOMBOX_SPACE_INSIDE;
        else
                front = (uint16_t)value;

        change.x = start.x + front;
        if(change.x != start.x)
            GFX_draw_thick_line(1,&t7screen, start, change, CLUT_Font020);
        if(change.x != stop.x)
            GFX_draw_thick_line(1,&t7screen, change, stop, CLUT_Font021);

        start.y -= 3;
    }

    // He
    start.y -= 28 + 14;
    for(int i=0;i<16;i++)
    {
        stop.y = start.y;
        change.y = start.y;

        value = pState->lifeData.tissue_helium_bar[i];
        value *= 20;

        if(value < 0)
            front = 0;
        else
        if(value > CUSTOMBOX_SPACE_INSIDE)
            front = CUSTOMBOX_SPACE_INSIDE;
        else
                front = (uint16_t)value;

        change.x = start.x + front;
        if(change.x != start.x)
            GFX_draw_thick_line(1,&t7screen, start, change, CLUT_Font020);
        if(change.x != stop.x)
            GFX_draw_thick_line(1,&t7screen, change, stop, CLUT_Font021);

        start.y -= 3;
    }

    // CNS == Oxygen
    start.y -= 28 + 14;

    value = pState->lifeData.cns;
    value *= (CUSTOMBOX_SPACE_INSIDE/2);
    value /= 100;

    if(value < 0)
        front = 0;
    else
    if(value > CUSTOMBOX_SPACE_INSIDE)
        front = CUSTOMBOX_SPACE_INSIDE;
    else
            front = (uint16_t)value;

    if(pState->lifeData.cns < 95)
        color = CLUT_Font020;
    else
    if(pState->lifeData.cns < 100)
        color =  CLUT_WarningYellow;
    else
        color = CLUT_WarningRed;

    for(int i=0;i<16;i++)
    {
        stop.y = start.y;
        change.y = start.y;

        change.x = start.x + front;
        if(change.x != start.x)
            GFX_draw_thick_line(1,&t7screen, start, change, color);
        if(change.x != stop.x)
            GFX_draw_thick_line(1,&t7screen, change, stop, CLUT_Font021);

        start.y -= 3;
    }

    // where is the onload/offload limit for N2 and He
    decom_get_inert_gases(pState->lifeData.pressure_ambient_bar, &pState->lifeData.actualGas, &percent_N2, &percent_He);
    partial_pressure_N2 =  (pState->lifeData.pressure_ambient_bar - WATER_VAPOUR_PRESSURE) * percent_N2;
    partial_pressure_He = (pState->lifeData.pressure_ambient_bar - WATER_VAPOUR_PRESSURE) * percent_He;

        if((percent_N2 > 0) && (partial_pressure_N2 > (0.8f + 0.5f)))
    {
        start.y = t7cH.WindowY0 + 1 - 5;
        stop.y = start.y - (3 * 15) - 1;

        value = partial_pressure_N2;
        value *= 20;

        if(value < 0)
            front = 3;
        else
        if(value + 5 > CUSTOMBOX_SPACE_INSIDE)
            front = CUSTOMBOX_SPACE_INSIDE - 3;
        else
                front = (uint16_t)value;

        start.x = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET + front;
        stop.x = start.x;
        GFX_draw_thick_line(2,&t7screen, start, stop, CLUT_EverythingOkayGreen);
    }

    if((percent_He > 0) && (partial_pressure_He > 0.5f))
    {
        start.y = t7cH.WindowY0 + 1 - 5 - 3*16 - 28 - 14;
        stop.y = start.y - (3 * 15) - 1;

        value = partial_pressure_He;
        value *= 20;

        if(value < 0)
            front = 3;
        else
        if(value + 5 > CUSTOMBOX_SPACE_INSIDE)
            front = CUSTOMBOX_SPACE_INSIDE - 3;
        else
                front = (uint16_t)value;

        start.x = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET + front;
        stop.x = start.x;
        GFX_draw_thick_line(2,&t7screen, start, stop, CLUT_EverythingOkayGreen);
    }

    start.y = t7cH.WindowY0 + 1 - 5 - 6*16 - 2*28 - 2*14;
    stop.y = start.y - (3 * 15) - 1;

    start.x = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET + CUSTOMBOX_SPACE_INSIDE/2;
    stop.x = start.x;
    GFX_draw_thick_line(2, &t7screen, start, stop, CLUT_WarningRed);
*/

/*
void t7_clock(void)
{
    point_t start, stop;


    for(uint16_t i=0;i<360;i+=30)
    {
        start = t7_compass_circle(1,i);
        stop = t7_compass_circle(2,i);
        start.x += 280; // standard center is 400, 250
        stop.x += 280;
        GFX_draw_thick_line(5,&t7screen, start, stop, CLUT_CompassSubTick);

        start.x = 400+280;
        start.y = 250;
        stop = t7_compass_circle(2,58);
        stop.x += 280;
        GFX_draw_thick_line(3,&t7screen, start, stop, CLUT_CompassNorthTick);
        stop = t7_compass_circle(0,302);
        stop.x += 280;
        GFX_draw_thick_line(3,&t7screen, start, stop, CLUT_CompassNorthTick);
    }
}
*/

//static float testCC = 180;
void t7_compass(uint16_t ActualHeading, uint16_t UserSetHeading)
{

/*
    static uint16_t lastHeading = 0;
    float differenceCompass, resultKalman;

    if(testCC > lastHeading)
        differenceCompass = testCC - lastHeading;
    else
        differenceCompass = lastHeading - testCC;

    resultKalman = Kalman_getAngle(differenceCompass, 2, 0.1);
    if(testCC > lastHeading)
        ActualHeading = lastHeading + resultKalman;
    else
        ActualHeading = lastHeading - resultKalman;

    lastHeading = ActualHeading;
*/
	uint16_t ActualHeadingRose;
    uint16_t LeftBorderHeading, LineHeading;
    uint32_t offsetPicture;
    point_t start, stop, center;
    static int32_t LastHeading = 0;
    int32_t newHeading = 0;
    int32_t diff = 0;
    int32_t diff2 = 0;

    int32_t diffAbs = 0;
    int32_t diffAbs2 = 0;

	SSettings* pSettings;
	pSettings = settingsGetPointer();

    newHeading = ActualHeading;

    diff = newHeading - LastHeading;

    if(newHeading < LastHeading)
        diff2 = newHeading + 360 - LastHeading;
    else
        diff2 = newHeading - 360 - LastHeading;

    diffAbs = diff;
    if(diffAbs < 0)
        diffAbs *= -1;

    diffAbs2 = diff2;
    if(diffAbs2 < 0)
        diffAbs2 *= -1;


    if(diffAbs <= diffAbs2)
        newHeading = LastHeading + (diff / 2);
    else
        newHeading = LastHeading + (diff2 / 2);

    if(newHeading < 0)
        newHeading += 360;
    else
    if(newHeading >= 360)
        newHeading -= 360;

    LastHeading = newHeading;
    ActualHeading = newHeading;
    ActualHeadingRose = ActualHeading;
/*
    if (ActualHeading < 90)
        ActualHeading += 360;

    if(ActualHeading > LastHeading)
    {
        if((ActualHeading - LastHeading) < 25)
            ActualHeading = LastHeading + 1;
    }
    else
    if(ActualHeading < LastHeading)
    {
        if((LastHeading - ActualHeading) < 25)
            ActualHeading = LastHeading - 1;
    }
*/
    if(pSettings->FlipDisplay)
    {
    	ActualHeadingRose = 360 - ActualHeadingRose;
    	if (ActualHeadingRose < 170) ActualHeadingRose += 360;
    }
    else
    {
    	if (ActualHeadingRose < 90) ActualHeadingRose += 360;
    	ActualHeading = ActualHeadingRose;
    }

    // new hw 160822
//	if (ActualHeading >= 360 + 90)
//		ActualHeading = 360;

    LeftBorderHeading = 2 * (ActualHeadingRose - (CUSTOMBOX_SPACE_INSIDE/4));

    if(pSettings->FlipDisplay) /* add offset caused by mirrowed drawing */
    {
    	LeftBorderHeading += 2 * 80;
    }

    offsetPicture = LeftBorderHeading * t7screenCompass.ImageHeight * 2;

/* the background is used to draw the rotating compass rose */
    background.pointer = t7screenCompass.FBStartAdress+offsetPicture;
    background.x0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    if(!pSettings->FlipDisplay)
    {
    	background.y0 = 65;
    }
    else
    {
    	background.y0 = 480 - t7screenCompass.ImageHeight - 65;
    }

    background.width = CUSTOMBOX_SPACE_INSIDE;
    background.height = t7screenCompass.ImageHeight;


    start.x = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET + (CUSTOMBOX_SPACE_INSIDE/2);
    stop.x = start.x;
    start.y = 65;
    stop.y =  start.y + 55;
    GFX_draw_line(&t7screen, start, stop, CLUT_Font030);


    center.x = start.x;
    center.y = 300;

    stop.x = center.x + 44;
    stop.y = center.y + 24;


    while(ActualHeading > 359) ActualHeading -= 360;

    LineHeading = 360 - ActualHeading;
    GFX_draw_thick_line(9,&t7screen, t7_compass_circle(0,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font030); // North
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(9,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031); // Maintick
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(9,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(9,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031);

    LineHeading = 360 - ActualHeading;
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(5,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031); // Subtick
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(5,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(5,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(5,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031);

    LineHeading = 360 - ActualHeading;
    LineHeading += 22;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031); // Subtick
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031); // Subtick
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t7screen, t7_compass_circle(1,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_Font031);

    if(UserSetHeading)
    {
        LineHeading = UserSetHeading + 360 - ActualHeading;
        if(LineHeading > 359) LineHeading -= 360;
        GFX_draw_thick_line(9,&t7screen, t7_compass_circle(3,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_CompassUserHeadingTick);

        // Rï¿½ckpeilung, User Back Heading
        LineHeading = UserSetHeading + 360 + 180 - ActualHeading;
        if(LineHeading > 359) LineHeading -= 360;
        if(LineHeading > 359) LineHeading -= 360;
        GFX_draw_thick_line(9,&t7screen, t7_compass_circle(3,LineHeading),  t7_compass_circle(2,LineHeading), CLUT_CompassUserBackHeadingTick);
    }

    center.x = start.x;
    center.y = 250;
    GFX_draw_circle(&t7screen, center, 116, CLUT_Font030);
    GFX_draw_circle(&t7screen, center, 118, CLUT_Font030);
    GFX_draw_circle(&t7screen, center, 117, CLUT_Font030);


}


/* Font_T42: N is 27 px, S is 20 px, W is 36 px, E is 23 px
 * max is NW with 63 px
 * Font_T24: N is 15 px, S is 12 px, W is 20 px, E is 13 px
 * max is NW with 35 px
 * NE is 28 px
 * SW is 32 px
 * SE is 25 px
 * space between each is 45 px * 2
 * FirstItem List
 * \177 \177 prepare for size
*/
void init_t7_compass(void)
{
    t7screenCompass.FBStartAdress = getFrame(21);

    char text[256];
    uint8_t textpointer = 0;

    text[textpointer++] = '\030';
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 76; // 90 - 14
    text[textpointer++] = '\016';
    text[textpointer++] = '\016';
    text[textpointer++] = 'N';
    text[textpointer++] = 'E'; // 96 + 28 = 124 total
    text[textpointer++] = '\017';
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 64; // 90 - 14 - 12
    text[textpointer++] = 'E'; // 124 + 74 + 23 = 221 total
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 66; // 90 - 11 - 13
    text[textpointer++] = '\016';
    text[textpointer++] = '\016';
    text[textpointer++] = 'S';
    text[textpointer++] = 'E';
    text[textpointer++] = '\017';
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 68; // 90 - 12 - 10
    text[textpointer++] = 'S';
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 64; // 90 - 10 - 16
    text[textpointer++] = '\016';
    text[textpointer++] = '\016';
    text[textpointer++] = 'S';
    text[textpointer++] = 'W';
    text[textpointer++] = '\017';
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 56; // 90 - 16 - 18
    text[textpointer++] = 'W';
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 54; // 90 - 18 - 18
    text[textpointer++] = '\016';
    text[textpointer++] = '\016';
    text[textpointer++] = 'N';
    text[textpointer++] = 'W';
    text[textpointer++] = '\017';
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 59; // 90 - 17 - 14
    text[textpointer++] = 'N';
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 63; // 90 - 13 - 14
    text[textpointer++] = '\016';
    text[textpointer++] = '\016';
    text[textpointer++] = 'N';
    text[textpointer++] = 'E';
    text[textpointer++] = '\017';
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 64; // 90 - 14 - 12
    text[textpointer++] = 'E';
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 66; // 90 - 11 - 13
    text[textpointer++] = '\016';
    text[textpointer++] = '\016';
    text[textpointer++] = 'S';
    text[textpointer++] = 'E';
    text[textpointer++] = '\017';
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 68; // 90 - 12 - 10
    text[textpointer++] = 'S';
    text[textpointer++] = '\177';
    text[textpointer++] = '\177';
    text[textpointer++] = 64; // 90 - 10 - 16
    text[textpointer++] = '\016';
    text[textpointer++] = '\016';
    text[textpointer++] = 'S';
    text[textpointer++] = 'W';
    text[textpointer++] = '\017';
    text[textpointer++] = 0; // end

    GFX_write_string(&FontT42,&t7pCompass,text,1);

    releaseAllFramesExcept(21,t7screenCompass.FBStartAdress);
}


void t7_miniLiveLogProfile(void)
{
    SWindowGimpStyle wintemp;
	SSettings* pSettings;
	pSettings = settingsGetPointer();

    wintemp.left = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    wintemp.right = wintemp.left + CUSTOMBOX_SPACE_INSIDE;
    if(!pSettings->FlipDisplay)
    {
    	wintemp.top = 480 - t7l1.WindowY0;
    	wintemp.bottom = wintemp. top + 200;
    }
    else
    {
    	wintemp.top = t7l1.WindowY1;
    	wintemp.bottom = wintemp. top + 200;
    }

    uint16_t max_depth = (uint16_t)(stateUsed->lifeData.max_depth_meter * 10);

    GFX_graph_print(&t7screen, &wintemp, 0,1,0, max_depth, getMiniLiveLogbookPointerToData(), getMiniLiveLogbookActualDataLength(), CLUT_Font030, NULL);
}

void t7_logo_OSTC(void)
{
    SWindowGimpStyle windowGimp;
	SSettings* pSettings;
	pSettings = settingsGetPointer();

    /* OSTC logo */
	if(!pSettings->FlipDisplay)
	{
		windowGimp.left = t7l1.WindowX1 + 32;
	}
	else
	{
		windowGimp.left = t7r1.WindowX1 + 32;
	}

    windowGimp.top = 40 + 32;
    GFX_draw_image_monochrome(&t7screen, windowGimp, &ImgOSTC, 0);
}
