///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/t6_apnea.c
/// \brief  dive screen for Gauge mode
/// \author Heinrichs Weikamp gmbh
/// \date   1-Feb-2017
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
#include "t6_apnea.h"

#include "data_exchange_main.h"
#include "decom.h"
#include "gfx_fonts.h"
#include "math.h"
#include "tHome.h"
#include "simulation.h"
#include "timer.h"
#include "unit.h"

/* Private variables ---------------------------------------------------------*/
GFX_DrawCfgScreen	t6screen;
GFX_DrawCfgWindow	t6l1;
GFX_DrawCfgWindow	t6r1;
GFX_DrawCfgWindow	t6c1;
GFX_DrawCfgWindow	t6c2;
GFX_DrawCfgWindow	t6c3; // for menu text

extern float depthLastCall[9];
extern uint8_t idDepthLastCall;
extern float temperatureLastCall[3];
extern uint8_t idTemperatureLastCall;

uint8_t t6_selection_customview = 0;

/* Importend function prototypes ---------------------------------------------*/
extern uint8_t write_gas(char *text, uint8_t oxygen, uint8_t helium);

/* Private types -------------------------------------------------------------*/

#define CUSTOMBOX_LINE_LEFT (250)
#define CUSTOMBOX_LINE_RIGHT (549)
#define CUSTOMBOX_INSIDE_OFFSET (2)
#define CUSTOMBOX_OUTSIDE_OFFSET (2)

#define TEXTSIZE 16

const uint8_t t6_customviewsStandard[] =
{
    CVIEW_noneOrDebug,
    CVIEW_T3_Temperature,
    CVIEW_T3_END
};

const uint8_t *t6_customviews = t6_customviewsStandard;
const uint8_t t6_customviewSurfaceMode = CVIEW_T3_ApnoeSurfaceInfo;

/* Private function prototypes -----------------------------------------------*/
void t6_refresh_divemode(void);
void t6_refresh_customview(float depth);

uint8_t t6_test_customview_warnings(void);
void t6_show_customview_warnings(void);
void t6_compass(uint16_t ActualHeading, uint16_t UserSetHeading);

/* Exported functions --------------------------------------------------------*/

// for tHomeDiveMenuControl() in tHome.c and t6_refresh_customview
uint8_t t6_getCustomView(void)
{
    if(stateUsed->lifeData.counterSecondsShallowDepth)
        return t6_customviewSurfaceMode;
    else
        return t6_selection_customview;
}

void t6_init(void)
{
    t6_selection_customview = t6_customviewsStandard[0];

    t6screen.FBStartAdress = 0;
    t6screen.ImageHeight = 480;
    t6screen.ImageWidth = 800;
    t6screen.LayerIndex = 1;

    t6l1.Image = &t6screen;
    t6l1.WindowNumberOfTextLines = 2;
    t6l1.WindowLineSpacing = 19; // Abstand von Y0
    t6l1.WindowTab = 100;
    t6l1.WindowX0 = 0;
    t6l1.WindowX1 = BigFontSeperationLeftRight - 5;
    t6l1.WindowY0 = BigFontSeperationTopBottom + 5;
    t6l1.WindowY1 = 479;

    t6r1.Image = &t6screen;
    t6r1.WindowNumberOfTextLines = t6l1.WindowNumberOfTextLines;
    t6r1.WindowLineSpacing = t6l1.WindowLineSpacing;
    t6r1.WindowTab = t6l1.WindowTab;
    t6r1.WindowX0 = BigFontSeperationLeftRight + 5;
    t6r1.WindowX1 = 799;
    t6r1.WindowY0 = t6l1.WindowY0;
    t6r1.WindowY1 = t6l1.WindowY1;

    t6c1.Image = &t6screen;
    t6c1.WindowNumberOfTextLines = 2;
    t6c1.WindowLineSpacing = t6l1.WindowLineSpacing;
    t6c1.WindowX0 = 0;
    t6c1.WindowX1 = 799;
    t6c1.WindowY0 = 0;
    t6c1.WindowY1 = BigFontSeperationTopBottom - 5;

    t6c2.Image = &t6screen;
    t6c2.WindowNumberOfTextLines = 3;
    t6c2.WindowLineSpacing = 58;
    t6c2.WindowX0 = 370;
    t6c2.WindowX1 = 799;
    t6c2.WindowY0 = 0;
    t6c2.WindowY1 = BigFontSeperationTopBottom - 5;
    t6c2.WindowTab = 600;

    t6c3.Image = &t6screen;
    t6c3.WindowNumberOfTextLines = 1;
    t6c3.WindowLineSpacing = 0; // Abstand von Y0
    t6c3.WindowTab = 100;
    t6c3.WindowX0 = CUSTOMBOX_LINE_LEFT + CUSTOMBOX_INSIDE_OFFSET;
    t6c3.WindowX1 = CUSTOMBOX_LINE_RIGHT - CUSTOMBOX_INSIDE_OFFSET;
    t6c3.WindowY0 = 0;
    t6c3.WindowY1 = 69;
}


