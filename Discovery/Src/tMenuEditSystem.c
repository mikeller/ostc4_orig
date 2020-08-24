///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tMenuEditSystem.c
/// \brief  Main Template file for Menu Edit System settings
/// \author heinrichs weikamp gmbh
/// \date   05-Aug-2014
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
#include "tMenuEditSystem.h"

#include "data_exchange_main.h"
#include "externLogbookFlash.h"
#include "gfx_fonts.h"
#include "ostc.h"
#include "settings.h" // for getLicence()
#include "tHome.h"  // for enum CUSTOMVIEWS and init_t7_compass()
#include "tMenu.h"
#include "tMenuEdit.h"
#include "tMenuSystem.h"
#include "motion.h"
#include "t7.h"


#define CV_SUBPAGE_MAX		(2u)	/* max number of customer view selection pages */
/*#define HAVE_DEBUG_VIEW */
static uint8_t infoPage = 0;

/* Private function prototypes -----------------------------------------------*/
void openEdit_DateTime(void);
void openEdit_Language(void);
void openEdit_Design(void);
void openEdit_Customview(void);
void openEdit_Information(void);
void openEdit_Reset(void);
void openEdit_CustomviewDivemode(uint8_t line);
//void openEdit_ShowDebugInfo(void);
//void openEdit_Salinity(void);

