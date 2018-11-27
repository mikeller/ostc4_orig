///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/t9.c
/// \brief  Template for screen with 4+1+4 screen layout with plugin
/// \author Heinrichs Weikamp gmbh
/// \date   23-April-2014
///
/// \details
/// Bonex t9 -> spï¿½ter auch andere Pro Anzeigen
/// Nur Tauchmodus, sonst Umschaltung t7
/// frames used 26 for compass laufband und 23 fï¿½r t9screen
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
#include "t9.h"

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

void t9_refresh_divemode(void);
void t9_refresh_divemode_userselected_left_lower_corner(void);
void t9_refresh_customview(void);

void t9_draw_frame(_Bool PluginBoxHeader, _Bool LinesOnTheSides, uint8_t colorBox, uint8_t colorLinesOnTheSide);

void t9_tissues(const SDiveState * pState);
void t9_compass(uint16_t ActualHeading, uint16_t UserSetHeading);
void t9_scooter(void);
void t9_debug(void);
void t9_SummaryOfLeftCorner(void);

void t9_miniLiveLogProfile(void);
void t9_colorscheme_mod(char *text);

uint8_t t9_test_customview_warnings(void);
void t9_show_customview_warnings(void);

void t9_show_customview_warnings_surface_mode(void);

uint8_t t9_customtextPrepare(char * text);

/* Importend function prototypes ---------------------------------------------*/
extern uint8_t write_gas(char *text, uint8_t oxygen, uint8_t helium);

/* Exported variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
float t9_depthLastCall[9] = { 0,0,0,0,0,0,0,0,0};
uint8_t idt9_depthLastCall = 0;
float t9_temperatureLastCall[3] = { 0,0,0};
uint8_t idt9_temperatureLastCall = 0;

GFX_DrawCfgScreen	t9screen;
GFX_DrawCfgScreen	t9screenCompass;

/* left 3 fields
 * right 3 fields
 * centered one field on top of customview, one below
 * customview header + customview + warning
 */
GFX_DrawCfgWindow	t9l1, t9l2, t9l3, t9l4;
GFX_DrawCfgWindow	t9r1, t9r2, t9r3, t9r4;
GFX_DrawCfgWindow	t9c1, t9batt, t9c2, t9charge;
GFX_DrawCfgWindow	t9cH, t9cC, t9cW, t9cY0free;
GFX_DrawCfgWindow	t9pCompass;
GFX_DrawCfgWindow	t9surfaceL, t9surfaceR;

uint8_t t9_selection_custom_field = 0;
uint8_t t9_selection_customview = 1;

typedef struct{
    uint32_t pointer;
    uint32_t x0;
    uint32_t y0;
    uint32_t width;
    uint32_t height;
} S9Background;

S9Background t9_background =
{
    .pointer = NULL,
};

/* Private types -------------------------------------------------------------*/
const uint8_t t9_customviewsDiveStandard[] =
{
    CVIEW_sensors,
    CVIEW_Compass,
    CVIEW_Decolist,
    CVIEW_Tissues,
    CVIEW_Profile,
    CVIEW_Gaslist,
    CVIEW_sensors_mV,
    CVIEW_EADTime,
//  CVIEW_SummaryOfLeftCorner, da hier der scooter drin ist
    CVIEW_noneOrDebug,
    CVIEW_END,
    CVIEW_END
};

