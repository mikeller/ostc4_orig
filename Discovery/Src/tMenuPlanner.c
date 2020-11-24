///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tMenuPlanner.c
/// \brief  Deco Planner and Simulator
/// \author heinrichs weikamp gmbh
/// \date   19-Dec-2014
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
#include <stdio.h>
#include <string.h>
#include "tMenu.h"
#include "tStructure.h"
#include "tMenuPlanner.h"
#include "text_multilanguage.h"
#include "settings.h"
#include "unit.h"

#define STD_depth (50)
#define STD_intervall (0)
#define STD_divetime (20)

uint16_t tMplan_depth_meter = STD_depth, tMplan_intervall_time_minutes = STD_intervall, tMplan_dive_time_minutes = STD_divetime;
uint16_t tMplan_depth_editor;

/* Exported functions --------------------------------------------------------*/

uint32_t tMPlanner_refresh(uint8_t line, char *text, uint16_t *tab, char *subtext)
{
    uint8_t textPointer;

    textPointer = 0;
    *tab = 400;//550;
    *subtext = 0;

    uint8_t  tMplan_gasConsumTravel = settingsGetPointer()->gasConsumption_travel_l_min;
    uint8_t  tMplan_gasConsumDeco = settingsGetPointer()->gasConsumption_deco_l_min;

//	SSettings *data = settingsGetPointer();

/*
    if(line == 0)
        strcpy(subtext,
        "\005"
        "a b c d e f g h i k l"
        "\n\r"
        "m n o p q r s t u v w"
        "\n\r"
        "x y z A B"
        "\006"
        );
*/
    if(line == 0)
    {
        tMplan_depth_meter = STD_depth;
        tMplan_intervall_time_minutes = STD_intervall;
        tMplan_dive_time_minutes = STD_divetime;
    }

    if((line == 0) || (line == 1))
    {
            text[textPointer++] = TXT_2BYTE;
            text[textPointer++] = TXT2BYTE_StartSimulator;
    }
    strcpy(&text[textPointer],"\n\r");
    textPointer += 2;
    if((line == 0) || (line == 2))
    {
        text[textPointer++] = TXT_2BYTE;
        text[textPointer++] = TXT2BYTE_Intervall;
        text[textPointer++] = '\t';
        textPointer += snprintf(&text[textPointer],30,"%u \016\016min\017",tMplan_intervall_time_minutes);
    }
    strcpy(&text[textPointer],"\n\r");
    textPointer += 2;
    if((line == 0) || (line == 3))
    {
        text[textPointer++] = TXT_2BYTE;
        text[textPointer++] = TXT2BYTE_SimDiveTime;
        text[textPointer++] = '\t';
        textPointer += snprintf(&text[textPointer],30,"%u \016\016min\017",tMplan_dive_time_minutes);
    }
    strcpy(&text[textPointer],"\n\r");
    textPointer += 2;
    if((line == 0) || (line == 4))
    {
        text[textPointer++] = TXT_2BYTE;
        text[textPointer++] = TXT2BYTE_SimMaxDepth;
        text[textPointer++] = '\t';
        textPointer += snprintf(&text[textPointer],30,"%u\016\016 %c%c\017",
            unit_depth_integer(tMplan_depth_meter),
            unit_depth_char1(),
            unit_depth_char2()
        );
    }
    strcpy(&text[textPointer],"\n\r");
    textPointer += 2;
    if((line == 0) || (line == 5))
    {
        text[textPointer++] = TXT_2BYTE;
        text[textPointer++] = TXT2BYTE_CalculateDeco;
    }
    strcpy(&text[textPointer],"\n\r");
    textPointer += 2;

    if((line == 0) || (line == 6))
    {
        text[textPointer++] = TXT_2BYTE;
        text[textPointer++] = TXT2BYTE_SimConsumption;
        text[textPointer++] = '\t';
        textPointer += snprintf(&text[textPointer],30,
            "%u"
            "\016\016 l\\min\017",
            tMplan_gasConsumTravel);
        text[textPointer++] = ' ';
        text[textPointer++] = ' ';
        textPointer += snprintf(&text[textPointer],30,
            "\016\016deco\017"
            " %u"
            "\016\016 l\\min\017",
            tMplan_gasConsumDeco);
    }
    strcpy(&text[textPointer],"\n\r");
    textPointer += 2;

    return StMPLAN;
}