/* Announced function prototypes -----------------------------------------------*/
uint8_t OnAction_Date					(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Time					(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_DDMMYY				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_MMDDYY				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_YYMMDD				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_DST					(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_English			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_German				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_French				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Italian			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Espanol			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
//uint8_t OnAction_Design_t7		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
//uint8_t OnAction_Design_t7ft	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
//uint8_t OnAction_Design_t3		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);

uint8_t OnAction_Units				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Colorscheme	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_DebugInfo		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);

uint8_t OnAction_CViewTimeout	 (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_CViewStandard (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_CornerTimeout (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_CornerStandard(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_ExtraDisplay	 (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_MotionCtrl	 (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);

uint8_t OnAction_Exit					(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Confirm			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_RebootRTE				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_ResetDeco		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_ResetAll			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_ResetLogbook	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_RebootMainCPU		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Nothing			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_LogbookOffset(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_SetFactoryDefaults(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_SetBatteryCharge(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
#ifdef ENABLE_ANALYSE_SAMPLES
uint8_t OnAction_RecoverSampleIdx(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
#endif
#ifdef SCREENTEST
uint8_t OnAction_ScreenTest		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
#endif
uint8_t OnAction_Information	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
/*
uint8_t OnAction_Salinity			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_TestCLog			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
*/

/* Exported functions --------------------------------------------------------*/

void openEdit_System(uint8_t line)
{
    set_globalState_Menu_Line(line);
    resetMenuEdit(CLUT_MenuPageSystem);

    if(actual_menu_content == MENU_SURFACE)
    {
        switch(line)
        {
        case 1:
        default:
            openEdit_DateTime();
        break;
        case 2:
            openEdit_Language();
        break;
        case 3:
            openEdit_Design();
        break;
        case 4:
            openEdit_Customview();
        break;
        case 5:
            openEdit_Information();
        break;
        case 6:
            openEdit_Reset();
        break;
/*
        case 3:
            openEdit_DecoFutureTTS();
        break;
        case 4:
            openEdit_DecoLastStop();
        break;
*/
        }
    }
    else
    {
        openEdit_CustomviewDivemode(line);
    }

}

/* Private functions ---------------------------------------------------------*/

void openEdit_CustomviewDivemode(uint8_t line)
{
	static uint8_t customviewsSubpage = 0;
	SSettings *pSettings = settingsGetPointer();
	extern _Bool WriteSettings;
	char text[MAX_PAGE_TEXTSIZE];
	uint16_t tabPosition;
	uint32_t id;


	if((line == 6) || (cv_changelist[customviewsSubpage * 5 + line-1] == CVIEW_END))		/* select next set of views */
	{
		customviewsSubpage++;
		if(customviewsSubpage == CV_SUBPAGE_MAX)
		{
			customviewsSubpage = 0;
		}
		set_CustomsviewsSubpage(customviewsSubpage);
		/* rebuild the selection page with the next set of customer views */
		id = tMSystem_refresh(0, text, &tabPosition, NULL);
		tM_build_page(id, text, tabPosition, NULL);
		openMenu(0);
	}
	else
	{
		pSettings->cv_configuration ^= 1 << (cv_changelist[customviewsSubpage * 5 + line-1]);
		if(t7_GetEnabled_customviews() == 0)
		{
			pSettings->cv_configuration ^= (1 << CVIEW_noneOrDebug);
		}
		WriteSettings = 1;
		InitMotionDetection(); /* consider new view setup for view selection by motion */
		exitMenuEdit_to_Menu_with_Menu_Update();
	}
}


void openEdit_DateTime(void)
{
    RTC_DateTypeDef Sdate;
    RTC_TimeTypeDef Stime;
    uint8_t day,month,year,hour,minute, dateFormat, ddmmyy, mmddyy, yymmdd;
    char text[32];
    SSettings *pSettings;
    const SDiveState * pStateReal = stateRealGetPointer();

    pSettings = settingsGetPointer();
    translateDate(pStateReal->lifeData.dateBinaryFormat, &Sdate);
    translateTime(pStateReal->lifeData.timeBinaryFormat, &Stime);
    year = Sdate.Year;
    month = Sdate.Month;
    day = Sdate.Date;
    hour = Stime.Hours;
    minute= Stime.Minutes;

    if(year < 16)
        year = 16;

    if(month < 1)
        month = 1;

    if(day < 1)
        day = 1;

//	daylightsaving = Stime.DayLightSaving;
    dateFormat = pSettings->date_format;
    ddmmyy = 0;
    mmddyy = 0;
    yymmdd = 0;

    if(dateFormat == DDMMYY)
        ddmmyy = 1;
    else
    if(dateFormat == MMDDYY)
        mmddyy = 1;
    else
        yymmdd = 1;

    text[0] = '\001';
    text[1] = TXT_DateAndTime;
    text[2] = 0;

    write_topline(text);

    write_label_fix(  20, 340, ME_Y_LINE1, &FontT42, TXT_TimeConfig);
    write_label_fix(  20, 340, ME_Y_LINE2, &FontT42, TXT_DateConfig);
    write_label_var( 600, 800, ME_Y_LINE2, &FontT48, "\016\016DDMMYY\017");
    write_label_fix(  20, 790, ME_Y_LINE3, &FontT42, TXT_Format);
//	write_label_fix( 350 ,580, 250, &FontT42, TXT_Daylightsaving);

    write_field_2digit(StMSYS1_Time,		320, 780, ME_Y_LINE1,  &FontT48, "##:##", (uint32_t)hour, (uint32_t)minute, 0, 0);
    write_field_2digit(StMSYS1_Date,		320, 780, ME_Y_LINE2,  &FontT48, "##-##-20##", (uint32_t)day, (uint32_t)month, (uint32_t)year, 0);
    write_field_on_off(StMSYS1_DDMMYY,	320, 790, ME_Y_LINE3,  &FontT48, "DDMMYY", ddmmyy);
    write_field_on_off(StMSYS1_MMDDYY,	320, 790, ME_Y_LINE4,  &FontT48, "MMDDYY", mmddyy);
    write_field_on_off(StMSYS1_YYMMDD,	320, 790, ME_Y_LINE5,  &FontT48, "YYMMDD", yymmdd);
//	write_field_on_off(StMSYS1_DST,			350, 580, 310,  &FontT48, "Active", daylightsaving);

    setEvent(StMSYS1_Date, 		(uint32_t)OnAction_Date);
    setEvent(StMSYS1_Time, 		(uint32_t)OnAction_Time);
    setEvent(StMSYS1_DDMMYY,	(uint32_t)OnAction_DDMMYY);
    setEvent(StMSYS1_MMDDYY,	(uint32_t)OnAction_MMDDYY);
    setEvent(StMSYS1_YYMMDD,	(uint32_t)OnAction_YYMMDD);
//	setEvent(StMSYS1_DST,			(uint32_t)OnAction_DST);

    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);
}


uint8_t OnAction_Date(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t digitContentNew;
    uint32_t newDay, newMonth, newYear;
    RTC_DateTypeDef sdatestructure;


    if(action == ACTION_BUTTON_ENTER)
    {
        return digitContent;
    }
    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &newDay, &newMonth, &newYear, 0);
        if(newDay == 0)
            newDay = 1;
        if(newDay > 31)
            newDay = 31;
        if(newMonth == 0)
            newMonth = 1;
        if(newMonth > 12)
            newMonth = 12;
        if((newMonth == 2) && (newDay > 29))
            newDay = 29;
        if((newDay > 30) && ((newMonth == 4) ||(newMonth == 6) ||(newMonth == 9) ||(newMonth == 11)))
            newDay = 30;
        if(newYear < 17)
            newYear = 17;
        if(newYear > 99)
            newYear = 99;

        sdatestructure.Date = newDay;
        sdatestructure.Month = newMonth;
        sdatestructure.Year = newYear;
        setWeekday(&sdatestructure);

        setDate(sdatestructure);

        tMenuEdit_newInput(editId, newDay, newMonth, newYear, 0);
        return UNSPECIFIC_RETURN;
    }
    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if((blockNumber == 0) && (digitContentNew > '0' + 31))
            digitContentNew = '1';
        if((blockNumber == 1) && (digitContentNew > '0' + 12))
            digitContentNew = '1';
        // year range 2017-2018
        if((blockNumber == 2) && (digitContentNew > '0' + 22))
            digitContentNew = '0' + 18;
        return digitContentNew;
    }
    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - 1;
        if((blockNumber == 0) && (digitContentNew < '1'))
            digitContentNew = '0' + 31;
        if((blockNumber == 1) && (digitContentNew < '1'))
            digitContentNew = '0' + 12;
        // year range 2016-2018
        if((blockNumber == 2) && (digitContentNew < '0' + 17))
            digitContentNew = '0' + 18;
        return digitContentNew;
    }
/*
    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if((blockNumber == 2) && (digitNumber == 0) && (digitContentNew > '2'))
            digitContentNew = '1';
        if((blockNumber == 0) && (digitNumber == 0) && (digitContentNew > '3'))
            digitContentNew = '0';
        if((blockNumber == 1) && (digitNumber == 0) && (digitContentNew > '1'))
            digitContentNew = '0';
        if(digitContentNew > '9')
            digitContentNew = '0';
        return digitContentNew;
    }
    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - 1;
        if((blockNumber == 2) && (digitNumber == 0) && (digitContentNew < '1'))
            digitContentNew = '2';
        if((blockNumber == 0) && (digitNumber == 0) && (digitContentNew < '0'))
            digitContentNew = '3';
        if((blockNumber == 1) && (digitNumber == 0) && (digitContentNew < '0'))
            digitContentNew = '1';
        if(digitContentNew < '0')
            digitContentNew = '9';
        return digitContentNew;
    }
*/
    return UNSPECIFIC_RETURN;
}


uint8_t OnAction_Time(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t digitContentNew;
    uint32_t newHour, newMinute;
    RTC_TimeTypeDef stimestructure;

    if(action == ACTION_BUTTON_ENTER)
    {
        return digitContent;
    }
    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &newHour, &newMinute, 0, 0);
        if(newHour > 23)
            newHour = 23;
        if(newMinute > 59)
            newMinute = 59;

        stimestructure.Hours = newHour;
        stimestructure.Minutes = newMinute;
        stimestructure.Seconds = 0;

        setTime(stimestructure);

        tMenuEdit_newInput(editId, newHour, newMinute, 0, 0);
        return UNSPECIFIC_RETURN;
    }
    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if((blockNumber == 0) && (digitContentNew > '0' + 23))
            digitContentNew = '0';
        if((blockNumber == 1) && (digitContentNew > '0' + 59))
            digitContentNew = '0';
        return digitContentNew;
    }
    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - 1;
        if((blockNumber == 0) && (digitContentNew < '0'))
            digitContentNew = '0' + 23;
        if((blockNumber == 1) && (digitContentNew < '0'))
            digitContentNew = '0' + 59;
        return digitContentNew;
    }
/*
    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if((blockNumber == 0) && (digitNumber == 0) && (digitContentNew > '2'))
            digitContentNew = '0';
        if((blockNumber == 1) && (digitNumber == 0) && (digitContentNew > '5'))
            digitContentNew = '0';
        if(digitContentNew > '9')
            digitContentNew = '0';
        return digitContentNew;
    }
    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - 1;
        if((blockNumber == 0) && (digitNumber == 0) && (digitContentNew < '0'))
            digitContentNew = '2';
        if((blockNumber == 1) && (digitNumber == 0) && (digitContentNew < '0'))
            digitContentNew = '5';
        if(digitContentNew < '0')
            digitContentNew = '9';
        return digitContentNew;
    }
*/
    return UNSPECIFIC_RETURN;
}


uint8_t OnAction_DDMMYY(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings;

    pSettings = settingsGetPointer();
    pSettings->date_format = DDMMYY;

    tMenuEdit_set_on_off(editId, 1);
    tMenuEdit_set_on_off(StMSYS1_MMDDYY, 0);
    tMenuEdit_set_on_off(StMSYS1_YYMMDD, 0);

    return UNSPECIFIC_RETURN;
}


uint8_t OnAction_MMDDYY(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings;

    pSettings = settingsGetPointer();
    pSettings->date_format = MMDDYY;

    tMenuEdit_set_on_off(editId, 1);
    tMenuEdit_set_on_off(StMSYS1_DDMMYY, 0);
    tMenuEdit_set_on_off(StMSYS1_YYMMDD, 0);

    return UNSPECIFIC_RETURN;
}


uint8_t OnAction_YYMMDD(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings;

    pSettings = settingsGetPointer();
    pSettings->date_format = YYMMDD;

    tMenuEdit_set_on_off(editId, 1);
    tMenuEdit_set_on_off(StMSYS1_MMDDYY, 0);
    tMenuEdit_set_on_off(StMSYS1_DDMMYY, 0);

    return UNSPECIFIC_RETURN;
}


uint8_t OnAction_DST(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    RTC_TimeTypeDef stimestructure;
    uint8_t newDST;

    get_RTC_DateTime(0, &stimestructure);
    newDST = stimestructure.DayLightSaving;
    if(newDST)
        newDST = 0;
    else
        newDST = 1;
    stimestructure.DayLightSaving = newDST;
    set_RTC_DateTime(0, &stimestructure);

    tMenuEdit_set_on_off(editId, newDST);

    return UNSPECIFIC_RETURN;
}


void openEdit_Language(void)
{
    char text[32];
    uint8_t actualLanguage, active;
    SSettings *pSettings;

    pSettings = settingsGetPointer();
    actualLanguage = pSettings->selected_language;

    text[0] = '\001';
    text[1] = TXT_Language;
    text[2] = 0;
    write_topline(text);

    text[0] = TXT_LanguageName;
    text[1] = 0;

    pSettings->selected_language = LANGUAGE_English;
    if(	actualLanguage == pSettings->selected_language)
        active = 1;
    else
        active = 0;
    write_field_on_off(StMSYS2_English,			 30, 500, ME_Y_LINE1,  &FontT48, text, active);

    pSettings->selected_language = LANGUAGE_German;
    if(	actualLanguage == pSettings->selected_language)
        active = 1;
    else
        active = 0;
    write_field_on_off(StMSYS2_German,			 30, 800, ME_Y_LINE2,  &FontT48, text, active);

    pSettings->selected_language = LANGUAGE_French;
    if(	actualLanguage == pSettings->selected_language)
        active = 1;
    else
        active = 0;
    write_field_on_off(StMSYS2_French,			 30, 800, ME_Y_LINE3,  &FontT48, text, active);


    pSettings->selected_language = LANGUAGE_Italian;
    if(	actualLanguage == pSettings->selected_language)
        active = 1;
    else
        active = 0;
    write_field_on_off(StMSYS2_Italian,			 30, 800, ME_Y_LINE4,  &FontT48, text, active);


    pSettings->selected_language = LANGUAGE_Espanol;
    if(	actualLanguage == pSettings->selected_language)
        active = 1;
    else
        active = 0;
    write_field_on_off(StMSYS2_Espanol,			 30, 800, ME_Y_LINE5,  &FontT48, text, active);

    pSettings->selected_language = actualLanguage;

    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);

    setEvent(StMSYS2_English, (uint32_t)OnAction_English);
    setEvent(StMSYS2_German, 	(uint32_t)OnAction_German);
    setEvent(StMSYS2_French,	(uint32_t)OnAction_French);
    setEvent(StMSYS2_Italian,	(uint32_t)OnAction_Italian);
    setEvent(StMSYS2_Espanol,	(uint32_t)OnAction_Espanol);
}


uint8_t OnAction_English			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings = settingsGetPointer();
    pSettings->selected_language = LANGUAGE_English;
    return EXIT_TO_MENU_WITH_LOGO;
}


uint8_t OnAction_German				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings = settingsGetPointer();
    pSettings->selected_language = LANGUAGE_German;
    return EXIT_TO_MENU_WITH_LOGO;
}