const uint8_t t9_customviewsDiveScooter[] =
{
    CVIEW_Scooter,
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


const uint8_t *t9_customviewsDive		= t9_customviewsDiveStandard;

#define TEXTSIZE 16
/* offset includes line: 2 = line +1
 * box (line) is 300 px
 * inside is 296 px
 * left of box are 249 px ( 0..248)
 * right of box are 249 px (551 .. 799)
 */

#define CUSTOMBOX_LINE_LEFT (250)
#define CUSTOMBOX_LINE_RIGHT (549)
#define CUSTOMBOX_INSIDE_OFFSET (2)
#define CUSTOMBOX_OUTSIDE_OFFSET (2)
#define CUSTOMBOX_SPACE_INSIDE (CUSTOMBOX_LINE_RIGHT + 1 - (CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET + CUSTOMBOX_INSIDE_OFFSET))


/* Exported functions --------------------------------------------------------*/

void t9_init(void)
{
    if(getLicence() == LICENCEBONEX)
    {
        t9_customviewsDive			= t9_customviewsDiveScooter;
    }

    t9_selection_custom_field = 0; // 0 is the new scooter temperature
    t9_selection_customview = t9_customviewsDive[0];

    t9screen.FBStartAdress = 0;
    t9screen.ImageHeight = 480;
    t9screen.ImageWidth = 800;
    t9screen.LayerIndex = 1;

    t9screenCompass.FBStartAdress = 0;
    t9screenCompass.ImageHeight = 240;
    t9screenCompass.ImageWidth = 1600;
    t9screenCompass.LayerIndex = 0;

    t9l1.Image = &t9screen;
    t9l1.WindowNumberOfTextLines = 2;
    t9l1.WindowLineSpacing = 5; // Abstand von Y0
    t9l1.WindowTab = 100; // vermtl. ohne Verwendung in diesem Fenster
    t9l1.WindowX0 = 0;
    t9l1.WindowX1 = CUSTOMBOX_LINE_LEFT - CUSTOMBOX_OUTSIDE_OFFSET;
    t9l1.WindowY1 = 479;
    t9l1.WindowY0 = t9l1.WindowY1 - 119;

    t9l2.Image = &t9screen;
    t9l2.WindowNumberOfTextLines = 2;
    t9l2.WindowLineSpacing = 5; // Abstand von Y0
    t9l2.WindowTab = 100; // vermtl. ohne Verwendung in diesem Fenster
    t9l2.WindowX0 = 0;
    t9l2.WindowX1 = t9l1.WindowX1;
    t9l2.WindowY1 = t9l1.WindowY0 - 4;
    t9l2.WindowY0 = t9l2.WindowY1 - 119;

    t9l3.Image = &t9screen;
    t9l3.WindowNumberOfTextLines = 2;
    t9l3.WindowLineSpacing = 5; // Abstand von Y0
    t9l3.WindowTab = 100; // vermtl. ohne Verwendung in diesem Fenster
    t9l3.WindowX0 = 0;
    t9l3.WindowX1 = t9l1.WindowX1;
    t9l3.WindowY1 = t9l2.WindowY0 - 4;
    t9l3.WindowY0 = t9l3.WindowY1 - 119;

    t9l4.Image = &t9screen;
    t9l4.WindowNumberOfTextLines = 2;
    t9l4.WindowLineSpacing = 50; // Abstand von Y0
    t9l4.WindowTab = 100; // vermtl. ohne Verwendung in diesem Fenster
    t9l4.WindowX0 = 0;
    t9l4.WindowX1 = t9l1.WindowX1;
    t9l4.WindowY1 = t9l3.WindowY0 - 4;
    t9l4.WindowY0 = 0;

    t9r1.Image = &t9screen;
    t9r1.WindowNumberOfTextLines = 2;
    t9r1.WindowLineSpacing = t9l1.WindowLineSpacing;
    t9r1.WindowTab = 100;
    t9r1.WindowX0 = 550;
    t9r1.WindowX1 = 799;
    t9r1.WindowY0 = t9l1.WindowY0;
    t9r1.WindowY1 = 479;

    t9r2.Image = &t9screen;
    t9r2.WindowNumberOfTextLines = 2;
    t9r2.WindowLineSpacing = t9l2.WindowLineSpacing;
    t9r2.WindowTab = 100;
    t9r2.WindowX0 = 550;
    t9r2.WindowX1 = 799;
    t9r2.WindowY0 = t9l2.WindowY0;
    t9r2.WindowY1 = t9l2.WindowY1;

    t9r3.Image = &t9screen;
    t9r3.WindowNumberOfTextLines = 2;
    t9r3.WindowLineSpacing = t9l3.WindowLineSpacing;
    t9r3.WindowTab = 100;
    t9r3.WindowX0 = 550;
    t9r3.WindowX1 = 799;
    t9r3.WindowY0 = t9l3.WindowY0;
    t9r3.WindowY1 = t9l3.WindowY1;

    t9r4.Image = &t9screen;
    t9r4.WindowNumberOfTextLines = 2;
    t9r4.WindowLineSpacing = t9l4.WindowLineSpacing;
    t9r4.WindowTab = 100;
    t9r4.WindowX0 = 550;
    t9r4.WindowX1 = 799;
    t9r4.WindowY0 = t9l4.WindowY0;
    t9r4.WindowY1 = t9l4.WindowY1;


/*
    t9r3.Image = &t9screen;
    t9r3.WindowNumberOfTextLines = 2;
    t9r3.WindowLineSpacing = 0;//t9l3.WindowLineSpacing;
    t9r3.WindowTab = 100;
    t9r3.WindowX0 = CUSTOMBOX_LINE_RIGHT + CUSTOMBOX_OUTSIDE_OFFSET;
    t9r3.WindowX1 = 799;
    t9r3.WindowY0 = t9l3.WindowY0;
    t9r3.WindowY1 = t9l3.WindowY1;
*/
    t9cC.Image = &t9screen;
    t9cC.WindowNumberOfTextLines = 3;
    t9cC.WindowLineSpacing = 95; // Abstand von Y0
    t9cC.WindowTab = 100;
    t9cC.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t9cC.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
    t9cC.WindowY0 = 90;
    t9cC.WindowY1 = 434 - 95;

    t9cH.Image = &t9screen;
    t9cH.WindowNumberOfTextLines = 1;
    t9cH.WindowLineSpacing = 95; // Abstand von Y0
    t9cH.WindowTab = 100;
    t9cH.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t9cH.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
    t9cH.WindowY0 = 434 - 94;
    t9cH.WindowY1 = 434;

    t9cW.Image = &t9screen;
    t9cW.WindowNumberOfTextLines = 3;
    t9cW.WindowLineSpacing = 95; // Abstand von Y0
    t9cW.WindowTab = 100;
    t9cW.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t9cW.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
    t9cW.WindowY0 = 90;
    t9cW.WindowY1 = 434 - 95;

    t9cY0free.Image = &t9screen;
    t9cY0free.WindowNumberOfTextLines = 1;
    t9cY0free.WindowLineSpacing = 95;
    t9cY0free.WindowTab = 100;
    t9cY0free.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t9cY0free.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
    t9cY0free.WindowY0 = 90;
    t9cY0free.WindowY1 = 434 - 95;

    t9batt.Image = &t9screen;
    t9batt.WindowNumberOfTextLines = 1;
    t9batt.WindowLineSpacing = 10;
    t9batt.WindowTab = 100;
    t9batt.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
    t9batt.WindowX0 = t9batt.WindowX1 - (52+52);
    t9batt.WindowY1 = 479;
    t9batt.WindowY0 = t9batt.WindowY1 - 25;

    t9charge.Image = &t9screen;
    t9charge.WindowNumberOfTextLines = 1;
    t9charge.WindowLineSpacing = 10;
    t9charge.WindowTab = 100;
    t9charge.WindowX1 = t9batt.WindowX1 - 18;
    t9charge.WindowX0 = t9charge.WindowX1 - 14;
    t9charge.WindowY1 = 479;
    t9charge.WindowY0 = t9batt.WindowY1 - 25;

    t9c1.Image = &t9screen;
    t9c1.WindowNumberOfTextLines = 1;
    t9c1.WindowLineSpacing = 10;
    t9c1.WindowTab = 100;
    t9c1.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t9c1.WindowX1 = t9batt.WindowX0 - 18;
    t9c1.WindowY0 = 435;
    t9c1.WindowY1 = 479;

    t9c2.Image = &t9screen;
    t9c2.WindowNumberOfTextLines = 1;
    t9c2.WindowLineSpacing = 0; // Abstand von Y0
    t9c2.WindowTab = 100;
    t9c2.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t9c2.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
    t9c2.WindowY0 = 0;
    t9c2.WindowY1 = 69;

    t9pCompass.Image = &t9screenCompass;
    t9pCompass.WindowNumberOfTextLines = 1;
    t9pCompass.WindowLineSpacing = 100; // Abstand von Y0
    t9pCompass.WindowTab = 100;
    t9pCompass.WindowX0 = 0;
    t9pCompass.WindowX1 = 1600-1;
    t9pCompass.WindowY0 = 0;
    t9pCompass.WindowY1 = 100-1;

    init_t9_compass();
}

uint8_t t9_test_customview_warnings(void)
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

void t9_refresh(void)
{
    static uint8_t last_mode = MODE_SURFACE;

    SStateList status;
    get_globalStateList(&status);

    if(stateUsed->mode != MODE_DIVE)
    {
        last_mode = MODE_SURFACE;
        settingsGetPointer()->design = 7;
        if(t9screen.FBStartAdress)
        {
            releaseFrame(23,t9screen.FBStartAdress);
            t9screen.FBStartAdress = 0;
        }
        return;
    }

    if(status.base != BaseHome)
        return;

    t9screen.FBStartAdress = getFrame(23);
    t9_background.pointer = 0;

    if(stateUsed->mode == MODE_DIVE)
    {
        if(last_mode != MODE_DIVE)
        {
            last_mode = MODE_DIVE;
            /* lower left corner primary */
//			t9_selection_custom_field = settingsGetPointer()->tX_userselectedLeftLowerCornerPrimary;
            t9_selection_custom_field = 0;
            /* custom view primary OR debug if automatic return is off */
            if((settingsGetPointer()->tX_customViewTimeout == 0) && (settingsGetPointer()->showDebugInfo))
                t9_selection_customview = CVIEW_noneOrDebug;
            else
                t9_selection_customview = settingsGetPointer()->tX_customViewPrimary;
        }

        if(status.page == PageSurface)
            set_globalState(StD);

        t9_refresh_divemode();
    }

    if(t9_background.pointer)
    {
        GFX_SetFrameTop(t9screen.FBStartAdress);
        GFX_SetFrameBottom(t9_background.pointer,t9_background.x0 , t9_background.y0, t9_background.width, t9_background.height);
    }
    else
        GFX_SetFramesTopBottom(t9screen.FBStartAdress, NULL,480);

    releaseAllFramesExcept(23,t9screen.FBStartAdress);
}


void original_t9_refresh(void)
{
    static uint8_t last_mode = MODE_SURFACE;

//	uint32_t oldScreen;//, oldPlugin;
    SStateList status;
    get_globalStateList(&status);


    if(stateUsed->mode == MODE_DIVE)
    {
        if(last_mode != MODE_DIVE)
        {
            last_mode = MODE_DIVE;
            /* lower left corner primary */
            t9_selection_custom_field = settingsGetPointer()->tX_userselectedLeftLowerCornerPrimary;
            /* custom view primary OR debug if automatic return is off */
            if((settingsGetPointer()->tX_customViewTimeout == 0) && (settingsGetPointer()->showDebugInfo))
                t9_selection_customview = CVIEW_noneOrDebug;
            else
                t9_selection_customview = settingsGetPointer()->tX_customViewPrimary;
        }
        if(status.page == PageSurface)
            set_globalState(StD);

        t9_refresh_divemode();
    }
    else
    {
        if(last_mode != MODE_SURFACE)
        {
            last_mode = MODE_SURFACE;
// CHANGE HERE
        }
        if(status.page == PageDive)
            set_globalState(StS);
    }

    if(status.base == BaseHome)
    {
        if(t9_background.pointer)
        {
            GFX_SetFrameTop(t9screen.FBStartAdress);
            GFX_SetFrameBottom(t9_background.pointer,t9_background.x0 , t9_background.y0, t9_background.width, t9_background.height);
        }
        else
            GFX_SetFramesTopBottom(t9screen.FBStartAdress, NULL,480);
    }

    releaseAllFramesExcept(23,t9screen.FBStartAdress);
}

/* Private functions ---------------------------------------------------------*/


void t9_show_customview_warnings(void)
{
    char text[256];
    uint8_t textpointer, lineFree;

    text[0] = '\025';
    text[1] = '\f';
    text[2] = '\001';
    text[3] = TXT_Warning;
    text[4] = 0;
    GFX_write_string(&FontT42,&t9cH,text,0);

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
    GFX_write_string(&FontT48,&t9cW,text,1);
}


void t9_set_customview_to_primary(void)
{
    if(stateUsed->mode == MODE_DIVE)
            t9_selection_customview = settingsGetPointer()->tX_customViewPrimary;
}


void t9_change_customview(void)
{
    const uint8_t *pViews;

    if(stateUsed->mode == MODE_DIVE)
        pViews = t9_customviewsDive;

    while((*pViews != CVIEW_END) && (*pViews != t9_selection_customview))
        {pViews++;}

    if(*pViews < CVIEW_END)
        pViews++;
    else
    {
        if(stateUsed->mode == MODE_DIVE)
            pViews = t9_customviewsDive;
    }

//	if((*pViews == CVIEW_Scooter) && (getLicence() != LICENCEBONEX))
//		pViews++;

    t9_selection_customview = *pViews;
}


uint8_t t9_get_length_of_customtext(void)
{
    uint8_t i = 0;
    settingsGetPointer()->customtext[60-1] = 0;
    while(settingsGetPointer()->customtext[i] > 0)
        i++;
    return i;
}


void t9_refresh_customview(void)
{
    if((t9_selection_customview == CVIEW_Scooter) && (getLicence() != LICENCEBONEX))
        t9_change_customview();
    if((t9_selection_customview == CVIEW_sensors) &&(stateUsed->diveSettings.ccrOption == 0))
        t9_change_customview();
    if((t9_selection_customview == CVIEW_sensors_mV) &&(stateUsed->diveSettings.ccrOption == 0))
        t9_change_customview();
    if((t9_selection_customview == CVIEW_sensors) &&(stateUsed->diveSettings.ccrOption == 0))
        t9_change_customview();

    char text[256];
    uint16_t textpointer = 0;
    int16_t start;
//	int16_t shiftWindowY0;
    RTC_DateTypeDef Sdate;
    RTC_TimeTypeDef Stime;
    float fPpO2limitHigh, fPpO2limitLow, fPpO2ofGasAtThisDepth; // CVIEW_Gaslist
    const SGasLine * pGasLine; // CVIEW_Gaslist
    uint8_t oxygen, helium; // CVIEW_Gaslist
    float depth, surface, fraction_nitrogen, fraction_helium, ead, end; // CVIEW_EADTime

    switch(t9_selection_customview)
    {
    case CVIEW_noneOrDebug:
        if(settingsGetPointer()->showDebugInfo)
        {
            // header
            strcpy(text,"\032\f\001Debug");
            GFX_write_string(&FontT42,&t9cH,text,0);
            // content
            t9_debug();
        }
    break;
    case CVIEW_SummaryOfLeftCorner:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_Summary);
        GFX_write_string(&FontT42,&t9cH,text,0);
        // content
        t9_SummaryOfLeftCorner();
    break;
    case CVIEW_Scooter:
        snprintf(text,100,"\032\f\001Scooter");
        GFX_write_string(&FontT42,&t9cH,text,0);
        t9_scooter();
    break;
    case CVIEW_Gaslist:
        // a lot of code taken from tMenuGas.c
        // header
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_Gaslist);
        GFX_write_string(&FontT42,&t9cH,text,0);
        // content
        textpointer = 0;
        t9cY0free.WindowY0 = t9cC.WindowY0 - 10;
        t9cY0free.WindowLineSpacing = 48+9;
        t9cY0free.WindowNumberOfTextLines = 5; // NUM_GASES == 5
        t9cY0free.WindowTab = 420;

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

            }
            GFX_write_string(&FontT42, &t9cY0free, text, gasId);
        }
    break;
    case CVIEW_EADTime:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_Info );
        GFX_write_string(&FontT42,&t9cH,text,0);
        textpointer = 0;

        t9cY0free.WindowY0 = t9cC.WindowY0 - 10;
        t9cY0free.WindowLineSpacing = 48;
        t9cY0free.WindowNumberOfTextLines = 6;

    // time
        snprintf(text,100,"\032\001%c%c",TXT_2BYTE,TXT2BYTE_Clock );
        GFX_write_string(&FontT42, &t9cY0free, text, 1);

        translateDate(stateRealGetPointer()->lifeData.dateBinaryFormat, &Sdate);
        translateTime(stateRealGetPointer()->lifeData.timeBinaryFormat, &Stime);
        if(Stime.Seconds % 2)
            textpointer += snprintf(&text[textpointer],100,"\030\001%02d:%02d",Stime.Hours,Stime.Minutes);
        else
            textpointer += snprintf(&text[textpointer],100,"\030\001%02d\031:\030%02d",Stime.Hours,Stime.Minutes);
        GFX_write_string(&FontT42, &t9cY0free, text, 2);

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
        GFX_write_string(&FontT42, &t9cY0free, text, 3);
        snprintf(text,100,"\030\001%01.1fm", ead);
        GFX_write_string(&FontT42, &t9cY0free, text, 4);

        snprintf(text,100,"\032\001END");
        GFX_write_string(&FontT42, &t9cY0free, text, 5);
        snprintf(text,100,"\030\001%01.1fm", end);
        GFX_write_string(&FontT42, &t9cY0free, text, 6);
    break;
    case CVIEW_Profile:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_Profile);
        GFX_write_string(&FontT42,&t9cH,text,0);
        textpointer = 0;
        t9_miniLiveLogProfile();
    break;
    case CVIEW_Tissues:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_Tissues);
        GFX_write_string(&FontT42,&t9cH,text,0);
        textpointer = 0;
        t9_tissues(stateUsed);
    break;
    case CVIEW_sensors:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_O2monitor);
        GFX_write_string(&FontT42,&t9cH,text,0);
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
        t9cC.WindowLineSpacing = 95;
        t9cC.WindowNumberOfTextLines = 3;
        text[textpointer] = 0;
        GFX_write_string(&FontT105,&t9cC,text,1);
    break;
    case CVIEW_sensors_mV:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE,TXT2BYTE_O2voltage);
        GFX_write_string(&FontT42,&t9cH,text,0);
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
        t9cC.WindowLineSpacing = 95;
        t9cC.WindowNumberOfTextLines = 3;
        text[textpointer] = 0;
        GFX_write_string(&FontT48,&t9cC,text,1);
    break;
    case CVIEW_Compass:
    default:
        snprintf(text,100,"\032\f\001%c%c",TXT_2BYTE, TXT2BYTE_Compass);
        GFX_write_string(&FontT42,&t9cH,text,0);
        t9_compass((uint16_t)stateUsed->lifeData.compass_heading, stateUsed->diveSettings.compassHeading);
        t9cY0free.WindowY0 = 230;
        t9cY0free.WindowX0 += 15;
        snprintf(text,100,"\030\001%03i`",(uint16_t)stateUsed->lifeData.compass_heading);
        GFX_write_string(&FontT54,&t9cY0free,text,0);
        t9cY0free.WindowX0 -= 15;
    break;
    case CVIEW_Decolist:
        snprintf(text,100,"\032\f\001%c%c", TXT_2BYTE, TXT2BYTE_Decolist);
        GFX_write_string(&FontT42,&t9cH,text,0);

        const SDecoinfo * pDecoinfo;
        uint8_t depthNext, depthLast, depthSecond, depthInc;

        if(stateUsed->diveSettings.deco_type.ub.standard == GF_MODE)
            pDecoinfo = &stateUsed->decolistBuehlmann;
        else
            pDecoinfo = &stateUsed->decolistVPM;

        depthLast 		= (uint8_t)(stateUsed->diveSettings.last_stop_depth_bar * 10);
        depthSecond 	= (uint8_t)(stateUsed->diveSettings.input_second_to_last_stop_depth_bar * 10);
        depthInc 			= (uint8_t)(stateUsed->diveSettings.input_next_stop_increment_depth_bar * 10);

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
                textpointer += snprintf(&text[textpointer],20,"\030\034   %2u\016\016m\017%3i'\n\r",depthNext, (pDecoinfo->output_stop_length_seconds[i]+59)/60);
            else
                textpointer += snprintf(&text[textpointer],20,"\031\034   %2u\016\016m\017\n\r",depthNext);
            if(textpointer > 200) break;
        }
        t9cY0free.WindowY0 = t9cC.WindowY0 - 10;
        t9cY0free.WindowLineSpacing = 48;
        t9cY0free.WindowNumberOfTextLines = 6;
        GFX_write_string(&FontT42, &t9cY0free, text, 1);
    break;
    }
}

