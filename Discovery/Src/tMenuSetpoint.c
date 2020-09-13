///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tMenuSetpoint.c
/// \brief
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
#include "tMenu.h"
#include "tMenuSetpoint.h"
#include "unit.h"

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

uint32_t tMSP_refresh(uint8_t line,
                      char *text,
                      uint16_t *tab,
                      char *subtext)
{
    const SSetpointLine * pSetpointLine;

    uint8_t textPointer, setpoint_cbar, sp_high, depthUp, first; //  active

    if(actual_menu_content == MENU_SURFACE)
        pSetpointLine = settingsGetPointer()->setpoint;
    else
        pSetpointLine = stateUsed->diveSettings.setpoint;

    textPointer = 0;
    *tab = 130;
    *subtext = 0;

    for(int spId=1;spId<=NUM_GASES;spId++)
    {
        if(line && (line != spId))
        {
                first = pSetpointLine[spId].note.ub.first;
                if(first == 0)
                {
                    strcpy(&text[textPointer],
                        "\t"
                        "\177"
                        "*"
                        "\n\r"
                    );
                    textPointer += 5;
                }
                else
                {
                    strcpy(&text[textPointer],"\n\r");
                    textPointer += 2;
                }
        }
        else
        {
            setpoint_cbar = pSetpointLine[spId].setpoint_cbar;
            depthUp = pSetpointLine[spId].depth_meter;
            //active = pSetpointLine[spId].note.ub.active;
            first = pSetpointLine[spId].note.ub.first;

            strcpy(&text[textPointer],"\020"); // if(active) always active
            textPointer += 1;

            sp_high = setpoint_cbar / 100;

            text[textPointer++] = 'S';
            text[textPointer++] = 'P';
            text[textPointer++] = '0' + spId;
            text[textPointer++] = '\t';

            if((first == 0) || (actual_menu_content != MENU_SURFACE))
                strcpy(&text[textPointer++],"\177");

            char color = '\031';
            if(depthUp)
                color = '\020';

            textPointer += snprintf(&text[textPointer], 57,
                "* "
                "%u.%02u"
                "\016\016"
                " bar"
                "\017"
                "\034"
                "   "
                "\016\016"
                " "
                "\017"
                "%c"
                "%3u"
                "\016\016"
                " %c%c"
                "\017"
                "\035"
                "\n\r",
                sp_high, setpoint_cbar - (100 * sp_high),
                color,
                unit_depth_integer(depthUp),
                unit_depth_char1(),
                unit_depth_char2()
            );
        }
    }
    if((actual_menu_content != MENU_SURFACE) /*&& (line == 0)*/)
    {
        text[textPointer++] = '\020';
        text[textPointer++] = TXT_2BYTE;
        text[textPointer++] = TXT2BYTE_UseSensor;
        text[textPointer++] = '\n';
        text[textPointer++] = '\r';
        text[textPointer++] = 0;
    }
    return StMSP;
}