uint8_t OnAction_French				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings = settingsGetPointer();
    pSettings->selected_language = LANGUAGE_French;
    return EXIT_TO_MENU_WITH_LOGO;
}


uint8_t OnAction_Italian			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings = settingsGetPointer();
    pSettings->selected_language = LANGUAGE_Italian;
    return EXIT_TO_MENU_WITH_LOGO;
}


uint8_t OnAction_Espanol			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings = settingsGetPointer();
    pSettings->selected_language = LANGUAGE_Espanol;
    return EXIT_TO_MENU_WITH_LOGO;
}


void openEdit_Design(void)
{
    refresh_Design();

    write_field_button(StMSYS3_Units,			400, 700, ME_Y_LINE1,  &FontT48, "");
    write_field_button(StMSYS3_Colors,		400, 700, ME_Y_LINE2,  &FontT48, "");
#ifdef HAVE_DEBUG_VIEW
	write_field_button(StMSYS3_Debug,			400, 700, ME_Y_LINE3,  &FontT48, "");
#endif
    setEvent(StMSYS3_Units,		(uint32_t)OnAction_Units);
    setEvent(StMSYS3_Colors,	(uint32_t)OnAction_Colorscheme);
#ifdef HAVE_DEBUG_VIEW
	setEvent(StMSYS3_Debug,		(uint32_t)OnAction_DebugInfo);
#endif
}


void refresh_Design(void)
{
    char text[32];

    // header
    text[0] = '\001';
    text[1] = TXT_2BYTE;
    text[2] = TXT2BYTE_Layout;
    text[3] = 0;
    write_topline(text);

    // units
    text[0] = TXT_2BYTE;
    text[1] = TXT2BYTE_Units;
    text[2] = 0;
    write_label_var(  30, 200, ME_Y_LINE1, &FontT48, text);

    if(settingsGetPointer()->nonMetricalSystem)
    {
        text[1] = TXT2BYTE_Units_feet;
    }
    else
    {
        text[1] = TXT2BYTE_Units_metric;
    }
    write_label_var( 400, 700, ME_Y_LINE1, &FontT48, text);

    // colorscheme
    text[0] = TXT_2BYTE;
    text[1] = TXT2BYTE_Farbschema;
    text[2] = 0;
    write_label_var(  30, 300, ME_Y_LINE2, &FontT48, text);

    text[0] = '0' + settingsGetPointer()->tX_colorscheme;
    text[1] = 0;
    write_label_var( 400, 700, ME_Y_LINE2, &FontT48, text);

#ifdef HAVE_DEBUG_VIEW
    // specials
    text[0] = TXT_2BYTE;
    text[1] = TXT2BYTE_ShowDebug;
    text[2] = 0;
    write_label_var(  30, 700, ME_Y_LINE3, &FontT48, text);

    if(settingsGetPointer()->showDebugInfo)
        text[0] = '\005';
    else
        text[0] = '\006';
    text[1] = 0;
    write_label_var( 400, 700, ME_Y_LINE3, &FontT48, text);
#endif

    // design
    text[0] = TXT_Depth;
    text[1] = 0;
    write_content( 30, 700, ME_Y_LINE4, &FontT24, text, CLUT_Font031);
    write_content( 30, 700, ME_Y_LINE4 + 30 + 70, &FontT48, "___________", CLUT_DIVE_FieldSeperatorLines);
    write_content(280, 700, ME_Y_LINE4 + 30 + 70 - 3, &FontT48, "|", CLUT_DIVE_pluginbox);
    write_content(290, 700, ME_Y_LINE4 + 30 + 70 - 37, &FontT48, "_______________", CLUT_DIVE_pluginbox);
    write_content( 30, 700, ME_Y_LINE4 + 30, &FontT144, "24.7", CLUT_Font027);

    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);
}


uint8_t OnAction_Units(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    settingsGetPointer()->nonMetricalSystem = !(settingsGetPointer()->nonMetricalSystem);
    return EXIT_TO_MENU_WITH_LOGO;//UPDATE_DIVESETTINGS;
}