/* DIVE MODE
 */
void t9_refresh_divemode(void)
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
        depth = (depthThisCall + t9_depthLastCall[0] + t9_depthLastCall[1] + t9_depthLastCall[2] + t9_depthLastCall[3] + t9_depthLastCall[4] + t9_depthLastCall[5] + t9_depthLastCall[6] + t9_depthLastCall[7] + t9_depthLastCall[8]) / 10.0f;

        idt9_depthLastCall++;
        if(idt9_depthLastCall >= 9)
            idt9_depthLastCall = 0;
        t9_depthLastCall[idt9_depthLastCall] = depthThisCall;
    }
    else
    {
        depth = (depthThisCall + t9_depthLastCall[0] + t9_depthLastCall[1] + t9_depthLastCall[2]) / 4.0f;

        idt9_depthLastCall++;
        if(idt9_depthLastCall >= 3)
            idt9_depthLastCall = 0;
        t9_depthLastCall[idt9_depthLastCall] = depthThisCall;
    }

    if(depth <= 0.3f)
        depth = 0;

    snprintf(TextL1,TEXTSIZE,"\032\f%c",TXT_Depth);
    GFX_write_string(&FontT24,&t9l2,TextL1,0);

    if( depth < 100)
        snprintf(TextL1,TEXTSIZE,"\020%01.1f",depth);
    else
        snprintf(TextL1,TEXTSIZE,"\020%01.0f",depth);

    t9_colorscheme_mod(TextL1);
    GFX_write_string(&FontT105,&t9l2,TextL1,1);

    /* max depth */
    snprintf(TextL2,TEXTSIZE,"\032\f%c",TXT_MaxDepth);
    GFX_write_string(&FontT24,&t9l3,TextL2,0);

    if(unit_depth_float(stateUsed->lifeData.max_depth_meter) < 100)
        snprintf(TextL2,TEXTSIZE,"\020%01.1f",unit_depth_float(stateUsed->lifeData.max_depth_meter));
    else
        snprintf(TextL2,TEXTSIZE,"\020%01.0f",unit_depth_float(stateUsed->lifeData.max_depth_meter));

    t9_colorscheme_mod(TextL2);
    GFX_write_string(&FontT105,&t9l3,TextL2,1);

    /* ascentrate graph */
    if(stateUsed->lifeData.ascent_rate_meter_per_min > 0)
    {
        start.y = t9l1.WindowY0 - 1;
        for(int i = 0; i<4;i++)
        {
            start.y += 5*6;
            stop.y = start.y;
            start.x = CUSTOMBOX_LINE_LEFT - 1;
            stop.x = start.x - 17;
            GFX_draw_line(&t9screen, start, stop, 0);
//			start.x = CUSTOMBOX_LINE_RIGHT + 2; old right too
//			stop.x = start.x + 17;
//			GFX_draw_line(&t9screen, start, stop, 0);
        }
        // new thick bar design Sept. 2015
        start.x = CUSTOMBOX_LINE_LEFT - CUSTOMBOX_OUTSIDE_OFFSET - 3 - 5;
        stop.x = start.x;
        start.y = t9l1.WindowY0 - 1;
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

        GFX_draw_thick_line(12,&t9screen, start, stop, color);
    }
    snprintf(TextL2,TEXTSIZE,"\f%.1f m/min",stateUsed->lifeData.ascent_rate_meter_per_min);

    /* divetime */
    if(stateUsed->lifeData.counterSecondsShallowDepth)
    {
        snprintf(TextR1,TEXTSIZE,"\f\002\136 %u:%02u",TimeoutTime.Minutes, TimeoutTime.Seconds);
        GFX_write_string(&FontT24,&t9r2,TextR1,0);
    }
    else
    {
        snprintf(TextR1,TEXTSIZE,"\032\f\002%c",TXT_Divetime);
        GFX_write_string(&FontT24,&t9r2,TextR1,0);
    }

    if(Divetime.Minutes < 1000)
        snprintf(TextR1,TEXTSIZE,"\020\016\002%u:%02u",Divetime.Minutes, Divetime.Seconds);
    else
        snprintf(TextR1,TEXTSIZE,"\020\016\002%u'",Divetime.Minutes);
    t9_colorscheme_mod(TextR1);
    GFX_write_string(&FontT105,&t9r2,TextR1,1);

    /* next deco stop */
    if(nextstopDepthMeter)
    {
        snprintf(TextR2,TEXTSIZE,"\032\f\002%c",TXT_Decostop);
        GFX_write_string(&FontT24,&t9r3,TextR2,0);
        textlength = snprintf(TextR2,TEXTSIZE,"\020\002%um %u'",nextstopDepthMeter,(nextstopLengthSeconds+59)/60);
        t9_colorscheme_mod(TextR2);
        if(time_elapsed_ms(pDecoinfo->tickstamp, HAL_GetTick()) > MAX_AGE_DECOINFO_MS)
            TextR2[0] = '\021';
        if(textlength <= 8)
            GFX_write_string(&FontT105,&t9r3,TextR2,1);
        else
            GFX_write_string(&FontT54,&t9r3,TextR2,1);
    }
    else
    if(SafetyStopTime.Total && (depth > timer_Safetystop_GetDepthUpperLimit()))
    {
        snprintf(TextR2,TEXTSIZE,"\032\f\002%c%c",TXT_2BYTE,TXT2BYTE_SafetyStop2);
        GFX_write_string(&FontT24,&t9r3,TextR2,0);
        snprintf(TextR2,TEXTSIZE,"\020\016\002%u:%02u",SafetyStopTime.Minutes,SafetyStopTime.Seconds);
        t9_colorscheme_mod(TextR2);
        GFX_write_string(&FontT105,&t9r3,TextR2,1);
    }

    /* tts - option 1
     * ndl - option 2
     * empty - option 3 */
    if(pDecoinfo->output_time_to_surface_seconds)
    {
        snprintf(TextR3,TEXTSIZE,"\032\f\002%c",TXT_TTS);
        GFX_write_string(&FontT24,&t9r4,TextR3,0);
        if(pDecoinfo->output_time_to_surface_seconds < 1000 * 60)
            snprintf(TextR3,TEXTSIZE,"\020\002%i'",(pDecoinfo->output_time_to_surface_seconds + 30)/ 60);
        else
            snprintf(TextR3,TEXTSIZE,"\020\002%ih",pDecoinfo->output_time_to_surface_seconds / 3600);
        t9_colorscheme_mod(TextR3);
        if(time_elapsed_ms(pDecoinfo->tickstamp, HAL_GetTick()) > MAX_AGE_DECOINFO_MS)
            TextR2[0] = '\021';
        GFX_write_string(&FontT105,&t9r4,TextR3,0);
    }
    else
    if(pDecoinfo->output_ndl_seconds)
    {
        snprintf(TextR3,TEXTSIZE,"\032\f\002%c",TXT_Nullzeit);
        GFX_write_string(&FontT24,&t9r4,TextR3,0);
        if(pDecoinfo->output_ndl_seconds < 1000 * 60)
            snprintf(TextR3,TEXTSIZE,"\020\002%i'",pDecoinfo->output_ndl_seconds/60);
        else
            snprintf(TextR3,TEXTSIZE,"\020\002%ih",pDecoinfo->output_ndl_seconds/3600);
        t9_colorscheme_mod(TextR3);
        if(time_elapsed_ms(pDecoinfo->tickstamp, HAL_GetTick()) > MAX_AGE_DECOINFO_MS)
            TextR2[0] = '\021';
        GFX_write_string(&FontT105,&t9r4,TextR3,0);
    }

    uint16_t scooterSpeedLocal;
    uint8_t scooterResidualCapacity;

    scooterSpeedLocal = unit_speed_integer(stateUsed->lifeData.scooterSpeed);
    scooterResidualCapacity = stateUsed_scooterRemainingBattCapacity();

    /* scooter global for both */
    uint16_t ageInMilliSeconds;
    ageInMilliSeconds = stateUsed->lifeData.scooterAgeInMilliSeconds;
    if(!ageInMilliSeconds)
        ageInMilliSeconds = 9999;

    /* scooter battery */
    snprintf(TextR1,TEXTSIZE,"\032\f\002%c%c",TXT_2BYTE, TXT2BYTE_ScooterRestkapazitaet);
    GFX_write_string(&FontT24,&t9r1,TextR1,0);
    snprintf(TextR3,TEXTSIZE,"\020\002%u\016\016%%\017",scooterResidualCapacity);
    if(ageInMilliSeconds > 1500)
    {
        for(int i=0; i < TEXTSIZE -2; i++)
        {
            if(TextR3[i] == '\020')
                TextR3[i] = '\031';
        }
    }
    else
        t9_colorscheme_mod(TextR3);
    GFX_write_string(&FontT105,&t9r1,TextR3,1);

    /*  scooter speed */
    textlength = snprintf(TextR3,TEXTSIZE-7,"\032\f%c%c",TXT_2BYTE, TXT2BYTE_ScooterSpeed);
    if(settingsGetPointer()->nonMetricalSystem == 0)
    {
        snprintf(&TextR3[textlength],8," m/min");
    }
    else
    {
        snprintf(&TextR3[textlength],8," ft/min");
    }
    GFX_write_string(&FontT24,&t9l1,TextR3,0);
    snprintf(TextR3,TEXTSIZE,"\020%u",scooterSpeedLocal);
    if(ageInMilliSeconds > 1500)
    {
        for(int i=0; i < TEXTSIZE -2; i++)
        {
            if(TextR3[i] == '\020')
                TextR3[i] = '\031';
        }
    }
    else
        t9_colorscheme_mod(TextR3);
    GFX_write_string(&FontT105,&t9l1,TextR3,1);



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
        GFX_write_string_color(&FontT48,&t9c2,TextR1,0,CLUT_WarningYellow);
    }
    else
    if(get_globalState() == StDMSPT)
    {
        textPointer = 0;
        TextR1[textPointer++] = '\a';
        TextR1[textPointer++] = '\001';
        TextR1[textPointer++] = ' ';
        textPointer += snprintf(&TextR1[textPointer],5,"%f01.2",((float)(stateUsed->diveSettings.setpoint[actualBetterSetpointId()].setpoint_cbar))/100);
        TextR1[textPointer++] = '?';
        TextR1[textPointer++] = ' ';
        TextR1[textPointer++] = 0;
        GFX_write_string_color(&FontT48,&t9c2,TextR1,0,CLUT_WarningYellow);
    }
    else
    if(get_globalState() == StDMENU)
    {
        snprintf(TextR1,TEXTSIZE,"\a\001%c%c", TXT_2BYTE, TXT2BYTE_DiveMenuQ);
        GFX_write_string_color(&FontT48,&t9c2,TextR1,0,CLUT_WarningYellow);
    }
    else
    if(get_globalState() == StDSIM1)
    {
        snprintf(TextR1,TEXTSIZE,"\a\001%c%c", TXT_2BYTE, TXT2BYTE_DiveQuitQ);
        GFX_write_string_color(&FontT48,&t9c2,TextR1,0,CLUT_WarningYellow);
    }
    else
    if(get_globalState() == StDSIM2)
    {
        snprintf(TextR1,TEXTSIZE,"\a\001" " Sim:-1m ");
        GFX_write_string_color(&FontT48,&t9c2,TextR1,0,CLUT_WarningYellow);
        snprintf(TextR1,TEXTSIZE,"\a\f %u m",simulation_get_aim_depth());
        GFX_write_string_color(&FontT42,&t9l1,TextR1,0,CLUT_WarningYellow);

    }
    else
    if(get_globalState() == StDSIM3)
    {
        snprintf(TextR1,TEXTSIZE,"\a\001" " Sim:+1m ");
        GFX_write_string_color(&FontT48,&t9c2,TextR1,0,CLUT_WarningYellow);
        snprintf(TextR1,TEXTSIZE,"\a\f %u m",simulation_get_aim_depth());
        GFX_write_string_color(&FontT42,&t9l1,TextR1,0,CLUT_WarningYellow);
    }
    else
    if(get_globalState() == StDSIM4)
    {
        snprintf(TextR1,TEXTSIZE,"\a\001" " Sim:+5' ");
        GFX_write_string_color(&FontT48,&t9c2,TextR1,0,CLUT_WarningYellow);
        snprintf(TextR1,TEXTSIZE,"\a\f %u m",simulation_get_aim_depth());
        GFX_write_string_color(&FontT42,&t9l1,TextR1,0,CLUT_WarningYellow);
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
            GFX_write_string_color(&FontT48,&t9c2,TextC2,0,CLUT_WarningYellow);
        }
        else
        {
            t9_colorscheme_mod(TextC2);
            GFX_write_string(&FontT48,&t9c2,TextC2,0); // T54 has only numbers
        }

        if(stateUsed->diveSettings.ccrOption)
        {
            if(stateUsed->diveSettings.diveMode == DIVEMODE_CCR)
            {
                snprintf(TextC2,TEXTSIZE,"\020%01.2f",stateUsed->lifeData.ppO2);
                if(stateUsed->warnings.betterSetpoint && warning_count_high_time && (stateUsed->diveSettings.diveMode == DIVEMODE_CCR))
                {
                    TextC2[0] = '\a'; // inverse instead of color \020
                    GFX_write_string_color(&FontT48,&t9c2,TextC2,0,CLUT_WarningYellow);
                }
                else
                {
                    t9_colorscheme_mod(TextC2);
                    GFX_write_string(&FontT48,&t9c2,TextC2,0);
                }
            }
        }
        else if(settingsGetPointer()->alwaysShowPPO2)
        {
            snprintf(TextC2,TEXTSIZE,"\020%01.2f",stateUsed->lifeData.ppO2);
            t9_colorscheme_mod(TextC2);
            GFX_write_string(&FontT48,&t9c2,TextC2,0);
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
        GFX_write_string_color(&FontT48,&t9c1,TextC2,0,CLUT_WarningRed);
    }
    else
    {
        if(stateUsed->warnings.aGf)
        {
            GFX_write_string_color(&FontT48,&t9c1,"\f" "aGF",0,CLUT_WarningYellow);
        }
        else if(stateUsed->diveSettings.deco_type.ub.standard == GF_MODE)
        {
            GFX_write_string(&FontT48,&t9c1,"\027\f" "GF",0);
        }
        else
        {
            GFX_write_string(&FontT48,&t9c1,"\027\f" "VPM",0);
        }

        if(stateUsed->diveSettings.diveMode == DIVEMODE_CCR)
            GFX_write_string(&FontT24,&t9c1,"\027\f\002" "CCR",0);
        //		GFX_write_string(&FontT24,&t9c1,"\f\177\177\x80" "CCR",0);
        else
        if(stateUsed->diveSettings.ccrOption)
            GFX_write_string(&FontT24,&t9c1,"\f\002\024" "Bailout",0);
        //		GFX_write_string(&FontT24,&t9c1,"\f\177\177\x80\024" "Bailout",0);
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
        GFX_write_string(&Batt24,&t9batt,TextC1,0);

        if((stateUsed->lifeData.battery_charge > 0) && (stateUsed->lifeData.battery_charge < 140))
        {
            snprintf(TextC1,16,"\004\025\f\002%u%%",(uint8_t)stateUsed->lifeData.battery_charge);
            if(warning_count_high_time)
                TextC1[0] = '\a';
            GFX_write_string(&FontT24,&t9batt,TextC1,0);
        }
    }
    else
    {
        t9_colorscheme_mod(TextC1);
        GFX_write_string(&Batt24,&t9batt,TextC1,0);

        if((stateUsed->lifeData.battery_charge > 0) && (stateUsed->lifeData.battery_charge < 140))
        {
            snprintf(TextC1,16,"\020\f\002%u%%",(uint8_t)stateUsed->lifeData.battery_charge);
            t9_colorscheme_mod(TextC1);
            GFX_write_string(&FontT24,&t9batt,TextC1,0);
        }
    }

    /* customizable left lower corner */
    t9_refresh_divemode_userselected_left_lower_corner();


    /* customview - option 1
     * warning - option 2 */
    if(stateUsed->warnings.numWarnings)
        customview_warnings = t9_test_customview_warnings();

    t9_background.pointer = NULL;
    if(customview_warnings && warning_count_high_time)
        t9_show_customview_warnings();
    else
        t9_refresh_customview();


    /* the frame */
    t9_draw_frame(1,1, CLUT_DIVE_pluginbox, CLUT_DIVE_FieldSeperatorLines);
}