void t6_refresh(void)
{
    static uint8_t last_mode = MODE_SURFACE;

    SStateList status;
    get_globalStateList(&status);

    if(stateUsed->mode != MODE_DIVE)
    {
        last_mode = MODE_SURFACE;
        settingsGetPointer()->design = 7;
        if(t6screen.FBStartAdress)
        {
            releaseFrame(24,t6screen.FBStartAdress);
            t6screen.FBStartAdress = 0;
        }
        return;
    }

    if(status.base != BaseHome)
        return;

    t6screen.FBStartAdress = getFrame(24);

    if(last_mode != MODE_DIVE)
    {
        last_mode = MODE_DIVE;
        t6_selection_customview = *t6_customviews;
    }

    if(status.page == PageSurface)
        set_globalState(StD);

    t6_refresh_divemode();
    GFX_SetFramesTopBottom(t6screen.FBStartAdress, NULL,480);
    releaseAllFramesExcept(24,t6screen.FBStartAdress);
}

/* Private functions ---------------------------------------------------------*/

void t6_refresh_divemode(void)
{
    char text[512];
    uint8_t  customview_warnings = 0;
    float depth_meter = 0.0;

    // everything like lines, depth, ascent graph and divetime or counterSecondsShallowDepth
    depth_meter = t3_basics_lines_depth_and_divetime(&t6screen, &t6l1, &t6r1, DIVEMODE_Apnea);


    // customview
    if(stateUsed->warnings.numWarnings)
        customview_warnings = t6_test_customview_warnings();

    if(customview_warnings && warning_count_high_time)
        t3_basics_show_customview_warnings(&t6c1);
    else
        t6_refresh_customview(depth_meter);

    if(stateUsed->warnings.lowBattery)
        t3_basics_battery_low_customview_extra(&t6c1);


    /* Menu Selection (and gas mix) */
    if(get_globalState() == StDBEAR)
    {
        snprintf(text,TEXTSIZE,"\a\001%c%c", TXT_2BYTE, TXT2BYTE_DiveBearingQ);
        GFX_write_string_color(&FontT48,&t6c3,text,0,CLUT_WarningYellow);
    }
    else if(get_globalState() == StDRAVG)
    {
        snprintf(text,TEXTSIZE,"\a\001%c%c", TXT_2BYTE, TXT2BYTE_DiveResetAvgQ);
        GFX_write_string_color(&FontT48,&t6c3,text,0,CLUT_WarningYellow);
    }
    else if(get_globalState() == StDQUIT)
    {
        snprintf(text,TEXTSIZE,"\a\001%c%c", TXT_2BYTE, TXT2BYTE_DiveQuitQ);
        GFX_write_string_color(&FontT48,&t6c3,text,0,CLUT_WarningYellow);
    }
    else if(get_globalState() == StDSIM1)
    {
        snprintf(text,TEXTSIZE,"\a\001%c%c", TXT_2BYTE, TXT2BYTE_DiveQuitQ);
        GFX_write_string_color(&FontT48,&t6c3,text,0,CLUT_WarningYellow);
    }
    else if(get_globalState() == StDSIM2)
    {
        if(settingsGetPointer()->nonMetricalSystem)
            snprintf(text,TEXTSIZE,"\a\001" " Sim:-3.33ft ");
        else
            snprintf(text,TEXTSIZE,"\a\001" " Sim:-1m ");
        GFX_write_string_color(&FontT48,&t6c3,text,0,CLUT_WarningYellow);
        snprintf(text,TEXTSIZE,"\a\f %u %c%c"
            , unit_depth_integer(simulation_get_aim_depth())
            , unit_depth_char1()
            , unit_depth_char2()
            );
        GFX_write_string_color(&FontT42,&t6l1,text,0,CLUT_WarningYellow);

    }
    else if(get_globalState() == StDSIM3)
    {
        if(settingsGetPointer()->nonMetricalSystem)
            snprintf(text,TEXTSIZE,"\a\001" " Sim:+3.33ft ");
        else
            snprintf(text,TEXTSIZE,"\a\001" " Sim:+1m ");
        GFX_write_string_color(&FontT48,&t6c3,text,0,CLUT_WarningYellow);
        snprintf(text,TEXTSIZE,"\a\f %u %c%c"
            , unit_depth_integer(simulation_get_aim_depth())
            , unit_depth_char1()
            , unit_depth_char2()
            );
        GFX_write_string_color(&FontT42,&t6l1,text,0,CLUT_WarningYellow);
    }
    else if(get_globalState() == StDSIM4)
    {
        snprintf(text,TEXTSIZE,"\a\001" " Sim:+5' ");
        GFX_write_string_color(&FontT48,&t6c3,text,0,CLUT_WarningYellow);
        snprintf(text,TEXTSIZE,"\a\f %u %c%c"
            , unit_depth_integer(simulation_get_aim_depth())
            , unit_depth_char1()
            , unit_depth_char2()
            );
        GFX_write_string_color(&FontT42,&t6l1,text,0,CLUT_WarningYellow);
    }
    else
    {
        // keep empty
    }
}