uint8_t OnAction_Colorscheme(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t newColorscheme;

    newColorscheme = settingsGetPointer()->tX_colorscheme + 1;

    if(newColorscheme > 3)
        newColorscheme = 0;

    settingsGetPointer()->tX_colorscheme = newColorscheme;
    GFX_use_colorscheme(newColorscheme);
    tHome_init_compass();
    return UNSPECIFIC_RETURN;
}


uint8_t OnAction_DebugInfo(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    settingsGetPointer()->showDebugInfo = !(settingsGetPointer()->showDebugInfo);
    return UPDATE_DIVESETTINGS;
}




/*
uint8_t OnAction_Design_t7ft		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings = settingsGetPointer();
    if((pSettings->design == 7) && pSettings->nonMetricalSystem)
        return EXIT_TO_MENU;
    pSettings->design = 7;
    pSettings->nonMetricalSystem = 1;
    tMenuEdit_set_on_off(StMSYS3_t7,	 0);
    tMenuEdit_set_on_off(StMSYS3_t7ft, 1);
    tMenuEdit_set_on_off(StMSYS3_t3,	 0);
    return UPDATE_DIVESETTINGS;
}


uint8_t OnAction_Design_t7			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings = settingsGetPointer();
    if((pSettings->design == 7) && (pSettings->nonMetricalSystem == 0))
        return EXIT_TO_MENU;
    pSettings->design = 7;
    pSettings->nonMetricalSystem = 0;
    tMenuEdit_set_on_off(StMSYS3_t7,	 1);
    tMenuEdit_set_on_off(StMSYS3_t7ft, 0);
    tMenuEdit_set_on_off(StMSYS3_t3,	 0);
    return UPDATE_DIVESETTINGS;
}


uint8_t OnAction_Design_t3			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings = settingsGetPointer();
    if(pSettings->design == 3)
        return EXIT_TO_MENU;
    pSettings->design = 3;
    pSettings->nonMetricalSystem = 0;
    tMenuEdit_set_on_off(StMSYS3_t7, 	0);
    tMenuEdit_set_on_off(StMSYS3_t7ft,0);
    tMenuEdit_set_on_off(StMSYS3_t3, 	1);
    return UPDATE_DIVESETTINGS;
}
*/


void openEdit_Customview(void)
{
    refresh_Customviews();

    write_field_button(StMSYS4_CViewTimeout,		400, 700, ME_Y_LINE1,  &FontT48, "");
    write_field_button(StMSYS4_CViewStandard,		400, 700, ME_Y_LINE2,  &FontT48, "");

    write_field_button(StMSYS4_CornerTimeout,		400, 700, ME_Y_LINE3,  &FontT48, "");
    write_field_button(StMSYS4_CornerStandard,		400, 700, ME_Y_LINE4,  &FontT48, "");

    write_field_button(StMSYS4_ExtraDisplay,		400, 700, ME_Y_LINE5,  &FontT48, "");
    write_field_button(StMSYS4_MotionCtrl,			400, 700, ME_Y_LINE6,  &FontT48, "");

    setEvent(StMSYS4_CViewTimeout,		(uint32_t)OnAction_CViewTimeout);
    setEvent(StMSYS4_CViewStandard,		(uint32_t)OnAction_CViewStandard);

    setEvent(StMSYS4_CornerTimeout,		(uint32_t)OnAction_CornerTimeout);
    setEvent(StMSYS4_CornerStandard,	(uint32_t)OnAction_CornerStandard);

    setEvent(StMSYS4_ExtraDisplay,		(uint32_t)OnAction_ExtraDisplay);
    setEvent(StMSYS4_MotionCtrl,		(uint32_t)OnAction_MotionCtrl);
}


void refresh_Customviews(void)
{
    char text[32];
    uint8_t textpointer = 0;

    // header
    text[0] = '\001';
    text[1] = TXT_2BYTE;
    text[2] = TXT2BYTE_Customviews;
    text[3] = 0;
    write_topline(text);

    // custom view center  return
    textpointer = 0;
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_CViewTimeout;
    textpointer += snprintf(&text[textpointer],11,"  %02u\016\016 %c\017",settingsGetPointer()->tX_customViewTimeout,TXT_Seconds);
    write_label_var(  30, 700, ME_Y_LINE1, &FontT48, text);

    // custom view center  primary
    text[0] = TXT_2BYTE;
    text[1] = TXT2BYTE_CViewStandard;
    text[2] = ' ';
    text[3] = ' ';
    switch(settingsGetPointer()->tX_customViewPrimary)
    {
    case CVIEW_sensors:
        text[4] = TXT_2BYTE;
        text[5] = TXT2BYTE_O2monitor;
        break;
    case CVIEW_sensors_mV:
        text[4] = TXT_2BYTE;
        text[5] = TXT2BYTE_O2voltage;
        break;
    case CVIEW_Compass:
        text[4] = TXT_2BYTE;
        text[5] = TXT2BYTE_Compass;
        break;
    case CVIEW_Decolist:
        text[4] = TXT_2BYTE;
        text[5] = TXT2BYTE_Decolist;
        break;
    case CVIEW_Tissues:
        text[4] = TXT_2BYTE;
        text[5] = TXT2BYTE_Tissues;
        break;
    case CVIEW_Profile:
        text[4] = TXT_2BYTE;
        text[5] = TXT2BYTE_Profile;
        break;
    case CVIEW_Gaslist:
        text[4] = TXT_2BYTE;
        text[5] = TXT2BYTE_Gaslist;
        break;
    case CVIEW_EADTime:
        text[4] = TXT_2BYTE;
        text[5] = TXT2BYTE_Info;
        break;
    case CVIEW_SummaryOfLeftCorner:
        text[4] = TXT_2BYTE;
        text[5] = TXT2BYTE_Summary;
        break;
    case CVIEW_noneOrDebug:
        text[4] = ' ';
        text[5] = '-';
        break;
    default:
        snprintf(&text[4],3,"%02u",settingsGetPointer()->tX_customViewPrimary);
    break;
    }
    text[6] = 0;
    write_label_var(  30, 700, ME_Y_LINE2, &FontT48, text);


    // field corner  return
    textpointer = 0;
    text[textpointer++] = TXT_2BYTE;
    text[textpointer++] = TXT2BYTE_CornerTimeout;
    textpointer += snprintf(&text[textpointer],11,"  %02u\016\016 %c\017",settingsGetPointer()->tX_userselectedLeftLowerCornerTimeout,TXT_Seconds);
    write_label_var(  30, 700, ME_Y_LINE3, &FontT48, text);

    // field corner  primary
    text[0] = TXT_2BYTE;
    text[1] = TXT2BYTE_CornerStandard;
    text[2] = ' ';
    text[3] = ' ';
    switch(settingsGetPointer()->tX_userselectedLeftLowerCornerPrimary)
    {
    /* Temperature */
    case LLC_Temperature:
        text[4] = TXT_Temperature;
        break;
    /* Average Depth */
    case LLC_AverageDepth:
        text[4] = TXT_AvgDepth;
        break;
    /* ppO2 */
    case LLC_ppO2:
        text[4] = TXT_ppO2;
        break;
    /* Stop Uhr */
    case LLC_Stopwatch:
        text[4] = TXT_Stopwatch;
        break;
    /* Ceiling */
    case LLC_Ceiling:
        text[4] = TXT_Ceiling;
        break;
    /* Future TTS */
    case LLC_FutureTTS:
        text[4] = TXT_FutureTTS;
        break;
    /* CNS */
    case LLC_CNS:
        text[4] = TXT_CNS;
        break;
    case LLC_GF:
    	text[4] = TXT_ActualGradient;
    	break;
#ifdef ENABLE_BOTTLE_SENSOR
    case LCC_BottleBar:
    	text[4] = TXT_AtemGasVorrat;
    	    	break;
#endif
    /* none */
    case LLC_Empty:
        text[4] = '-';
        break;
    default:
        snprintf(&text[4],2,"%u",settingsGetPointer()->tX_userselectedLeftLowerCornerPrimary);
    break;
    }
    text[5] = 0;
    write_label_var(  30, 700, ME_Y_LINE4, &FontT48, text);


    // extra display
    text[0] = TXT_2BYTE;
    text[1] = TXT2BYTE_ExtraDisplay;
    text[2] = ' ';
    text[3] = ' ';
    text[4] = TXT_2BYTE;
    switch(settingsGetPointer()->extraDisplay)
    {
    /* BigFont */
    case EXTRADISPLAY_BIGFONT:
        text[5] = TXT2BYTE_ExtraBigFont;
        break;
    /* DecoGame */
    case EXTRADISPLAY_DECOGAME:
        text[5] = TXT2BYTE_ExtraDecoGame;
        break;
    /* none */
    case EXTRADISPLAY_none:
        text[5] = TXT2BYTE_ExtraNone;
        break;
#ifdef ENABLE_BIGFONT_VX
    case EXTRADISPLAY_BIGFONT2:
        text[5] = TXT2BYTE_ExtraBigFontV2;
        break;
#endif
    default:
        snprintf(&text[4],2,"%u",settingsGetPointer()->extraDisplay);
    break;
    }
    text[6] = 0;
    write_label_var(  30, 700, ME_Y_LINE5, &FontT48, text);


    /* MotionCtrl */
    text[0] = TXT_2BYTE;
    text[1] = TXT2BYTE_MotionCtrl;
    text[2] = ' ';
    text[3] = ' ';
    text[4] = TXT_2BYTE;
    switch(settingsGetPointer()->MotionDetection)
    {
		case MOTION_DETECT_OFF:
			text[5] = TXT2BYTE_MoCtrlNone;
			break;
		case MOTION_DETECT_MOVE:
			text[5] = TXT2BYTE_MoCtrlPitch;
			break;
		case MOTION_DETECT_SECTOR:
			text[5] = TXT2BYTE_MoCtrlSector;
			break;
		case MOTION_DETECT_SCROLL:
			text[5] = TXT2BYTE_MoCtrlScroll;
					break;
		default:
			snprintf(&text[4],2,"%u",settingsGetPointer()->MotionDetection);
		break;
    }
    text[6] = 0;
    write_label_var(  30, 700, ME_Y_LINE6, &FontT48, text);

    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);
}