void t9_set_field_to_primary(void)
{
    if(stateUsed->mode == MODE_DIVE)
            t9_selection_custom_field = settingsGetPointer()->tX_userselectedLeftLowerCornerPrimary;
}


void t9_change_field(void)
{
    const uint8_t minVal = 0;
    const uint8_t maxVal = 7;

    t9_selection_custom_field++;
    if(t9_selection_custom_field > maxVal)
    t9_selection_custom_field = minVal;
}


void t9_refresh_divemode_userselected_left_lower_corner(void)
{
    // zero is scooter temperature :-)

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

    float scooterTemperatureLocal;
    uint16_t ageInMilliSeconds;

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

    switch(t9_selection_custom_field)
    {
    /* scooter temp. */
    case 0:
        scooterTemperatureLocal = unit_temperature_float(((float)(stateUsed->lifeData.scooterTemperature)) / 10.0f);
        headerText[2] = TXT_2BYTE;
        headerText[3] = TXT2BYTE_ScooterTemperature;
        textpointer = snprintf(text,TEXTSIZE,"\020\016%01.1f \140",scooterTemperatureLocal); // "\016\016%01.1f `" + C or F
        if(settingsGetPointer()->nonMetricalSystem == 0)
            text[textpointer++] = 'C';
        else
            text[textpointer++] = 'F';
        text[textpointer++] = 0;
        tinyHeaderFont = 0;
        // connection active
        ageInMilliSeconds = stateUsed->lifeData.scooterAgeInMilliSeconds;
        if(!ageInMilliSeconds)
            ageInMilliSeconds = 9999;
        if(ageInMilliSeconds > 1500)
        {
            for(int i=0; i < TEXTSIZE -2; i++)
            {
                if(text[i] == '\020')
                    text[i] = '\031';
            }
        }
        break;

    /* Temperature */
    case 1:
    default:
        // mean value
        temperatureThisCall = unit_temperature_float(stateUsed->lifeData.temperature_celsius);
        temperature = (temperatureThisCall + t9_temperatureLastCall[0] + t9_temperatureLastCall[1] + t9_temperatureLastCall[2]) / 4.0f;
        idt9_temperatureLastCall++;
        if(idt9_temperatureLastCall >= 3)
            idt9_temperatureLastCall = 0;
        t9_temperatureLastCall[idt9_temperatureLastCall] = temperatureThisCall;
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
        snprintf(text,TEXTSIZE,"\020\016\016%u:%02u\n\r%01.1f",Stopwatch.Minutes, Stopwatch.Seconds,fAverageDepth);
        tinyHeaderFont = 1;
        line = 1;
        break;

    /* Ceiling */
    case 5:
        headerText[2] = TXT_Ceiling;
        if(pDecoinfoStandard->output_ceiling_meter <= 99.9f)
            snprintf(text,TEXTSIZE,"\020%01.1f",pDecoinfoStandard->output_ceiling_meter);
        else
            snprintf(text,TEXTSIZE,"\020%01.0f",pDecoinfoStandard->output_ceiling_meter);
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

    /* scooter voltage*/
    case 77: // used as 7: alternative for Bonex
        headerText[2] = TXT_2BYTE;
        headerText[3] = TXT2BYTE_ScooterVolt;
        if(stateUsed->lifeData.scooterSpannung < 99)
        {
            snprintf(text,TEXTSIZE,"\020\016%01.2f",stateUsed->lifeData.scooterSpannung);
            ageInMilliSeconds = stateUsed->lifeData.scooterAgeInMilliSeconds;
            if(!ageInMilliSeconds)
                ageInMilliSeconds = 9999;
            if(ageInMilliSeconds > 1500)
            {
                for(int i=0; i < TEXTSIZE -2; i++)
                {
                    if(text[i] == '\020')
                        text[i] = '\031';
                }
            }
        }
        else
        {
            text[0] = 0;
        }
        break;

    }
    if(headerText[2] == TXT_2BYTE)
        headerText[4] = 0;
    else
        headerText[3] = 0;

    if(!tinyHeaderFont)
        GFX_write_string(&FontT24,&t9l4,headerText,0);

    t9_colorscheme_mod(text);
    GFX_write_string(&FontT105,&t9l4,text,line);
}

