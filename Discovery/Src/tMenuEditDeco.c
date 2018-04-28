///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tMenuEditDeco.c
/// \brief  Main Template file for Menu Edit Deco Parameters
/// \author heinrichs weikamp gmbh
/// \date   31-July-2014
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
#include "tMenuEditDeco.h"

#include "gfx_fonts.h"
#include "tMenuEdit.h"

/* Private variables ---------------------------------------------------------*/
uint8_t lineSelected = 0;

/* Private function prototypes -----------------------------------------------*/

void openEdit_DiveMode(void);
void openEdit_CCRModeSensorOrFixedSP(void);
void openEdit_ppO2max(void);
void openEdit_SafetyStop(void);
void openEdit_FutureTTS(void);
void openEdit_Salinity(void);

/* Announced function prototypes -----------------------------------------------*/

uint8_t OnAction_OC			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_CC			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Apnea		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Gauge		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_FutureTTS	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_ppO2Max	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_SafetyStop (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Salinity	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
/* Exported functions --------------------------------------------------------*/

void openEdit_Deco(uint8_t line)
{
    set_globalState_Menu_Line(line);
    resetMenuEdit(CLUT_MenuPageDeco);

    SSettings *data = settingsGetPointer();

    lineSelected = line;

    if((data->dive_mode != DIVEMODE_CCR )&&  (line > 1))
        line += 1;

    switch(line)
    {
    case 1:
    default:
        openEdit_DiveMode();
        break;
    case 2:
        openEdit_CCRModeSensorOrFixedSP();
        break;
    case 3:
        openEdit_ppO2max();
        break;
    case 4:
        openEdit_SafetyStop();
        break;
    case 5:
        openEdit_FutureTTS();
        break;
    case 6:
        openEdit_Salinity();
        break;
    }
}

/* Private functions ---------------------------------------------------------*/


void openEdit_DiveMode(void)
{
#define APNEAANDGAUGE

    char text[32];
    uint8_t actualDiveMode, active;
    SSettings *pSettings = settingsGetPointer();
    actualDiveMode = pSettings->dive_mode;

    text[0] = '\001';
    text[1] = TXT_DiveMode;
    text[2] = 0;
    write_topline(text);

    text[1] = 0;


    text[0] = TXT_OpenCircuit;
    if(actualDiveMode == DIVEMODE_OC)
        active = 1;
    else
        active = 0;
    write_field_on_off(StMDECO1_OC,			 30, 500, ME_Y_LINE1,  &FontT48, text, active);

    text[0] = TXT_ClosedCircuit;
    if(actualDiveMode == DIVEMODE_CCR)
        active = 1;
    else
        active = 0;
    write_field_on_off(StMDECO1_CC,			 30, 500, ME_Y_LINE2,  &FontT48, text, active);

    #ifdef APNEAANDGAUGE
    text[0] = TXT_Apnoe;
    if(actualDiveMode == DIVEMODE_Apnea)
        active = 1;
    else
        active = 0;
    write_field_on_off(StMDECO1_Apnea,	 30, 500, ME_Y_LINE3,  &FontT48, text, active);

    text[0] = TXT_Gauge;
    if(actualDiveMode == DIVEMODE_Gauge)
        active = 1;
    else
        active = 0;
    write_field_on_off(StMDECO1_Gauge,	 30, 500, ME_Y_LINE4,  &FontT48, text, active);
    #endif

    setEvent(StMDECO1_OC, 			(uint32_t)OnAction_OC);
    setEvent(StMDECO1_CC, 			(uint32_t)OnAction_CC);
    #ifdef APNEAANDGAUGE
    setEvent(StMDECO1_Apnea, 		(uint32_t)OnAction_Apnea);
    setEvent(StMDECO1_Gauge, 		(uint32_t)OnAction_Gauge);
    #endif

    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);
}

uint8_t OnAction_OC						(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings = settingsGetPointer();
    if(pSettings->dive_mode == DIVEMODE_OC)
        return EXIT_TO_MENU;
    pSettings->dive_mode = DIVEMODE_OC;
    setActualGasFirst(&stateRealGetPointerWrite()->lifeData);
    tMenuEdit_set_on_off(editId, 1);
    tMenuEdit_set_on_off(StMDECO1_CC, 0);
    tMenuEdit_set_on_off(StMDECO1_Apnea, 0);
    tMenuEdit_set_on_off(StMDECO1_Gauge, 0);
    return UPDATE_DIVESETTINGS;
}