uint8_t OnAction_CViewTimeout(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t value;
    value = settingsGetPointer()->tX_customViewTimeout;

    if(value < 5)
        value = 5;
    else if(value < 10)
        value = 10;
    else if(value < 15)
        value = 15;
    else if(value < 20)
        value = 20;
    else if(value < 30)
        value = 30;
    else if(value < 45)
        value = 45;
    else if(value < 60)
        value = 60;
    else
        value = 0;

    settingsGetPointer()->tX_customViewTimeout = value;
    return UPDATE_DIVESETTINGS;
}


uint8_t OnAction_CViewStandard(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t newValue;
    switch(settingsGetPointer()->tX_customViewPrimary)
    {
    case CVIEW_sensors:
        newValue = CVIEW_sensors_mV;
        break;
    case CVIEW_sensors_mV:
        newValue = CVIEW_Compass;
        break;
    case CVIEW_Compass:
        newValue = CVIEW_Decolist;
        break;
    case CVIEW_Decolist:
        newValue = CVIEW_Tissues;
        break;
    case CVIEW_Tissues:
        newValue = CVIEW_Profile;
        break;
    case CVIEW_Profile:
        newValue = CVIEW_Gaslist;
        break;
    case CVIEW_Gaslist:
        newValue = CVIEW_EADTime;
        break;
    case CVIEW_EADTime:
        newValue = CVIEW_SummaryOfLeftCorner;
        break;
    case CVIEW_SummaryOfLeftCorner:
        newValue = CVIEW_noneOrDebug;
        break;
    case CVIEW_noneOrDebug:
    default:
         newValue = CVIEW_sensors;
        break;
    }
    settingsGetPointer()->tX_customViewPrimary = newValue;
    return UPDATE_DIVESETTINGS;
}


uint8_t OnAction_CornerTimeout(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t value;
    value = settingsGetPointer()->tX_userselectedLeftLowerCornerTimeout;

    if(value < 5)
        value = 5;
    else  if(value < 10)
        value = 10;
    else if(value < 15)
        value = 15;
    else if(value < 20)
        value = 20;
    else if(value < 30)
        value = 30;
    else if(value < 45)
        value = 45;
    else if(value < 60)
        value = 60;
    else
        value = 0;

    settingsGetPointer()->tX_userselectedLeftLowerCornerTimeout = value;
    return UPDATE_DIVESETTINGS;
}


uint8_t OnAction_CornerStandard(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t value;
    value = settingsGetPointer()->tX_userselectedLeftLowerCornerPrimary;

    value += 1;

    if(value >= LLC_END)
        value = 0;

    settingsGetPointer()->tX_userselectedLeftLowerCornerPrimary = value;
    return UPDATE_DIVESETTINGS;
}


uint8_t OnAction_ExtraDisplay	 (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t newValue;

    newValue = settingsGetPointer()->extraDisplay + 1;
    if(newValue == EXTRADISPLAY_DECOGAME)  /* Decogame not yet implemented */
    {
    	newValue++;
    }
    if(newValue >= EXTRADISPLAY_END)
    {
    	newValue = EXTRADISPLAY_none;
    }
    settingsGetPointer()->extraDisplay = newValue;
    return UNSPECIFIC_RETURN;
}


uint8_t OnAction_MotionCtrl	 (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t newValue;
    switch(settingsGetPointer()->MotionDetection)
    {
    case MOTION_DETECT_OFF:
        newValue = MOTION_DETECT_MOVE;
        break;
    case MOTION_DETECT_MOVE:
        newValue = MOTION_DETECT_SECTOR;
        break;
    case MOTION_DETECT_SECTOR:
        newValue = MOTION_DETECT_SCROLL;
        break;
    case MOTION_DETECT_SCROLL:
    	newValue = MOTION_DETECT_OFF;
    	break;
    default:
        newValue = MOTION_DETECT_OFF;
        break;
    }
    settingsGetPointer()->MotionDetection = newValue;
    InitMotionDetection();
    return UNSPECIFIC_RETURN;
}

void openEdit_Information(void)
{
    char text[70];

    infoPage = 0;

    text[0] = '\001';
    text[1] = TXT_Information;
    text[2] = 0;
    write_topline(text);

    text[0] = TXT_2BYTE;
    text[1] = TXT2BYTE_ButtonNext;
    text[2] = 0;

    write_field_button(StMSYS5_Info, 30, 800, ME_Y_LINE6,  &FontT48, text);

    setEvent(StMSYS5_Info, (uint32_t)OnAction_Information);
}