/* Private functions ---------------------------------------------------------*/

uint8_t t9_customtextPrepare(char * text)
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
void t9_colorscheme_mod(char *text)
{
    if((text[0] == '\020') && !GFX_is_colorschemeDiveStandard())
    {
        text[0] = '\027';
    }
}

void t9_draw_frame(_Bool PluginBoxHeader, _Bool LinesOnTheSides, uint8_t colorBox, uint8_t colorLinesOnTheSide)
{
    point_t LeftLow, WidthHeight;
    point_t start, stop;

    // plugin box
    LeftLow.x = CUSTOMBOX_LINE_LEFT;
    WidthHeight.x = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_LINE_LEFT;
    LeftLow.y = 60;
    WidthHeight.y = 440 - LeftLow.y;
    GFX_draw_box(&t9screen, LeftLow, WidthHeight, 1, colorBox);

    if(PluginBoxHeader)
    {
            // plugin box - header
            start.x = CUSTOMBOX_LINE_LEFT;
            stop.x = CUSTOMBOX_LINE_RIGHT;
            stop.y = start.y = 440 - 60;
            GFX_draw_line(&t9screen, start, stop, colorBox);
    }

    if(LinesOnTheSides)
    {
        // aufteilung links
        start.x = 0;
        stop.x = CUSTOMBOX_LINE_LEFT;
        stop.y = start.y = t9l1.WindowY0 - 1;
        GFX_draw_line(&t9screen, start, stop, colorLinesOnTheSide);
        stop.y = start.y = t9l2.WindowY0 -1;
        GFX_draw_line(&t9screen, start, stop, colorLinesOnTheSide);
        stop.y = start.y = t9l3.WindowY0 -1;
        GFX_draw_line(&t9screen, start, stop, colorLinesOnTheSide);

        // aufteilung rechts
        start.x = CUSTOMBOX_LINE_RIGHT;
        stop.x = 799;
        stop.y = start.y = t9l1.WindowY0 - 1;
        GFX_draw_line(&t9screen, start, stop, colorLinesOnTheSide);
        stop.y = start.y = t9l2.WindowY0 - 1;
        GFX_draw_line(&t9screen, start, stop, colorLinesOnTheSide);
        stop.y = start.y = t9l3.WindowY0 - 1;
        GFX_draw_line(&t9screen, start, stop, colorLinesOnTheSide);
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


point_t t9_compass_circle(uint8_t id, uint16_t degree)
{
    float fCos, fSin;
    const float piMult =  ((2 * 3.14159) / 360);
//	const int radius[4] = {95,105,115,60};
    const int radius[4] = {95,105,115,100};
    const point_t offset = {.x = 400, .y = 250};

    static point_t r[4][360] = { 0 };

    if(r[0][0].y == 0)
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
void t9_tissues(const SDiveState * pState)
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


    /* N2 */
    t9cY0free.WindowLineSpacing = 28 + 48 + 14;
    t9cY0free.WindowY0 = t9cH.WindowY0 - 5 - 2 * t9cY0free.WindowLineSpacing;
    t9cY0free.WindowNumberOfTextLines = 3;

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

    GFX_write_string(&FontT24, &t9cY0free, text, 1);

    start.y = t9cH.WindowY0 - 5;
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
        else if(value > CUSTOMBOX_SPACE_INSIDE)
            front = CUSTOMBOX_SPACE_INSIDE;
        else
                front = (uint16_t)value;

        change.x = start.x + front;
        if(change.x != start.x)
            GFX_draw_thick_line(1,&t9screen, start, change, CLUT_Font030);
        if(change.x != stop.x)
            GFX_draw_thick_line(1,&t9screen, change, stop, CLUT_Font031);

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
            GFX_draw_thick_line(1,&t9screen, start, change, CLUT_Font030);
        if(change.x != stop.x)
            GFX_draw_thick_line(1,&t9screen, change, stop, CLUT_Font031);

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
            GFX_draw_thick_line(1,&t9screen, start, change, color);
        if(change.x != stop.x)
            GFX_draw_thick_line(1,&t9screen, change, stop, CLUT_Font031);

        start.y -= 3;
    }

    /* where is the onload/offload limit for N2 and He */
    decom_get_inert_gases(pState->lifeData.pressure_ambient_bar, &pState->lifeData.actualGas, &percent_N2, &percent_He);
    partial_pressure_N2 =  (pState->lifeData.pressure_ambient_bar - WATER_VAPOUR_PRESSURE) * percent_N2;
    partial_pressure_He = (pState->lifeData.pressure_ambient_bar - WATER_VAPOUR_PRESSURE) * percent_He;

    // Nitrogen vertical bar
    start.y = t9cH.WindowY0 + 1 - 5;
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
    GFX_draw_thick_line(2,&t9screen, start, stop, CLUT_EverythingOkayGreen);


    // Helium vertical bar
    start.y = t9cH.WindowY0 + 1 - 5 - 3*16 - 28 - 14;
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
    GFX_draw_thick_line(2,&t9screen, start, stop, CLUT_EverythingOkayGreen);

    // Oxygen vertical bar
    start.y = t9cH.WindowY0 + 1 - 5 - 6*16 - 2*28 - 2*14;
    stop.y = start.y - (3 * 15) - 1;

    start.x = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET + cns100pixel;
    stop.x = start.x;
    GFX_draw_thick_line(2, &t9screen, start, stop, CLUT_WarningRed);
}


void t9_debug(void)
{
    char text[256+50];
    uint8_t textpointer = 0;

    t9cY0free.WindowLineSpacing = 28 + 48 + 14;
    t9cY0free.WindowY0 = t9cH.WindowY0 - 5 - 2 * t9cY0free.WindowLineSpacing;
    t9cY0free.WindowNumberOfTextLines = 3;

    textpointer += snprintf(&text[textpointer],50,"Ambient [bar]\n\r");
    textpointer += snprintf(&text[textpointer],50,"Surface [bar] + salt\n\r");
//	textpointer += snprintf(&text[textpointer],50,"Difference [mbar]\n\r");
    textpointer += snprintf(&text[textpointer],50,"ShallowCounter [s]\n\r");
    GFX_write_string(&FontT24, &t9cY0free, text, 1);

    t9cY0free.WindowY0 -= 52;
//  snprintf(text,60,"%0.2f\n\r%0.2f       %u%%\n\r%0.0f",stateUsed->lifeData.pressure_ambient_bar, stateUsed->lifeData.pressure_surface_bar, settingsGetPointer()->salinity, 1000 * (stateUsed->lifeData.pressure_ambient_bar-stateUsed->lifeData.pressure_surface_bar));
    snprintf(text,60,
        "%0.2f\n\r"
        "%0.2f       %u%%\n\r"
        "%u"
        ,stateUsed->lifeData.pressure_ambient_bar
        ,stateUsed->lifeData.pressure_surface_bar
        ,settingsGetPointer()->salinity
        ,stateUsed->lifeData.counterSecondsShallowDepth);
    GFX_write_string(&FontT42, &t9cY0free, text, 1);
}


void t9_SummaryOfLeftCorner(void)
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

    t9cY0free.WindowY0 = t9cC.WindowY0 - 10;
    t9cY0free.WindowLineSpacing = 48;
    t9cY0free.WindowNumberOfTextLines = 6;
    t9cY0free.WindowTab = 420;

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
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_ScooterTemperature;
    text[textpointer++] = '\017';
    text[textpointer++] = 0;
    t9cY0free.WindowX0 += 10;
    t9cY0free.WindowY0 += 10;
    GFX_write_string(&FontT24, &t9cY0free, text, 1);
    t9cY0free.WindowX0 -= 10;
    t9cY0free.WindowY0 -= 10;

    textpointer = 0;
    text[textpointer++] = '\t';
    textpointer += snprintf(&text[textpointer],10,"\020%01.2f",	stateUsed->lifeData.ppO2);
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = '\t';
    if(pDecoinfoStandard->output_ceiling_meter <= 99.9f)
        textpointer += snprintf(&text[textpointer],10,"\020%01.1f",pDecoinfoStandard->output_ceiling_meter);
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
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = '\t';
    textpointer += snprintf(&text[textpointer],15,"\020\016%01.0f \140",unit_temperature_float(((float)(stateUsed->lifeData.scooterTemperature)) / 10.0f)); // "\016\016%01.1f `" + C or F
    if(settingsGetPointer()->nonMetricalSystem == 0)
        text[textpointer++] = 'C';
    else
        text[textpointer++] = 'F';
    text[textpointer++] = 0;
    text[textpointer++] = 0;
    GFX_write_string(&FontT42, &t9cY0free, text, 1);
}