uint8_t OnAction_CC						(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings = settingsGetPointer();
    if(pSettings->dive_mode == DIVEMODE_CCR)
        return EXIT_TO_MENU;
    pSettings->dive_mode = DIVEMODE_CCR;
    setActualGasFirst(&stateRealGetPointerWrite()->lifeData);
    tMenuEdit_set_on_off(editId, 1);
    tMenuEdit_set_on_off(StMDECO1_OC, 0);
    tMenuEdit_set_on_off(StMDECO1_Apnea, 0);
    tMenuEdit_set_on_off(StMDECO1_Gauge, 0);
    return UPDATE_DIVESETTINGS;
}


uint8_t OnAction_Apnea				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings = settingsGetPointer();
    if(pSettings->dive_mode == DIVEMODE_Apnea)
        return EXIT_TO_MENU;
    pSettings->dive_mode = DIVEMODE_Apnea;
    setActualGasFirst(&stateRealGetPointerWrite()->lifeData);
    tMenuEdit_set_on_off(editId, 1);
    tMenuEdit_set_on_off(StMDECO1_CC, 0);
    tMenuEdit_set_on_off(StMDECO1_OC, 0);
    tMenuEdit_set_on_off(StMDECO1_Gauge, 0);
    return UPDATE_DIVESETTINGS;
}


uint8_t OnAction_Gauge				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings = settingsGetPointer();
    if(pSettings->dive_mode == DIVEMODE_Gauge)
        return EXIT_TO_MENU;
    pSettings->dive_mode = DIVEMODE_Gauge;
    setActualGasFirst(&stateRealGetPointerWrite()->lifeData);
    tMenuEdit_set_on_off(editId, 1);
    tMenuEdit_set_on_off(StMDECO1_CC, 0);
    tMenuEdit_set_on_off(StMDECO1_Apnea, 0);
    tMenuEdit_set_on_off(StMDECO1_OC, 0);
    return UPDATE_DIVESETTINGS;
}


void openEdit_CCRModeSensorOrFixedSP(void)
{
    SSettings *pSettings = settingsGetPointer();

    if(pSettings->CCR_Mode == CCRMODE_Sensors)
        pSettings->CCR_Mode = CCRMODE_FixedSetpoint;
    else
        pSettings->CCR_Mode = CCRMODE_Sensors;

    exitEditWithUpdate();
}


void openEdit_SafetyStop(void)
{
    uint32_t safetystopDuration, safetystopDepth;
    char text[64];
    uint16_t y_line;

    safetystopDuration = settingsGetPointer()->safetystopDuration;
    safetystopDepth = settingsGetPointer()->safetystopDepth;

    y_line = ME_Y_LINE_BASE + (lineSelected * ME_Y_LINE_STEP);

    text[0] = '\001';
    text[1] = TXT_SafetyStop;
    text[2] = 0;
    write_topline(text);

    write_label_fix(   20, 800, y_line, &FontT48, TXT_SafetyStop);

    strcpy(text,"\016\016");
    text[2] = TXT_Minutes;
    if(settingsGetPointer()->nonMetricalSystem)
    {
        strcpy(&text[3],
            "\017"
            "  @       "
            "\016\016"
            " ft"
            "\017"
        );
    }
    else
    {
        strcpy(&text[3],
            "\017"
            "  @       "
            "\016\016"
            " m"
            "\017"
        );
    }
    write_label_var(  410, 800, y_line, &FontT48, text);

    if(settingsGetPointer()->nonMetricalSystem)
    {
        write_field_2digit(StMDECO4_SafetyStop,	 		350, 800, y_line, &FontT48, "##               ##", safetystopDuration, unit_depth_integer(safetystopDepth), 0, 0);
    }
    else
    {
        write_field_udigit(StMDECO4_SafetyStop,	 		370, 800, y_line, &FontT48, "#                 #", safetystopDuration, safetystopDepth, 0, 0);
    }

    write_buttonTextline(TXT2BYTE_ButtonMinus,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonPlus);

    setEvent(StMDECO4_SafetyStop, 		(uint32_t)OnAction_SafetyStop);
    startEdit();
}


