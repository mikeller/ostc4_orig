///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tMenuEditSetpoint.c
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
#include "tMenuEditSetpoint.h"

#include "check_warning.h"
#include "gfx_fonts.h"
#include "tMenuEdit.h"
#include "unit.h"

/* Private types -------------------------------------------------------------*/
typedef struct
{
    uint8_t spID;
    SSetpointLine * pSetpointLine;
} SEditSetpointPage;


/* Private variables ---------------------------------------------------------*/
SEditSetpointPage editSetpointPage;

/* Private function prototypes -----------------------------------------------*/

void openEdit_DiveSetpointSelect(uint8_t line);
void openEdit_DiveSetpointSelect_Subroutine(uint8_t line);

/* Announced function prototypes -----------------------------------------------*/
uint8_t OnAction_SP_Setpoint    (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_SP_Depth       (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t On                      (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
//uint8_t OnAction_SP_First     (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
//uint8_t OnAction_SP_Active    (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
//uint8_t OnAction_SP_DM_Select (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_SP_DM_Sensor1	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_SP_DM_Sensor2	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_SP_DM_Sensor3	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);

/* Exported functions --------------------------------------------------------*/

void openEdit_Setpoint(uint8_t line)
{
    uint8_t useSensorSubMenu = 0;

    /* dive mode */
    if(actual_menu_content != MENU_SURFACE)
    {
        uint8_t setpointCbar, actualGasID;
        SDiveState *pState;
        setpointCbar = 100;

        if(actual_menu_content == MENU_DIVE_REAL)
            pState = stateRealGetPointerWrite();
        else
            pState = stateSimGetPointerWrite();

        // actualGasID
        if(pState->diveSettings.diveMode != DIVEMODE_CCR)
        {
            actualGasID = pState->lifeData.lastDiluent_GasIdInSettings;
            if((actualGasID <= NUM_OFFSET_DILUENT) || (actualGasID > NUM_GASES + NUM_OFFSET_DILUENT))
                actualGasID = NUM_OFFSET_DILUENT + 1;
        }
        else
            actualGasID = pState->lifeData.actualGas.GasIdInSettings;

        // setpointCbar, CCR_Mode and sensor menu
        if(line < 6)
        {
            setpointCbar = pState->diveSettings.setpoint[line].setpoint_cbar;
            pState->diveSettings.CCR_Mode = CCRMODE_FixedSetpoint;

            // BetterSetpoint warning only once
            if(actualBetterSetpointId() == line)
            {
                uint8_t depth;
                depth = pState->diveSettings.setpoint[line].depth_meter;
                // BetterSetpoint warning only once -> clear active
                for(int i=0; i<=NUM_GASES; i++)
                {
                    pState->diveSettings.setpoint[i].note.ub.first = 0;
                    if(pState->diveSettings.setpoint[i].depth_meter <= depth)
                        pState->diveSettings.setpoint[i].note.ub.active = 0;
                }
                pState->diveSettings.setpoint[line].note.ub.first = 1;
            }
        }
        else
        {
            if(pState->diveSettings.CCR_Mode != CCRMODE_Sensors)
            {
                /* setpoint_cbar will be written by updateSetpointStateUsed() in main.c loop */
                setpointCbar = 255;
                pState->diveSettings.CCR_Mode = CCRMODE_Sensors;
            }
            else
            {
                useSensorSubMenu = 1;
            }
        }

        setActualGas_DM(&pState->lifeData,actualGasID,setpointCbar);

        if(pState->diveSettings.diveMode != DIVEMODE_CCR)
        {
            pState->diveSettings.diveMode = DIVEMODE_CCR;
            unblock_diluent_page();
        }

        clear_warning_fallback();

        if(!useSensorSubMenu)
        {
            exitMenuEdit_to_Home();
        }
        else // entire sub menu during dive to select sensors active
        {
            set_globalState_Menu_Line(line);
            resetMenuEdit(CLUT_MenuPageGasSP);

            char text[3];
            uint8_t sensorActive[3];

            text[0] = '\001';
            text[1] = TXT_o2Sensors;
            text[2] = 0;
            write_topline(text);

            write_label_var(  96, 340, ME_Y_LINE1, &FontT48, "Sensor 1");
            write_label_var(  96, 340, ME_Y_LINE2, &FontT48, "Sensor 2");
            write_label_var(  96, 340, ME_Y_LINE3, &FontT48, "Sensor 3");

            sensorActive[0] = 1;
            sensorActive[1] = 1;
            sensorActive[2] = 1;
            if(pState->diveSettings.ppo2sensors_deactivated & 1)
                sensorActive[0] = 0;
            if(pState->diveSettings.ppo2sensors_deactivated & 2)
                sensorActive[1] = 0;
            if(pState->diveSettings.ppo2sensors_deactivated & 4)
                sensorActive[2] = 0;

            write_field_on_off(StMSP_Sensor1,	 30, 95, ME_Y_LINE1,  &FontT48, "", sensorActive[0]);
            write_field_on_off(StMSP_Sensor2,	 30, 95, ME_Y_LINE2,  &FontT48, "", sensorActive[1]);
            write_field_on_off(StMSP_Sensor3,	 30, 95, ME_Y_LINE3,  &FontT48, "", sensorActive[2]);

            setEvent(StMSP_Sensor1, (uint32_t)OnAction_SP_DM_Sensor1);
            setEvent(StMSP_Sensor2, (uint32_t)OnAction_SP_DM_Sensor2);
            setEvent(StMSP_Sensor3, (uint32_t)OnAction_SP_DM_Sensor3);
        }
        return;
    }
    else
    {
        /* surface mode */
        uint8_t spId, setpoint_cbar, sp_high, depthDeco, first;
        // uint8_t active,
        char text[70];
        uint8_t textPointer;
        uint16_t y_line;

        set_globalState_Menu_Line(line);

        resetMenuEdit(CLUT_MenuPageGasSP);

        spId = line;
        editSetpointPage.spID = spId;
        SSettings *data = settingsGetPointer();
        editSetpointPage.pSetpointLine = data->setpoint;

        setpoint_cbar = editSetpointPage.pSetpointLine[spId].setpoint_cbar;
        depthDeco = editSetpointPage.pSetpointLine[spId].depth_meter;
        //active = editSetpointPage.pSetpointLine[spId].note.ub.active;
        first = editSetpointPage.pSetpointLine[spId].note.ub.first;

        sp_high = setpoint_cbar / 100;

        strcpy(text, "\001" "Setpoint #0 X");
        text[11] += spId;
        text[13] = TXT_Setpoint_Edit;
        write_topline(text);


        y_line = ME_Y_LINE_BASE + (line * ME_Y_LINE_STEP);

        textPointer = 0;
        text[textPointer++] = 'S';
        text[textPointer++] = 'P';
        text[textPointer++] = '0' + spId;
        text[textPointer++] = ' ';
        text[textPointer++] = ' ';

        if(first == 0)
            strcpy(&text[textPointer++],"\177");

        textPointer += snprintf(&text[textPointer], 60,\
            "* "
            "       "
            "\016\016"
            " bar"
            "\017"
            "\034"
            "   "
            "\016\016"
            " "
            "\017"
            "           "
            "\016\016"
            "meter"
            "\017"
            "\035"
            "\n\r"
        );
        write_label_var(  20, 800, y_line, &FontT48, text);

        write_field_udigit(StMSP_ppo2_setting,	160, 800, y_line, &FontT48, "#.##            ###", (uint32_t)sp_high, (uint32_t)(setpoint_cbar - (100 * sp_high)), depthDeco, 0);
        setEvent(StMSP_ppo2_setting,	(uint32_t)OnAction_SP_Setpoint);
        startEdit();
    }
}

uint8_t OnAction_SP_Setpoint(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    int8_t digitContentNew;
    uint32_t new_integer_part, new_fractional_part, new_cbar, newDepth;

    if(action == ACTION_BUTTON_ENTER)
        return digitContent;

    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &new_integer_part, &new_fractional_part, &newDepth, 0);

        new_cbar = (new_integer_part * 100) + new_fractional_part;

        if(new_cbar < 50)
            new_cbar = 50;

        if(new_cbar > 160)
            new_cbar = 160;

        new_integer_part = new_cbar / 100;
        new_fractional_part = new_cbar - (new_integer_part * 100);

        editSetpointPage.pSetpointLine[editSetpointPage.spID].setpoint_cbar = new_cbar;

        if(newDepth > 255)
            newDepth = 255;

        editSetpointPage.pSetpointLine[editSetpointPage.spID].depth_meter = newDepth;

        return UPDATE_AND_EXIT_TO_MENU;
    }

    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if((blockNumber == 0) && (digitContentNew > '1'))
            digitContentNew = '0';
        if(digitContentNew > '9')
            digitContentNew = '0';
        return digitContentNew;
    }

    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - 1;
        if((blockNumber == 0) && (digitContentNew > '1'))
            digitContentNew = '1';
        if(digitContentNew < '0')
            digitContentNew = '9';
        return digitContentNew;
    }

    return EXIT_TO_MENU;
}