void t9_scooter(void)
{
    float scooterTemperatureLocal;
    uint16_t scooterSpeedLocal;
//	uint16_t scooterDrehzhl;
    uint8_t scooterResidualCapacity;
//	float scooterVoltage;
//	uint8_t scooterCurrent;
    //uint16_t scooterWattHours;
//	uint16_t bkpX0, bkpX1;

    uint16_t ageInMilliSeconds;
    uint8_t textSize;

    scooterTemperatureLocal = unit_temperature_float(((float)(stateUsed->lifeData.scooterTemperature)) / 10.0f);
    scooterSpeedLocal = unit_speed_integer(stateUsed->lifeData.scooterSpeed);
    scooterResidualCapacity = stateUsed_scooterRemainingBattCapacity();

//	scooterDrehzhl = stateUsed->lifeData.scooterDrehzahl;
//	scooterVoltage = stateUsed->lifeData.scooterSpannung;
//	scooterCurrent = stateUsed->lifeData.scooterAmpere;
//	scooterWattHours = stateUsed->lifeData.scooterWattstunden;

    ageInMilliSeconds = stateUsed->lifeData.scooterAgeInMilliSeconds;
    if(!ageInMilliSeconds)
        ageInMilliSeconds = 9999;

    char text[256+60];
    uint8_t textpointer = 0;

    t9cY0free.WindowLineSpacing = 28 + 48 + 14;
    t9cY0free.WindowY0 = t9cH.WindowY0 - 5 - 2 * t9cY0free.WindowLineSpacing;
    t9cY0free.WindowNumberOfTextLines = 3;

    // header
    text[textpointer++] = '\032';
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_ScooterRestkapazitaet;
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_ScooterTemperature;
    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_ScooterSpeed;
    text[textpointer++] = 0;
    GFX_write_string(&FontT24, &t9cY0free, text, 1);

/*
snprintf(text,60,
            "\032"
            "%0u" "\016\016 Wh used\017"
            ,stateUsed->lifeData.scooterWattstunden);
*/
if(ageInMilliSeconds > 1500)
    text[0] = '\031';
GFX_write_string(&FontT24, &t9cY0free, text, 1);

/*
snprintf(text,60,
            "\030"
            "\n\r"
            "\n\r"
            "%0u" "\022\016\016 rpm\017\030"
            ,stateUsed->lifeData.scooterDrehzahl);
GFX_write_string(&FontT24, &t9cY0free, text, 1);
*/
    // data
    t9cY0free.WindowY0 -= 52;
    if(settingsGetPointer()->nonMetricalSystem == 0)
    {
        textSize = snprintf(text,60,
            "\030"
            "%0u" "\022\016\016 %%\017\030"
            "\n\r"
            "%0.0f\140" "\022\016\016 C\017\030"
            "\n\r"
            "%u"  "\022\016\016 m/min\017\030"
            ,scooterResidualCapacity,scooterTemperatureLocal,scooterSpeedLocal);
    }
    else
    {
        textSize = snprintf(text,60,
            "\030"
            "%0u" "\022\016\016 %%\017\030"
            "\n\r"
            "%0.0f\140" "\022\016\016 Fht\017\030"
            "\n\r"
            "%u"  "\022\016\016 ft/min\017\030"
            ,scooterResidualCapacity,scooterTemperatureLocal,scooterSpeedLocal);
    }
    // connection active
    if(ageInMilliSeconds > 1500)
    {
        for(int i=0; i < textSize -2; i++)
        {
            if(text[i] == '\030')
                text[i] = '\031';
        }
    }
    // write data
    GFX_write_string(&FontT42, &t9cY0free, text, 1);

    // age stamp
    if(ageInMilliSeconds < 9999)
    {
        t9cY0free.WindowY0 -= 30;
        snprintf(text,60,
            "\021\001%u"
            ,ageInMilliSeconds);
        GFX_write_string(&FontT24, &t9cY0free, text, 0);
    }
}