uint8_t OnAction_Information	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    resetEnterPressedToStateBeforeButtonAction();

    infoPage++;
    if(infoPage > 3)
        return EXIT_TO_MENU;
    else
        return UNSPECIFIC_RETURN;
}


void refresh_InformationPage(void)
{
    char text_header[5];
    char text_button[5];
    char text_content[256];
    uint8_t date[3], year,month,day;

    RTC_DateTypeDef Sdate, Sdate2;
    float temperature1, temperature2, voltage, offsetTemperature;

    //RTC_TimeTypeDef Stime;

    /*
        SDeviceLine batteryChargeCycles;
        SDeviceLine batteryChargeCompleteCycles;
        SDeviceLine temperatureMinimum;
        SDeviceLine temperatureMaximum;
        SDeviceLine depthMaximum;
        SDeviceLine diveCycles;
        SDeviceLine voltageMinimum;
    */

    switch(infoPage)
    {
    case 0:
        text_header[0] = '\001';
        text_header[1] = TXT_Information;
        text_header[2] = 0;

        write_label_var(  20, 800, ME_Y_LINE1, &FontT42, "Dive Computer OSTC4");
        write_label_var(  20, 800, ME_Y_LINE2, &FontT42, "Design heinrichs/weikamp");

        Sdate.Year = firmwareDataGetPointer()->release_year;
        Sdate.Month = firmwareDataGetPointer()->release_month;
        Sdate.Date = firmwareDataGetPointer()->release_day;

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
        snprintf(text_content,40,"Firmware release date:  %02d.%02d.%02d",date[0],date[1],date[2]);
        write_label_var(  20, 800, ME_Y_LINE3, &FontT42, text_content);
        write_label_var(  20, 800, ME_Y_LINE4, &FontT42, "for more information");
        write_label_var(  20, 800, ME_Y_LINE5, &FontT42, "info@heinrichsweikamp.com");

        text_button[0] = TXT_2BYTE;
        text_button[1] = TXT2BYTE_ButtonNext;
        text_button[2] = 0;
        break;

    case 1:
        text_header[0] = '\001';
        text_header[1] = TXT_2BYTE;
        text_header[2] = TXT2BYTE_Usage_Battery;
        text_header[3] = 0;

        text_content[0] = TXT_2BYTE;
        text_content[1] = TXT2BYTE_ChargeCycles;
        text_content[2] = 0;
        write_label_var(  20, 800, ME_Y_LINE1, &FontT42, text_content);
        snprintf(text_content,80,"%ld (%ld)",stateDeviceGetPointer()->batteryChargeCycles.value_int32,stateDeviceGetPointer()->batteryChargeCompleteCycles.value_int32);
        write_label_var(  20, 800, ME_Y_LINE2, &FontT42, text_content);

        translateDate(stateDeviceGetPointer()->batteryChargeCycles.date_rtc_dr, &Sdate);
        translateDate(stateDeviceGetPointer()->batteryChargeCompleteCycles.date_rtc_dr, &Sdate2);
        snprintf(text_content,80,"%u.%u.20%02u (%u.%u.20%02u)",Sdate.Date,Sdate.Month,Sdate.Year, Sdate2.Date,Sdate2.Month,Sdate2.Year);
        write_label_var(  20, 800, ME_Y_LINE3, &FontT42, text_content);

        text_content[0] = TXT_2BYTE;
        text_content[1] = TXT2BYTE_LowestVoltage;
        text_content[2] = 0;
        write_label_var(  20, 800, ME_Y_LINE4, &FontT42, text_content);

        translateDate(stateDeviceGetPointer()->voltageMinimum.date_rtc_dr, &Sdate);
        voltage = ((float)stateDeviceGetPointer()->voltageMinimum.value_int32) / 1000;
        snprintf(text_content,80,"%0.3fV (%u.%u.20%02u)",voltage, Sdate.Date,Sdate.Month,Sdate.Year);
        write_label_var(  20, 800, ME_Y_LINE5, &FontT42, text_content);

        text_button[0] = TXT_2BYTE;
        text_button[1] = TXT2BYTE_ButtonNext;
        text_button[2] = 0;
    break;

    case 2:
        text_header[0] = '\001';
        text_header[1] = TXT_2BYTE;
        text_header[2] = TXT2BYTE_Usage_Dives;
        text_header[3] = 0;

        text_content[0] = TXT_2BYTE;
        text_content[1] = TXT2BYTE_NumberOfDives;
        text_content[2] = 0;
        write_label_var(  20, 800, ME_Y_LINE1, &FontT42, text_content);

        snprintf(text_content,80,"%ld (%ld)",stateDeviceGetPointer()->diveCycles.value_int32,(stateDeviceGetPointer()->depthMaximum.value_int32 - 1000) / 100);
        write_label_var(  20, 800, ME_Y_LINE2, &FontT42, text_content);

        translateDate(stateDeviceGetPointer()->diveCycles.date_rtc_dr, &Sdate);
        translateDate(stateDeviceGetPointer()->depthMaximum.date_rtc_dr, &Sdate2);
        snprintf(text_content,80,"%u.%u.20%02u (%u.%u.20%02u)",Sdate.Date,Sdate.Month,Sdate.Year, Sdate2.Date,Sdate2.Month,Sdate2.Year);
        write_label_var(  20, 800, ME_Y_LINE3, &FontT42, text_content);

        text_content[0] = TXT_2BYTE;
        text_content[1] = TXT2BYTE_HoursOfOperation;
        text_content[2] = 0;
        write_label_var(  20, 800, ME_Y_LINE4, &FontT42, text_content);

        snprintf(text_content,80,"%ld",(stateDeviceGetPointer()->hoursOfOperation.value_int32)/3600);
        write_label_var(  20, 800, ME_Y_LINE5, &FontT42, text_content);

        text_button[0] = TXT_2BYTE;
        text_button[1] = TXT2BYTE_ButtonNext;
        text_button[2] = 0;
    break;

    case 3:
        text_header[0] = '\001';
        text_header[1] = TXT_2BYTE;
        text_header[2] = TXT2BYTE_Usage_Environment;
        text_header[3] = 0;

        text_content[0] = TXT_2BYTE;
        text_content[1] = TXT2BYTE_AmbientTemperature;
        text_content[2] = 0;
        write_label_var(  20, 800, ME_Y_LINE1, &FontT42, text_content);

        temperature1 = ((float)stateDeviceGetPointer()->temperatureMinimum.value_int32) / 100;
        temperature2 = ((float)stateDeviceGetPointer()->temperatureMaximum.value_int32) / 100;
        snprintf(text_content,80,"%0.2f\140C  /  %0.2f\140C",temperature1,temperature2);
        write_label_var(  20, 800, ME_Y_LINE2, &FontT42, text_content);

        translateDate(stateDeviceGetPointer()->temperatureMinimum.date_rtc_dr, &Sdate);
        translateDate(stateDeviceGetPointer()->temperatureMaximum.date_rtc_dr, &Sdate2);
        snprintf(text_content,80,"(%u.%u.20%02u  /  %u.%u.20%02u)",Sdate.Date,Sdate.Month,Sdate.Year, Sdate2.Date,Sdate2.Month,Sdate2.Year);
        write_label_var(  20, 800, ME_Y_LINE3, &FontT42, text_content);

        text_content[0] = TXT_2BYTE;
        text_content[1] = TXT2BYTE_Korrekturwerte;
        text_content[2] = 0;
        write_label_var(  20, 800, ME_Y_LINE4, &FontT42, text_content);

        offsetTemperature = ((float)settingsGetPointer()->offsetTemperature_centigrad) / 10;
        snprintf(text_content,80,"%i %s  /  %0.2f\140C",settingsGetPointer()->offsetPressure_mbar, TEXT_PRESSURE_UNIT, offsetTemperature);
        write_label_var(  20, 800, ME_Y_LINE5, &FontT42, text_content);

        text_button[0] = TXT_2BYTE;
        text_button[1] = TXT2BYTE_Exit;
        text_button[2] = 0;
    break;
    }

    write_topline(text_header);
    tMenuEdit_newButtonText(StMSYS5_Info, text_button);
    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonNext,0);
}