uint8_t OnAction_SafetyStop		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t digitContentNew;
    uint32_t newSafetystopDuration, newSafetystopDepth;

    if(action == ACTION_BUTTON_ENTER)
    {
        return digitContent;
    }
    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &newSafetystopDuration, &newSafetystopDepth, 0, 0);

        if(settingsGetPointer()->nonMetricalSystem != 0) // new hw 170718
        {
            newSafetystopDepth += 2; // fï¿½r rundung
            newSafetystopDepth = (newSafetystopDepth * 3) / 10;
        }


        settingsGetPointer()->safetystopDuration = newSafetystopDuration;
        settingsGetPointer()->safetystopDepth = newSafetystopDepth;

        tMenuEdit_newInput(editId, newSafetystopDuration, newSafetystopDepth, 0, 0);
        return UPDATE_AND_EXIT_TO_MENU;
    }
    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if(blockNumber == 0)
        {
            if(digitContentNew > '5')
                digitContentNew = '0';
        }
        else
        {
            if(settingsGetPointer()->nonMetricalSystem == 0)
            {
                if(digitContentNew > '6')
                    digitContentNew = '3';
            }
            else
            {
                if(digitContent < 13 + '0')
                    digitContentNew = 13 + '0';
                else if(digitContent < 16 + '0')
                    digitContentNew = 16 + '0';
                else if(digitContent < 20 + '0')
                    digitContentNew = 20 + '0';
                else
                    digitContentNew = 10 + '0';
            }
        }
        return digitContentNew;
    }
    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - 1;
        if(blockNumber == 0)
        {
            if(digitContentNew < '0')
                digitContentNew = '5';
        }
        else
        {
            if(settingsGetPointer()->nonMetricalSystem == 0)
            {
            if(digitContentNew < '3')
                digitContentNew = '6';
            }
            else
            {
                if(digitContent >= 20 + '0')
                    digitContentNew = 16 + '0';
                else if(digitContent >= 16 + '0')
                    digitContentNew = 13 + '0';
                else if(digitContent >= 13 + '0')
                    digitContentNew = 10 + '0';
                else
                    digitContentNew = 20 + '0';
            }
        }
        return digitContentNew;
    }
    return EXIT_TO_MENU;
}


void openEdit_Salinity(void)
{
    char text[32];
    uint16_t y_line;

    text[0] = '\001';
    text[1] = TXT_Salinity;
    text[2] = 0;
    write_topline(text);

    y_line = ME_Y_LINE_BASE + (lineSelected * ME_Y_LINE_STEP);

    write_label_fix(   30, 800, y_line, &FontT48, TXT_Salinity);
    write_label_var(  400, 800, y_line, &FontT48, "\016\016 %\017");

    write_field_udigit(StMDECO6_SALINITY,	370, 800, y_line, &FontT48, "#", (uint32_t)settingsGetPointer()->salinity, 0, 0, 0);

    write_buttonTextline(TXT2BYTE_ButtonMinus,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonPlus);

    setEvent(StMDECO6_SALINITY, 	(uint32_t)OnAction_Salinity);
    startEdit();
}


uint8_t OnAction_Salinity(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings;
    uint8_t digitContentNew;
    uint32_t salinity;

    if(action == ACTION_BUTTON_ENTER)
    {
        return digitContent;
    }
    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &salinity, 0, 0, 0);

        if(salinity >= 4)
            salinity = 4;

        pSettings = settingsGetPointer();

        pSettings->salinity = salinity;

        tMenuEdit_newInput(editId, salinity, 0, 0, 0);
        return UPDATE_AND_EXIT_TO_MENU;
    }
    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if(digitContentNew > '4')
            digitContentNew = '0';
        return digitContentNew;
    }
    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - 1;
        if(digitContentNew < '0')
            digitContentNew = '4';
        return digitContentNew;
    }

    return EXIT_TO_MENU;
}