void t9_scooter_May2016_01(void)
{
    float scooterTemperature;
    uint16_t scooterDrehzhl;
    uint8_t scooterResidualCapacity;
    float scooterSpeed;
    float scooterVoltage;
    uint8_t scooterCurrent;
//	uint16_t scooterWattHours;
    uint16_t bkpX0, bkpX1;

    uint16_t ageInMilliSeconds;
    uint8_t textSize;
// old	scooterStatus = bC_getData(0,&scooterTemperature,&scooterDrehzhl,&scooterResidualCapacity);

    scooterDrehzhl = stateUsed->lifeData.scooterDrehzahl;
    scooterTemperature = ((float)(stateUsed->lifeData.scooterTemperature)) / 10.0f;
    scooterResidualCapacity = stateUsed_scooterRemainingBattCapacity();

    scooterVoltage = stateUsed->lifeData.scooterSpannung;
    scooterCurrent = stateUsed->lifeData.scooterAmpere;
//	scooterWattHours = stateUsed->lifeData.scooterWattstunden;

    ageInMilliSeconds = stateUsed->lifeData.scooterAgeInMilliSeconds;
    if(!ageInMilliSeconds)
        ageInMilliSeconds = 9999;

    scooterSpeed = scooterDrehzhl * 80 / 3300;

    char text[256+60];

    t9cY0free.WindowLineSpacing = (28 + 48 + 14)/2;
    t9cY0free.WindowY0 = t9cH.WindowY0 - 5 - 5 * t9cY0free.WindowLineSpacing;
    t9cY0free.WindowNumberOfTextLines = 6;

    t9cY0free.WindowY0 -= 7;

    bkpX0 = t9cY0free.WindowX0;
    bkpX1 = t9cY0free.WindowX1;
    t9cY0free.WindowX0 = 430;

    textSize = snprintf(text,120,
        "\022\016\016"
        "%%"
        "\n\r"
        "celsius"
        "\n\r"
        "rpm"
        "\n\r"
        "m/min"
        "\n\r"
        "Ampere"
        "\n\r"
        "Volt"
//		"\n\r"
//		"Wh"
            );
    GFX_write_string(&FontT42, &t9cY0free, text, 1);

    t9cY0free.WindowX0 = bkpX0;
    t9cY0free.WindowX1 = 420;

    textSize = snprintf(text,120,
        "\030"
        "\002"
        "%0u"
        "\n\r"
        "\002"
        "%0.0f"
        "\n\r"
        "\002"
        "%0u"
        "\n\r"
        "\002"
        "%0.0f"
        "\n\r"
        "\002"
        "%0u"
        "\n\r"
        "\002"
        "%0.1f"
//		"\n\r"
//		"%0u"  "\022\016\016 Wh\017\030"
        ,scooterResidualCapacity,scooterTemperature,scooterDrehzhl,scooterSpeed
        ,scooterCurrent,scooterVoltage);//,scooterWattHours);

    if((ageInMilliSeconds > 1500) || (stateUsed->lifeData.scooterType == 0xFF))
    {
        for(int i=0; i < textSize -2; i++)
        {
            if(text[i] == '\030')
                text[i] = '\031';
        }
    }
    GFX_write_string(&FontT42, &t9cY0free, text, 1);

    t9cY0free.WindowX0 = bkpX0;
    t9cY0free.WindowX1 = bkpX1;

    t9cY0free.WindowY0 -= 30;
    snprintf(text,60,
        "\021\001%u"
        ,ageInMilliSeconds);
    GFX_write_string(&FontT24, &t9cY0free, text, 0);

}