void openEdit_Reset(void)
{
    char text[32];

    text[0] = '\001';
    text[1] = TXT_2BYTE;
    text[2] = TXT2BYTE_ResetMenu;
    text[3] = 0;
    write_topline(text);

    text[0] = TXT_2BYTE;
    text[1] = TXT2BYTE_LogbookOffset;
    text[7] = 0;

    write_label_var(  30, 400, ME_Y_LINE1, &FontT48, text);

    write_field_udigit(StMSYS6_LogbookOffset,420, 800, ME_Y_LINE1, &FontT48, "####", settingsGetPointer()->logbookOffset,0,0,0);

    text[0] = TXT_2BYTE;
    text[2] = 0;

    text[1] = TXT2BYTE_ResetAll;
    write_field_button(StMSYS6_ResetAll,		30, 800, ME_Y_LINE2,  &FontT48, text);

    text[1] = TXT2BYTE_ResetDeco;
    write_field_button(StMSYS6_ResetDeco,		30, 800, ME_Y_LINE3,  &FontT48, text);

    text[1] = TXT2BYTE_Reboot;
    write_field_button(StMSYS6_Reboot,			30, 800, ME_Y_LINE4,  &FontT48, text);

    text[1] = TXT2BYTE_Maintenance;
    write_field_button(StMSYS6_Maintenance,	30, 800, ME_Y_LINE5,  &FontT48, text);

#ifndef RESETLOGBLOCK
    text[1] = TXT2BYTE_ResetLogbook;
    write_field_button(StMSYS6_ResetLogbook,30, 800, ME_Y_LINE6,  &FontT48, text);
#else
    text[0] = '\021';
    text[1] = TXT_2BYTE;
    text[2] = TXT2BYTE_ResetLogbook;
    text[3] = 0;
    write_field_button(StMSYS6_ResetLogbook,30, 800, ME_Y_LINE6,  &FontT48, text);
    text[0] = TXT_2BYTE;
    text[2] = 0;
#endif

    setEvent(StMSYS6_LogbookOffset,	(uint32_t)OnAction_LogbookOffset);
    setEvent(StMSYS6_ResetAll, 			(uint32_t)OnAction_Confirm);
    setEvent(StMSYS6_ResetDeco, 		(uint32_t)OnAction_Confirm);
    setEvent(StMSYS6_Reboot, 				(uint32_t)OnAction_Confirm);
    setEvent(StMSYS6_Maintenance,		(uint32_t)OnAction_Confirm);
#ifndef RESETLOGBLOCK
    setEvent(StMSYS6_ResetLogbook,	(uint32_t)OnAction_Confirm);
#else
    setEvent(StMSYS6_ResetLogbook,	(uint32_t)OnAction_Nothing);
#endif

    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);
}


void openEdit_ResetConfirmation(uint32_t editIdOfCaller)
{
    char text[32];

    resetMenuEdit(CLUT_MenuPageSystem);

    text[0] = '\001';
    text[1] = TXT_2BYTE;
    text[2] = TXT2BYTE_AreYouSure;
    text[3] = 0;
    write_topline(text);

    text[0] = TXT_2BYTE;
    text[2] = 0;
    text[1] = TXT2BYTE_Abort;

    write_field_button(StMSYS6_Exit,				30, 800, ME_Y_LINE1,  &FontT48, text);

    text[2] = 0;
    text[3] = 0;
    switch(editIdOfCaller)
    {
    case StMSYS6_Reboot:
    case StMSYS6_RebootRTE:
    case StMSYS6_RebootMainCPU:
        text[1] = TXT2BYTE_RebootMainCPU;
        write_field_button(StMSYS6_RebootMainCPU,	30, 800, ME_Y_LINE2,  &FontT48, text);
        text[1] = TXT2BYTE_RebootRTE;
        write_field_button(StMSYS6_RebootRTE,			30, 800, ME_Y_LINE3,  &FontT48, text);
        setEvent(StMSYS6_Exit, (uint32_t)OnAction_Exit);
        setEvent(StMSYS6_RebootMainCPU, (uint32_t)OnAction_RebootMainCPU);
        setEvent(StMSYS6_RebootRTE, 		(uint32_t)OnAction_RebootRTE);
        text[0] = '\025';
        text[1] = TXT_2BYTE;
        text[2] = TXT2BYTE_DecoDataLost;
        text[3] = 0;
        write_label_var(			30, 800, ME_Y_LINE4,  &FontT48, text);
        break;

    case StMSYS6_ResetDeco:
        text[1] = TXT2BYTE_ResetDeco;
        write_field_button(editIdOfCaller,			30, 800, ME_Y_LINE2,  &FontT48, text);
        setEvent(StMSYS6_Exit, (uint32_t)OnAction_Exit);
        setEvent(editIdOfCaller, (uint32_t)OnAction_ResetDeco);
        text[0] = '\025';
        text[1] = TXT_2BYTE;
        text[2] = TXT2BYTE_DecoDataLost;
        text[3] = 0;
        write_label_var(			30, 800, ME_Y_LINE4,  &FontT48, text);
        break;

    case StMSYS6_ResetAll:
        text[1] = TXT2BYTE_ResetAll;
        write_field_button(editIdOfCaller,			30, 800, ME_Y_LINE2,  &FontT48, text);
        setEvent(StMSYS6_Exit, (uint32_t)OnAction_Exit);
        setEvent(editIdOfCaller, (uint32_t)OnAction_ResetAll);
        break;

    case StMSYS6_ResetLogbook:
        text[1] = TXT2BYTE_ResetLogbook;
        write_field_button(editIdOfCaller,			30, 800, ME_Y_LINE2,  &FontT48, text);
        setEvent(StMSYS6_Exit, (uint32_t)OnAction_Exit);
        setEvent(editIdOfCaller, (uint32_t)OnAction_ResetLogbook);
        break;

    case StMSYS6_Maintenance:
    case StMSYS6_SetBattCharge:
    case StMSYS6_SetSampleIndx:
        text[0] = TXT_2BYTE;
        text[1] = TXT2BYTE_SetFactoryDefaults;
        text[2] = 0;
        write_field_button(StMSYS6_SetFactoryBC,			30, 800, ME_Y_LINE2,  &FontT48, text);

#ifdef ENABLE_ANALYSE_SAMPLES
        text[0] = TXT_2BYTE;
        text[1] = TXT2BYTE_SetSampleIndex;
        text[2] = 0;
        write_field_button(StMSYS6_SetSampleIndx,			30, 800, ME_Y_LINE3,  &FontT48, text);
#endif


        if(stateRealGetPointer()->lifeData.battery_charge == 0)
        {
            text[0] = TXT_2BYTE;
            text[1] = TXT2BYTE_SetBatteryCharge;
            text[2] = 0;
            snprintf(&text[2],10,": %u%%",settingsGetPointer()->lastKnownBatteryPercentage);
#ifdef ENABLE_ANALYSE_SAMPLES
            write_field_button(StMSYS6_SetBattCharge,			30, 800, ME_Y_LINE4,  &FontT48, text);
#else
            write_field_button(StMSYS6_SetBattCharge,			30, 800, ME_Y_LINE3,  &FontT48, text);
#endif

            setEvent(StMSYS6_Exit, (uint32_t)OnAction_Exit);
            setEvent(StMSYS6_SetFactoryBC, (uint32_t)OnAction_SetFactoryDefaults);
#ifdef ENABLE_ANALYSE_SAMPLES
            setEvent(StMSYS6_SetSampleIndx, (uint32_t)OnAction_RecoverSampleIdx);
#endif
            setEvent(StMSYS6_SetBattCharge, (uint32_t)OnAction_SetBatteryCharge);
        }
        else
        {
            setEvent(StMSYS6_Exit, (uint32_t)OnAction_Exit);
            setEvent(StMSYS6_SetFactoryBC, (uint32_t)OnAction_SetFactoryDefaults);
#ifdef ENABLE_ANALYSE_SAMPLES
            setEvent(StMSYS6_SetSampleIndx, (uint32_t)OnAction_RecoverSampleIdx);
#endif
        }
//		write_field_button(StMSYS6_ScreenTest,			30, 800, ME_Y_LINE3,  &FontT48, "Screen Test");
//		setEvent(StMSYS6_ScreenTest, (uint32_t)OnAction_ScreenTest);

        text[0] = TXT_2BYTE;
        text[1] = TXT2BYTE_WarnBatteryLow;
        text[2] = 0;
        snprintf(&text[2],10,": %01.2fV",stateRealGetPointer()->lifeData.battery_voltage);
        write_label_var(  30, 800, ME_Y_LINE5, &FontT42, text);
        
        snprintf(&text[0],30,"Code: %X",getLicence());
        write_label_var(  30, 800, ME_Y_LINE6, &FontT42, text);
        break;

    }

    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);
}

