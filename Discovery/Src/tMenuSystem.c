///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tMenuSystem.c
/// \brief  Main Template file for Menu Page System settings
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
#include "tMenu.h"
#include "tMenuSystem.h"
#include "tHome.h"  // for enum CUSTOMVIEWS and init_t7_compass()

/* Private function prototypes -----------------------------------------------*/
char customview_TXT2BYTE_helper(uint8_t customViewId);

/* Exported functions --------------------------------------------------------*/

uint32_t tMSystem_refresh(uint8_t line, char *text, uint16_t *tab, char *subtext)
{
    SSettings *data;
    uint8_t textPointer;
    uint8_t dateFormat;
    uint8_t RTEhigh, RTElow;
    RTC_DateTypeDef Sdate;
    RTC_TimeTypeDef Stime;
    const SDiveState * pStateReal = stateRealGetPointer();

    data = settingsGetPointer();
    textPointer = 0;
    *tab = 300;
    *subtext = 0;

    // dive mode
    if(actual_menu_content != MENU_SURFACE)
    {
        uint8_t id;

        for(int i=0; i<6;i++)
        {
            id = cv_changelist[i];
            text[textPointer++] = '\006' - CHECK_BIT_THOME(data->cv_configuration,id);
            text[textPointer++] = ' ';
            textPointer += snprintf(&text[textPointer], 60,
                "%c%c\n\r",
                TXT_2BYTE, customview_TXT2BYTE_helper(id)
            );
        }
        text[textPointer] = 0;

        return StMSYS;
    }

    // surface mode
    getActualRTEandFONTversion(&RTEhigh,&RTElow,NULL,NULL);

    if((RTEhigh == 0xFF) || (RTElow == 0xFF))
    {
        RTEhigh = 0;
        RTElow = 0;
    }

    if((line == 0) || (line == 1))
    {
        translateDate(pStateReal->lifeData.dateBinaryFormat, &Sdate);
        translateTime(pStateReal->lifeData.timeBinaryFormat, &Stime);

        dateFormat = data->date_format;

        textPointer += snprintf(&text[textPointer], 40,
            "Date"
            "\016\016"
            " "
        );

        if(dateFormat == DDMMYY)
        {
            textPointer += snprintf(&text[textPointer], 40,
                "DDMMYY"
                "\017"
                "\t"
                "%02d-%02d-%02d"
                "  "
                , Sdate.Date, Sdate.Month, 2000 + Sdate.Year
            );
        }
        else
        if(dateFormat == MMDDYY)
        {
            textPointer += snprintf(&text[textPointer], 40,
                "MMDDYY"
                "\017"
                "\t"
                "%02d-%02d-%02d"
                "  "
                ,Sdate.Month, Sdate.Date, 2000 + Sdate.Year
            );
        }
        else
        if(dateFormat == YYMMDD)
        {
            textPointer += snprintf(&text[textPointer], 40,
                "YYMMDD"
                "\017"
                "\t"
                "%02d-%02d-%02d"
                "  "
                , 2000 + Sdate.Year, Sdate.Month, Sdate.Date
            );
        }

        textPointer += snprintf(&text[textPointer], 60,
            "%02d:%02d:%02d"
            "\n\r"
            ,Stime.Hours, Stime.Minutes, Stime.Seconds
        );
    }
    else
    {
        strcpy(&text[textPointer],"\n\r");
        textPointer += 2;
    }

    if((line == 0) || (line == 2))
    {
        text[textPointer++] = TXT_Language;
        text[textPointer++] = '\t';
        text[textPointer++] = TXT_LanguageName;
        text[textPointer++] = '\n';
        text[textPointer++] = '\r';
        text[textPointer] = 0;
    }
    else
    {
        strcpy(&text[textPointer],"\n\r");
        textPointer += 2;
    }

    if((line == 0) || (line == 3))
    {
        text[textPointer++] = TXT_2BYTE;
        text[textPointer++] = TXT2BYTE_Layout;
        text[textPointer++] = '\t';

        if(!data->showDebugInfo)
        {
            text[textPointer++] = TXT_2BYTE;
            if(data->nonMetricalSystem == 0)
                text[textPointer++] = TXT2BYTE_Units_metric;
            else
                text[textPointer++] = TXT2BYTE_Units_feet;

            if(data->tX_colorscheme != 0)
            {
                text[textPointer++] = ' ';
                text[textPointer++] = ' ';
                text[textPointer++] = ' ';
                text[textPointer++] = ' ';
                text[textPointer++] = '\027';
                text[textPointer++] = '/';
                text[textPointer++] = ' ';
                text[textPointer++] = '0' + data->tX_colorscheme;
                text[textPointer++] = '\020';
            }
        }
        else
        {
            if(data->nonMetricalSystem == 0)
            {
            text[textPointer++] = 'm';
            text[textPointer++] = '/';
            text[textPointer++] = 'C';
            }
            else
            {
            text[textPointer++] = 'f';
            text[textPointer++] = 't';
            text[textPointer++] = '/';
            text[textPointer++] = 'F';
            }

            text[textPointer++] = ' ';
            text[textPointer++] = ' ';
            text[textPointer++] = ' ';
            text[textPointer++] = ' ';
            text[textPointer++] = '\027';
            text[textPointer++] = '/';
            text[textPointer++] = ' ';
            text[textPointer++] = '0' + data->tX_colorscheme;
            text[textPointer++] = '\020';
            text[textPointer++] = ' ';
            text[textPointer++] = ' ';
            text[textPointer++] = 'd';
            text[textPointer++] = 'e';
            text[textPointer++] = 'b';
            text[textPointer++] = 'u';
            text[textPointer++] = 'g';
        }

        text[textPointer++] = '\n';
        text[textPointer++] = '\r';
        text[textPointer] = 0;
    }
    else
    {
        strcpy(&text[textPointer],"\n\r");
        textPointer += 2;
    }

    if((line == 0) || (line == 4))
    {
        text[textPointer++] = TXT_2BYTE;
        text[textPointer++] = TXT2BYTE_Customviews;//TXT2BYTE_ShowDebug;
    }
    text[textPointer++] = '\n';
    text[textPointer++] = '\r';
    text[textPointer] = 0;

    if((line == 0) || (line == 5))
    {
        text[textPointer++] = TXT_Information;
        text[textPointer++] = '\t';
        textPointer += snprintf(&text[textPointer],29,"RTE %u.%u  OS %i.%i.%i"
            ,RTEhigh
            ,RTElow
            ,firmwareDataGetPointer()->versionFirst
            ,firmwareDataGetPointer()->versionSecond
            ,firmwareDataGetPointer()->versionThird
        );
    }
    strcpy(&text[textPointer],"\n\r");
    textPointer += 2;

    if((line == 0) || (line == 6))
    {
        text[textPointer++] = TXT_2BYTE;
        text[textPointer++] = TXT2BYTE_ResetMenu;
        text[textPointer] = 0;
    }
    strcpy(&text[textPointer],"\n\r");
    textPointer += 2;

    return StMSYS;
}


/* Private functions ---------------------------------------------------------*/

char customview_TXT2BYTE_helper(uint8_t customViewId)
{
    char text = 0;

    switch(customViewId)
    {
    case CVIEW_sensors:
        text = TXT2BYTE_O2monitor;
        break;
    case CVIEW_sensors_mV:
        text = TXT2BYTE_O2voltage;
        break;
    case CVIEW_Compass:
        text = TXT2BYTE_Compass;
        break;
    case CVIEW_Decolist:
        text = TXT2BYTE_Decolist;
        break;
    case CVIEW_Tissues:
        text = TXT2BYTE_Tissues;
        break;
    case CVIEW_Profile:
        text = TXT2BYTE_Profile;
        break;
    case CVIEW_Gaslist:
        text = TXT2BYTE_Gaslist;
        break;
    case CVIEW_EADTime:
        text = TXT2BYTE_Info;
        break;
    case CVIEW_SummaryOfLeftCorner:
        text = TXT2BYTE_Summary;
        break;
    case CVIEW_noneOrDebug:
        break;
    default:
        break;
    }
    return text;
}