uint8_t OnAction_SP_Depth(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t digitContentNew;
    uint32_t newDepth;

    if(action == ACTION_BUTTON_ENTER)
    {
        return digitContent;
    }
    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &newDepth, 0, 0, 0);
        if(newDepth > 255)
            newDepth = 255;

        editSetpointPage.pSetpointLine[editSetpointPage.spID].depth_meter = newDepth;

        tMenuEdit_newInput(editId, newDepth, 0, 0, 0);
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

void openEdit_DiveSelectBetterSetpoint(void)
{
    uint8_t spId;
    SDiveState *pState;
    uint8_t depth;

    spId = actualBetterSetpointId();

    if(actual_menu_content == MENU_DIVE_REAL)
        pState = stateRealGetPointerWrite();
    else
        pState = stateSimGetPointerWrite();

    depth = pState->diveSettings.setpoint[spId].depth_meter;

    // BetterSetpoint warning only once -> clear active
    for(int i=0; i<=NUM_GASES; i++)
    {
        pState->diveSettings.setpoint[i].note.ub.first = 0;
        if(pState->diveSettings.setpoint[i].depth_meter <= depth)
            pState->diveSettings.setpoint[i].note.ub.active = 0;
    }

    // new setpoint
    pState->diveSettings.setpoint[spId].note.ub.first = 1;

    // change in lifeData
    setActualGas_DM(&pState->lifeData, pState->lifeData.actualGas.GasIdInSettings, pState->diveSettings.setpoint[spId].setpoint_cbar);
}