void t6_battery_low_customview_extra(void)
{
    char TextC1[256];

    TextC1[0] = '\002';
    TextC1[1] = '\f';
    TextC1[2] = '\025';
    TextC1[3] = '3';
    TextC1[4] = '1';
    TextC1[5] = '1';
    TextC1[6] = '1';
    TextC1[7] = '1';
    TextC1[8] = '1';
    TextC1[9] = '1';
    TextC1[10] = '1';
    TextC1[11] = '1';
    TextC1[12] = '1';
    TextC1[13] = '1';
    TextC1[14] = '0';
    TextC1[15] = 0;

    if(!warning_count_high_time)
        TextC1[4] = '2';

    GFX_write_string(&Batt24,&t6c1,TextC1,0);
}


void t6_battery_scooter_customview_extra(void)
{
    char TextC1[256];

    TextC1[0] = '\001';
    TextC1[1] = '\f';
    TextC1[2] = '\032';
    TextC1[3] = '3';
    TextC1[4] = '1';
    TextC1[5] = '1';
    TextC1[6] = '1';
    TextC1[7] = '1';
    TextC1[8] = '1';
    TextC1[9] = '1';
    TextC1[10] = '1';
    TextC1[11] = '1';
    TextC1[12] = '1';
    TextC1[13] = '1';
    TextC1[14] = '0';
    TextC1[15] = 0;

    for(int i=1;i<=10;i++)
    {
        if(	stateUsed_scooterRemainingBattCapacity()  > (9 * i))
            TextC1[i+3] += 1;
    }

    if(stateUsed_scooterRemainingBattCapacity() < 10)
        TextC1[2] = '\025';

    if(!warning_count_high_time)
        TextC1[4] = '2';

    if(stateUsed->lifeData.scooterAgeInMilliSeconds > 1500)
        TextC1[2] = '\031';

    GFX_write_string(&Batt24,&t6c1,TextC1,0);
}