void openEdit_ppO2max(void)
{
    uint8_t maxL_std, maxL_deco;
    uint16_t y_line;
    char text[32];
    SSettings *pSettings = settingsGetPointer();

    maxL_std = pSettings->ppO2_max_std - 100;
    maxL_deco = pSettings->ppO2_max_deco - 100;

    y_line = ME_Y_LINE_BASE + (lineSelected * ME_Y_LINE_STEP);

    text[0] = '\001';
    text[1] = TXT_ppO2Name;
    text[2] = 0;
    write_topline(text);

    strcpy(text,"ppO2\016\016max\017");
    write_label_var(   20, 800, y_line, &FontT48, text);
    strcpy(text,
        "\016\016"
        " bar "
        "  deco "
        "\017"
        "      "
        "\016\016"
        " bar"
        "\017"
    );
    write_label_var(  460, 800, y_line, &FontT48, text);

//	write_field_udigit(StMDECO4_PPO2Max,		410, 800, y_line, &FontT48, "##              ##", (uint32_t)maxL_std, (uint32_t)maxL_deco, 0, 0);
    write_field_udigit(StMDECO3_PPO2Max,		370, 800, y_line, &FontT48, "1.##           1.##", (uint32_t)maxL_std, (uint32_t)maxL_deco, 0, 0);

    write_buttonTextline(TXT2BYTE_ButtonMinus,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonPlus);

    setEvent(StMDECO3_PPO2Max,			(uint32_t)OnAction_ppO2Max);
    startEdit();
}


uint8_t OnAction_ppO2Max(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings;
    uint8_t digitContentNew;
    uint32_t newPPO2LStd, newPPO2LDeco;

    if(action == ACTION_BUTTON_ENTER)
    {
        return digitContent;
    }
    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &newPPO2LStd, &newPPO2LDeco, 0, 0);

        if(newPPO2LStd > 90)
            newPPO2LStd = 90;

        if(newPPO2LDeco > 90)
            newPPO2LDeco = 90;

        pSettings = settingsGetPointer();
        pSettings->ppO2_max_std = 100 + newPPO2LStd;
        pSettings->ppO2_max_deco = 100 + newPPO2LDeco;

        tMenuEdit_newInput(editId, newPPO2LStd, newPPO2LDeco, 0, 0);
        return UPDATE_AND_EXIT_TO_MENU;
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
    return EXIT_TO_MENU;
}


void openEdit_FutureTTS(void)
{
    uint8_t futureTTS;
    uint16_t y_line;

    char text[32];
    SSettings *pSettings = settingsGetPointer();
    futureTTS = pSettings->future_TTS;

    y_line = ME_Y_LINE_BASE + (lineSelected * ME_Y_LINE_STEP);

    text[0] = '\001';
    text[1] = TXT_FutureTTS;
    text[2] = 0;
    write_topline(text);

    strcpy(text,"\016\016");
    text[2] = TXT_Minutes;
    text[3] = 0;
    write_label_fix(   20, 800, y_line, &FontT48, TXT_FutureTTS);
    write_label_var(  435, 800, y_line, &FontT48, text);
    write_field_2digit(StMDECO5_FUTURE,	 		370, 500, y_line, &FontT48, "##", (uint32_t)futureTTS, 0, 0, 0);

    write_buttonTextline(TXT2BYTE_ButtonMinus,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonPlus);

    setEvent(StMDECO5_FUTURE, 		(uint32_t)OnAction_FutureTTS);
    startEdit();
}


uint8_t OnAction_FutureTTS(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    SSettings *pSettings;
    int8_t digitContentNew;
    uint32_t newFutureTTS;

    if(action == ACTION_BUTTON_ENTER)
    {
        return digitContent;
    }
    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &newFutureTTS, 0, 0, 0);

        if(newFutureTTS > 15)
            newFutureTTS = 15;

        pSettings = settingsGetPointer();
        pSettings->future_TTS = newFutureTTS;

        tMenuEdit_newInput(editId, newFutureTTS, 0, 0, 0);
        return UPDATE_AND_EXIT_TO_MENU;
    }
    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if(digitContentNew > '0'+ 15)
            digitContentNew = '0';
        return digitContentNew;
    }
    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - 1;
        if(digitContentNew < '0')
            digitContentNew = '0' + 15;
        return digitContentNew;
    }
/*
    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if((digitNumber == 0) && (digitContentNew > '1'))
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
            digitContentNew = '1';
        else
        if(digitContentNew < '0')
            digitContentNew = '9';
        return digitContentNew;
    }
*/
    return EXIT_TO_MENU;
}