void t9_compass(uint16_t ActualHeading, uint16_t UserSetHeading)
{
    uint16_t LeftBorderHeading, LineHeading;
    uint32_t offsetPicture;
    point_t start, stop, center;
    static int32_t LastHeading = 0;
    int32_t newHeading = 0;
    int32_t diff = 0;
    int32_t diff2 = 0;

    int32_t diffAbs = 0;
    int32_t diffAbs2 = 0;

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

    if (ActualHeading < 90)
        ActualHeading += 360;

    LeftBorderHeading = 2 * (ActualHeading - (CUSTOMBOX_SPACE_INSIDE/4));

    offsetPicture = LeftBorderHeading * t9screenCompass.ImageHeight * 2;

    t9_background.pointer = t9screenCompass.FBStartAdress+offsetPicture;
    t9_background.x0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t9_background.y0 = 65;
    t9_background.width = CUSTOMBOX_SPACE_INSIDE;
    t9_background.height = t9screenCompass.ImageHeight;

    start.x = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET + (CUSTOMBOX_SPACE_INSIDE/2);
    stop.x = start.x;
    start.y = 65;
    stop.y =  start.y + 55;
    GFX_draw_line(&t9screen, start, stop, CLUT_Font030);


    center.x = start.x;
    center.y = 300;

    stop.x = center.x + 44;
    stop.y = center.y + 24;


    while(ActualHeading > 359) ActualHeading -= 360;
    LineHeading = 360 - ActualHeading;
    GFX_draw_thick_line(9,&t9screen, t9_compass_circle(0,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font030); // North
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(9,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031); // Maintick
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(9,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(9,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031);

    LineHeading = 360 - ActualHeading;
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(5,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031); // Subtick
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(5,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(5,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(5,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031);

    LineHeading = 360 - ActualHeading;
    LineHeading += 22;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031); // Subtick
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031); // Subtick
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t9screen, t9_compass_circle(1,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_Font031);

    if(UserSetHeading)
    {
        LineHeading = UserSetHeading + 360 - ActualHeading;
        if(LineHeading > 359) LineHeading -= 360;
        GFX_draw_thick_line(9,&t9screen, t9_compass_circle(3,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_CompassUserHeadingTick);

        // Rï¿½ckpeilung, User Back Heading
        LineHeading = UserSetHeading + 360 + 180 - ActualHeading;
        if(LineHeading > 359) LineHeading -= 360;
        if(LineHeading > 359) LineHeading -= 360;
        GFX_draw_thick_line(9,&t9screen, t9_compass_circle(3,LineHeading),  t9_compass_circle(2,LineHeading), CLUT_CompassUserBackHeadingTick);
    }

    center.x = start.x;
    center.y = 250;
    GFX_draw_circle(&t9screen, center, 116, CLUT_Font030);
    GFX_draw_circle(&t9screen, center, 118, CLUT_Font030);
    GFX_draw_circle(&t9screen, center, 117, CLUT_Font030);
}


void init_t9_compass(void)
{
    t9screenCompass.FBStartAdress = getFrame(26);

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

    GFX_write_string(&FontT42,&t9pCompass,text,1);

    releaseAllFramesExcept(26,t9screenCompass.FBStartAdress);
}


void t9_miniLiveLogProfile(void)
{
    SWindowGimpStyle wintemp;
    wintemp.left = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    wintemp.right = wintemp.left + CUSTOMBOX_SPACE_INSIDE;
    wintemp.top = 480 - t9l1.WindowY0;
    wintemp.bottom = wintemp. top + 200;

    uint16_t max_depth = (uint16_t)(stateUsed->lifeData.max_depth_meter * 10);

    GFX_graph_print(&t9screen, &wintemp, 0,1,0, max_depth, getMiniLiveLogbookPointerToData(), getMiniLiveLogbookActualDataLength(), CLUT_Font030, NULL);
}