uint8_t OnAction_LogbookOffset(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t digitContentNew;
    uint32_t newOffset;

    if(action == ACTION_BUTTON_ENTER)
        return digitContent;

    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &newOffset, 0, 0, 0);
        if(newOffset > 9000)
            newOffset = 0;
        tMenuEdit_newInput(editId, newOffset, 0, 0, 0);
        settingsGetPointer()->logbookOffset = (uint16_t)newOffset;
        return UPDATE_DIVESETTINGS;
    }

    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if(digitContentNew > '9')
            digitContentNew = '0';
        return digitContentNew;
    }

    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - 1;
        if(digitContentNew < '0')
            digitContentNew = '9';
        return digitContentNew;
    }
    return UNSPECIFIC_RETURN;
}

uint8_t OnAction_Nothing			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    return UNSPECIFIC_RETURN;
}

uint8_t OnAction_Exit					(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    return EXIT_TO_MENU;
}
uint8_t OnAction_Confirm				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    openEdit_ResetConfirmation(editId);
    return UNSPECIFIC_RETURN;
}

uint8_t OnAction_RebootRTE				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    MX_SmallCPU_Reset_To_Standard();
    return EXIT_TO_MENU;
}

uint8_t OnAction_ResetDeco		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    clearDeco();
    return EXIT_TO_MENU;
}

uint8_t OnAction_ResetAll			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    set_settings_to_Standard();
    check_and_correct_settings();

    return UPDATE_AND_EXIT_TO_HOME;
}

uint8_t OnAction_ResetLogbook	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    write_label_var( 430, 740, 350, &FontT42, "Wait");
    ext_flash_erase_logbook();

    SSettings * pSettings = settingsGetPointer();
    pSettings->lastDiveLogId = 255;
    pSettings->logFlashNextSampleStartAddress = 0;

    return EXIT_TO_MENU;
}

uint8_t OnAction_RebootMainCPU		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    settingsGetPointer()->showDebugInfo = 0;
    extern	uint8_t bootToBootloader;
    bootToBootloader = 1;
    return UNSPECIFIC_RETURN;
}


uint8_t OnAction_SetFactoryDefaults(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    settingsWriteFactoryDefaults(settingsGetPointer()->ButtonResponsiveness[3], settingsGetPointer()->buttonBalance);
    return EXIT_TO_MENU;
}

#ifdef ENABLE_ANALYSE_SAMPLES
uint8_t OnAction_RecoverSampleIdx(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
	char text[32];
	char strResult[20];

	ext_flash_AnalyseSampleBuffer(strResult);
    snprintf(&text[0],30,"Ring: %s",strResult); //"Code: %X",settingsGetPointer()->logFlashNextSampleStartAddress); //getLicence());
    write_label_var(  30, 800, ME_Y_LINE6, &FontT42, text);
    return UNSPECIFIC_RETURN;
}
#endif

uint8_t OnAction_SetBatteryCharge(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    setBatteryPercentage(settingsGetPointer()->lastKnownBatteryPercentage);
//	setBatteryPercentage(100);
    return EXIT_TO_MENU;
}

#ifdef SCREENTEST
uint8_t OnAction_ScreenTest		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    static uint8_t FrameCount = 1; // 0 is invisible frame
    char text[5];
    GFX_DrawCfgScreen	tTestScreen;
    tTestScreen.FBStartAdress = 0;
    tTestScreen.ImageHeight = 480;
    tTestScreen.ImageWidth = 800;
    tTestScreen.LayerIndex = 1;

    set_globalState(StMSYS6_ScreenTest);
    tTestScreen.FBStartAdress = getFrameByNumber(FrameCount);
    if(tTestScreen.FBStartAdress == 0)
    {
        extern	uint8_t bootToBootloader;
        bootToBootloader = 1;
    }
    GFX_fill_buffer(tTestScreen.FBStartAdress, 0xFF, FrameCount);
    snprintf(text,5,"%u",FrameCount);
    Gfx_write_label_var(&tTestScreen,  10,100,10,&FontT48,CLUT_Font027,text);
    GFX_SetFramesTopBottom(tTestScreen.FBStartAdress, NULL,480);
    FrameCount++;
}
#endif
/*
uint8_t OnAction_TestCLog	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    write_label_var( 430, 740, 350, &FontT42, "Wait");

    test_log_only(20, 5);
    test_log_only(30, 10);
    ext_flash_write_settings();
    return EXIT_TO_MENU;
}
*/