uint8_t OnAction_SP_DM_Sensor1	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SDiveState * pDiveState = 0;

    if(actual_menu_content == MENU_DIVE_REAL)
        pDiveState = stateRealGetPointerWrite();
    else
        pDiveState = stateSimGetPointerWrite();

    if(pDiveState->diveSettings.ppo2sensors_deactivated & 1)
    {
        pDiveState->diveSettings.ppo2sensors_deactivated &= 4+2;
        tMenuEdit_set_on_off(editId, 1);
    }
    else
    {
        pDiveState->diveSettings.ppo2sensors_deactivated |= 1;
        tMenuEdit_set_on_off(editId, 0);
    }

    return UNSPECIFIC_RETURN;
}

uint8_t OnAction_SP_DM_Sensor2	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SDiveState * pDiveState = 0;

    if(actual_menu_content == MENU_DIVE_REAL)
        pDiveState = stateRealGetPointerWrite();
    else
        pDiveState = stateSimGetPointerWrite();

    if(pDiveState->diveSettings.ppo2sensors_deactivated & 2)
    {
        pDiveState->diveSettings.ppo2sensors_deactivated &= 4+1;
        tMenuEdit_set_on_off(editId, 1);
    }
    else
    {
        pDiveState->diveSettings.ppo2sensors_deactivated |= 2;
        tMenuEdit_set_on_off(editId, 0);
    }

    return UNSPECIFIC_RETURN;
}

uint8_t OnAction_SP_DM_Sensor3	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SDiveState * pDiveState = 0;

    if(actual_menu_content == MENU_DIVE_REAL)
        pDiveState = stateRealGetPointerWrite();
    else
        pDiveState = stateSimGetPointerWrite();

    if(pDiveState->diveSettings.ppo2sensors_deactivated & 4)
    {
        pDiveState->diveSettings.ppo2sensors_deactivated &= 2+1;
        tMenuEdit_set_on_off(editId, 1);
    }
    else
    {
        pDiveState->diveSettings.ppo2sensors_deactivated |= 4;
        tMenuEdit_set_on_off(editId, 0);
    }
    return UNSPECIFIC_RETURN;
}

/* Private functions ---------------------------------------------------------*/

/*
uint8_t OnAction_SP_Active(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t active, first;

    first = editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.first;

    if(first)
        return UNSPECIFIC_RETURN;

    active = editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.active;

    if(active)
    {
        active = 0;
        editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.active = 0;
    }
    else
    {
        active = 1;
        editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.active = 1;
    }
    tMenuEdit_set_on_off(editId, active);

    return UPDATE_DIVESETTINGS;
}

uint8_t OnAction_SP_First(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t active, first, i;
    SDiveState * pStateReal = stateRealGetPointerWrite();

    first = editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.first;

    if(first)
        return UNSPECIFIC_RETURN;

    for(i=0;i<NUM_GASES;i++)
        editSetpointPage.pSetpointLine[i].note.ub.first = 0;

    editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.first = 1;

    active = editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.active;
    if(active == 0)
    {
        editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.active = 1;
    }

    tMenuEdit_set_on_off(editId, 1);

    return UPDATE_DIVESETTINGS;
}

uint8_t OnAction_SP_DM_Select(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SDiveState * pDiveState = 0;

    if(editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.active == 0)
        editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.active = 1;

    for(int i=0;i<NUM_GASES;i++)
        editSetpointPage.pSetpointLine[i].note.ub.first = 0;

    editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.first = 1;

    if(actual_menu_content == MENU_DIVE_REAL)
        pDiveState = stateRealGetPointerWrite();
    else
        pDiveState = stateSimGetPointerWrite();

    setActualGas_DM(&pDiveState->lifeData, pDiveState->lifeData.actualGas.GasIdInSettings, editSetpointPage.pSetpointLine[editSetpointPage.spID].setpoint_cbar);

    return EXIT_TO_HOME;
}
*/