void t6_change_customview(void)
{
    t3_basics_change_customview(&t6_selection_customview, t6_customviews);
}


void t6_refresh_customview(float depth)
{
    uint8_t customViewLeftSide = CVIEW_T3_MaxDepth;

    if((t6_selection_customview == CVIEW_sensors) &&(stateUsed->diveSettings.ccrOption == 0))
        t6_change_customview();

    if(t6_getCustomView() == CVIEW_T3_ApnoeSurfaceInfo)
        customViewLeftSide = CVIEW_T3_ApnoeSurfaceInfo;

    t3_basics_refresh_customview(depth, customViewLeftSide,  &t6screen, &t6c1, &t6c2, DIVEMODE_Apnea);
    t3_basics_refresh_apnoeRight(depth, t6_getCustomView(), &t6screen, &t6c1, &t6c2, DIVEMODE_Apnea);
}


uint8_t t6_test_customview_warnings(void)
{
    uint8_t count = 0;

    count = 0;
    return count;
}

/*

    char text[512];
    uint16_t textpointer = 0;


    // CVIEW_T3_Temperature
    float temperatureThisCall;
    float temperature;


    // CVIEW_T3_StopWatch
    SDivetime Stopwatch = {0,0,0,0};
    float fAverageDepth, fAverageDepthAbsolute;
    uint16_t tempWinX0;
    uint16_t tempWinY0;

    switch(t6_selection_customview)
    {
        case CVIEW_T3_ApnoeTimes:
        break;

        case CVIEW_T3_StopWatch:
            Stopwatch.Total = timer_Stopwatch_GetTime();
            Stopwatch.Minutes = Stopwatch.Total / 60;
            Stopwatch.Seconds = Stopwatch.Total - ( Stopwatch.Minutes * 60 );
            fAverageDepth = timer_Stopwatch_GetAvarageDepth_Meter();
            fAverageDepthAbsolute = stateUsed->lifeData.average_depth_meter;

            snprintf(text,TEXTSIZE,"\032\f%c",TXT_AvgDepth);
            GFX_write_string(&FontT42,&t6c1,text,0);
            snprintf(text,TEXTSIZE,"\030\003\016%01.1f",fAverageDepthAbsolute);
            GFX_write_string(&FontT105,&t6c1,text,0);

            tempWinX0 = t6c1.WindowX0;
            tempWinY0 = t6c1.WindowY0;
            t6c1.WindowX0 = 480;
//			snprintf(text,TEXTSIZE,"\032\f%c%c - %c",TXT_2BYTE, TXT2BYTE_Clock, TXT_AvgDepth);
            snprintf(text,TEXTSIZE,"\032\f%c", TXT_Stopwatch);
            GFX_write_string(&FontT42,&t6c1,text,0);
            snprintf(text,TEXTSIZE,"\030\016%01.1f",fAverageDepth);
            GFX_write_string(&FontT105,&t6c1,text,0);
            t6c1.WindowY0 = 100;
            snprintf(text,TEXTSIZE,"\030%u:\016\016%02u",Stopwatch.Minutes, Stopwatch.Seconds);
            GFX_write_string(&FontT105,&t6c1,text,0);
            t6c1.WindowX0 = tempWinX0;
            t6c1.WindowY0 = tempWinY0;
        break;

        case CVIEW_T3_Temperature:
            snprintf(text,TEXTSIZE,"\032\f%c",TXT_Temperature);
            GFX_write_string(&FontT42,&t6c1,text,0);
            // mean value
            temperatureThisCall = unit_temperature_float(stateUsed->lifeData.temperature_celsius);
            temperature = (temperatureThisCall + temperatureLastCall[0] + temperatureLastCall[1] + temperatureLastCall[2]) / 4.0f;
            idTemperatureLastCall++;
            if(idTemperatureLastCall >= 3)
                idTemperatureLastCall = 0;
            temperatureLastCall[idTemperatureLastCall] = temperatureThisCall;
            textpointer = snprintf(text,TEXTSIZE,"\030\003\016%01.1f \140",temperature); // "\016\016%01.1f `" + C or F
            if(settingsGetPointer()->nonMetricalSystem == 0)
                text[textpointer++] = 'C';
            else
                text[textpointer++] = 'F';
            text[textpointer++] = 0;
            GFX_write_string(&FontT105,&t6c1,text,0);
        break;

        case CVIEW_Compass:
            snprintf(text,TEXTSIZE,"\032\f%c%c",TXT_2BYTE, TXT2BYTE_Compass);
            GFX_write_string(&FontT42,&t6c1,text,0);
            snprintf(text,100,"\030\003%03i`",(uint16_t)stateUsed->lifeData.compass_heading);
            GFX_write_string(&FontT105,&t6c1,text,0);
            t6_compass((uint16_t)stateUsed->lifeData.compass_heading, stateUsed->diveSettings.compassHeading);
        break;

        case CVIEW_T3_MaxDepth:
        default:
            snprintf(text,TEXTSIZE,"\032\f%c",TXT_MaxDepth);
            GFX_write_string(&FontT42,&t6c1,text,0);
            snprintf(text,TEXTSIZE,"\020\003\016%01.1f",unit_depth_float(stateUsed->lifeData.max_depth_meter));
            t3_basics_colorscheme_mod(text);
            GFX_write_string(&FontT105,&t6c1,text,1);
            break;
    }
}


void t6_show_customview_warnings(void)
{
    char text[256], textMain[256];
    uint8_t textpointer, textpointerMain, lineFree, more;

    snprintf(text,TEXTSIZE,"\025\f%c",TXT_Warning);
    GFX_write_string(&FontT42,&t6c1,text,0);

    lineFree = 1;
    more = 0;

    textpointerMain = 0;
    textMain[textpointerMain++] = '\025';
    textMain[textpointerMain++] = '\003';

    textpointer = 0;

    text[textpointer++] = '\021';
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_WarnDecoMissed;
    if(stateUsed->warnings.decoMissed)
    {
        text[textpointer - 3] =  '\025';
        if(lineFree)
        {
            textMain[textpointerMain++] = TXT_2BYTE;
            textMain[textpointerMain++] = text[textpointer - 1];
            textMain[textpointerMain] = 0;
            lineFree--;
        }
        else
        {
            more++;
        }
    }

    text[textpointer++] = '\t';
    text[textpointer++] = '\021';
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_WarnPPO2Low;
    if(stateUsed->warnings.ppO2Low)
    {
        text[textpointer - 3] =  '\025';
        if(lineFree)
        {
            textMain[textpointerMain++] = TXT_2BYTE;
            textMain[textpointerMain++] = text[textpointer - 1];
            textMain[textpointerMain] = 0;
            lineFree--;
        }
        else
        {
            more++;
        }
    }

    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = '\021';
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_WarnPPO2High;
    if(stateUsed->warnings.ppO2High)
    {
        text[textpointer - 3] =  '\025';
        if(lineFree)
        {
            textMain[textpointerMain++] = TXT_2BYTE;
            textMain[textpointerMain++] = text[textpointer - 1];
            textMain[textpointerMain] = 0;
            lineFree--;
        }
        else
        {
            more++;
        }
    }

    text[textpointer++] = '\t';
    text[textpointer++] = '\021';
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_WarnFallback;
    if(stateUsed->warnings.fallback)
    {
        text[textpointer - 3] =  '\025';
        if(lineFree)
        {
            textMain[textpointerMain++] = TXT_2BYTE;
            textMain[textpointerMain++] = text[textpointer - 1];
            textMain[textpointerMain] = 0;
            lineFree--;
        }
        else
        {
            more++;
        }
    }

    text[textpointer++] = '\n';
    text[textpointer++] = '\r';
    text[textpointer++] = '\021';
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_WarnSensorLinkLost;
    if(stateUsed->warnings.sensorLinkLost)
    {
        text[textpointer - 3] =  '\025';
        if(lineFree)
        {
            textMain[textpointerMain++] = TXT_2BYTE;
            textMain[textpointerMain++] = text[textpointer - 1];
            textMain[textpointerMain] = 0;
            lineFree--;
        }
        else
        {
            more++;
        }
    }

    text[textpointer] = 0;
    GFX_write_string(&FontT48,&t6c1,textMain,1);
    if(more)
    {
        GFX_write_string(&FontT48,&t6c2,text,1);
    }
}


void t6_change_customview(void)
{
    const uint8_t *pViews;
    pViews = t6_customviews;

    while((*pViews != CVIEW_T3_END) && (*pViews != t6_selection_customview))
        {pViews++;}

    if(*pViews < CVIEW_T3_END)
        pViews++;

    if(*pViews == CVIEW_T3_END)
    {
        t6_selection_customview = t6_customviews[0];
    }
    else
        t6_selection_customview = *pViews;
}


void t3_basics_colorscheme_mod(char *text)
{
    if((text[0] == '\020') && !GFX_is_colorschemeDiveStandard())
    {
        text[0] = '\027';
    }
}

point_t t6_compass_circle(uint8_t id, uint16_t degree)
{
    float fCos, fSin;
    const float piMult =  ((2 * 3.14159) / 360);
//	const int radius[4] = {95,105,115,60};
    const int radius[4] = {85,95,105,90};
    const point_t offset = {.x = 600, .y = 116};

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


void t6_compass(uint16_t ActualHeading, uint16_t UserSetHeading)
{
    uint16_t LineHeading;
    point_t center;
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

    while(ActualHeading > 359) ActualHeading -= 360;

    LineHeading = 360 - ActualHeading;
    GFX_draw_thick_line(9,&t6screen, t6_compass_circle(0,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font030); // North
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(9,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031); // Maintick
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(9,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(9,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031);

    LineHeading = 360 - ActualHeading;
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(5,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031); // Subtick
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(5,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(5,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 90;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(5,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031);

    LineHeading = 360 - ActualHeading;
    LineHeading += 22;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031); // Subtick
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031); // Subtick
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031);
    LineHeading += 45;
    if(LineHeading > 359) LineHeading -= 360;
    GFX_draw_thick_line(3,&t6screen, t6_compass_circle(1,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_Font031);

    if(UserSetHeading)
    {
        LineHeading = UserSetHeading + 360 - ActualHeading;
        if(LineHeading > 359) LineHeading -= 360;
        GFX_draw_thick_line(9,&t6screen, t6_compass_circle(3,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_CompassUserHeadingTick);

        // Rï¿½ckpeilung, User Back Heading
        LineHeading = UserSetHeading + 360 + 180 - ActualHeading;
        if(LineHeading > 359) LineHeading -= 360;
        if(LineHeading > 359) LineHeading -= 360;
        GFX_draw_thick_line(9,&t6screen, t6_compass_circle(3,LineHeading),  t6_compass_circle(2,LineHeading), CLUT_CompassUserBackHeadingTick);
    }

    center.x = 600;
    center.y = 116;
    GFX_draw_circle(&t6screen, center, 106, CLUT_Font030);
    GFX_draw_circle(&t6screen, center, 107, CLUT_Font030);
    GFX_draw_circle(&t6screen, center, 108, CLUT_Font030);
}
*/

