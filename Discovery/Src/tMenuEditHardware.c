///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tMenuEditHardware.c
/// \brief  BUTTONS
/// \author heinrichs weikamp gmbh
/// \date   15-Sept-2016
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
#include "tMenuEditHardware.h"

#include "externCPU2bootloader.h"
#include "gfx_fonts.h"
#include "ostc.h"
#include "tCCR.h"
#include "tMenuEdit.h"
#include "tHome.h"
#include "tInfo.h"
#include "tInfoLog.h"
#include "tComm.h"
#include "data_exchange_main.h"

extern void tM_build_pages(void);

/* Private function prototypes -----------------------------------------------*/
void openEdit_Bluetooth(void);
void openEdit_Compass(void);
void openEdit_O2Sensors(void);
void openEdit_Brightness(void);
//void openEdit_Luftintegration(void);
void openEdit_ButtonSens(void);
void openEdit_FlipDisplay(void);

/* Announced function prototypes -----------------------------------------------*/
uint8_t OnAction_Compass		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Bearing		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_BearingClear	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_InertiaLevel	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
//uint8_t OnAction_ExitHardw	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Sensor1		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Sensor2		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Sensor3		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_O2_Fallback	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_O2_Calibrate   (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_O2_Source		(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_Button			(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
uint8_t OnAction_ButtonBalance	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
// not required uint8_t OnAction_Bluetooth				(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);

/* Exported functions --------------------------------------------------------*/


#define O2_CALIB_FRACTION_AIR	(0.209F)
#define O2_CALIB_FRACTION_O2	(0.98F)

static uint8_t	O2_calib_gas = 21;

void openEdit_Hardware(uint8_t line)
{
    set_globalState_Menu_Line(line);
    resetMenuEdit(CLUT_MenuPageHardware);

    switch(line)
    {
    case 1:
    default:
        openEdit_Bluetooth();
    break;
    case 2:
        openEdit_Compass();
    break;
    case 3:
        openEdit_O2Sensors();
    break;
    case 4:
        openEdit_Brightness();
    break;
    case 5:
        openEdit_ButtonSens();
    break;
    case 6:
    	openEdit_FlipDisplay();
    break;
    }
}

/* Private functions ---------------------------------------------------------*/
void openEdit_Bluetooth(void)
{
/* does not work like this	resetEnterPressedToStateBeforeButtonAction(); */

    SSettings *pSettings = settingsGetPointer();

    if(pSettings->bluetoothActive == 0)
    {
        pSettings->bluetoothActive = 1;
        MX_Bluetooth_PowerOn();
        tComm_StartBlueModConfig();
    }
    else
    {
        pSettings->bluetoothActive = 0;
        MX_Bluetooth_PowerOff();
    }
    exitMenuEdit_to_Menu_with_Menu_Update_do_not_write_settings_for_this_only();
}

void openEdit_FlipDisplay(void)
{
/* does not work like this	resetEnterPressedToStateBeforeButtonAction(); */

    SSettings *pSettings = settingsGetPointer();

    if(pSettings->FlipDisplay == 0)
    {
        pSettings->FlipDisplay = 1;
    }
    else
    {
        pSettings->FlipDisplay = 0;
    }
    /* reinit all views */
    tHome_init();
    tI_init();
    tM_init();
    tMenuEdit_init();
    tInfoLog_init();
    tM_build_pages();
    GFX_build_logo_frame();
    GFX_build_hw_background_frame();

    exitEditWithUpdate();
    exitMenuEdit_to_Home();
}

void refresh_CompassEdit(void)
{
    uint16_t heading;
    char text[32];
    uint8_t textIndex = 0;

    text[0] = '\001';
    text[1] = TXT_2BYTE;
    text[2] = TXT2BYTE_Compass;
    text[3] = 0;
    write_topline(text);

    if(settingsGetPointer()->compassInertia)
    {
    	heading = (uint16_t)compass_getCompensated();
    }
    else
    {
    	heading = (uint16_t)stateUsed->lifeData.compass_heading;
    }
    snprintf(text,32,"\001%03i`",heading);
    write_label_var(   0, 800, ME_Y_LINE1, &FontT54, text);

    tMenuEdit_refresh_field(StMHARD2_Compass_SetCourse);
    tMenuEdit_refresh_field(StMHARD2_Compass_Calibrate);
    tMenuEdit_refresh_field(StMHARD2_Compass_ResetCourse);
    text[textIndex++] = TXT_2BYTE;
    text[textIndex++] = TXT2BYTE_CompassInertia;
    text[textIndex++] = ':';
    text[textIndex++] = ' ';
    text[textIndex++] = '0' + settingsGetPointer()->compassInertia;

    write_label_var(30, 800, ME_Y_LINE5,  &FontT48, text);

    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);
}


void openEdit_Compass(void)
{
    char text[10];
    uint8_t textIndex = 0;

    text[textIndex++] = '\001';
    text[textIndex++] = TXT_2BYTE;
    text[textIndex++] = TXT2BYTE_Compass;
    text[textIndex++] = 0;
    write_topline(text);

    text[0] = TXT_2BYTE;
    text[2] = 0;

    text[1] = TXT2BYTE_SetBearing;
    write_field_button(StMHARD2_Compass_SetCourse,	 30, 800, ME_Y_LINE2,  &FontT48, text);

    text[1] = TXT2BYTE_CompassCalib;
    write_field_button(StMHARD2_Compass_Calibrate,	 30, 800, ME_Y_LINE3,  &FontT48, text);

    text[1] = TXT2BYTE_ResetBearing;
    write_field_button(StMHARD2_Compass_ResetCourse, 30, 800, ME_Y_LINE4,  &FontT48, text);

    text[1] = TXT2BYTE_CompassInertia;
    textIndex = 2;
    text[textIndex++] = ':';
    text[textIndex++] = ' ';
    text[textIndex++] = '0' + settingsGetPointer()->compassInertia;

    write_field_button(StMHARD2_Compass_Inertia, 30, 800, ME_Y_LINE5,  &FontT48, text);

    setEvent(StMHARD2_Compass_SetCourse,		(uint32_t)OnAction_Bearing);
    setEvent(StMHARD2_Compass_Calibrate,		(uint32_t)OnAction_Compass);
    setEvent(StMHARD2_Compass_ResetCourse,	(uint32_t)OnAction_BearingClear);
    setEvent(StMHARD2_Compass_Inertia,	(uint32_t)OnAction_InertiaLevel);

    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);
}


uint8_t OnAction_Compass (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    calibrateCompass();
    return EXIT_TO_INFO_COMPASS;
}


uint8_t OnAction_Bearing	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    settingsGetPointer()->compassBearing = (int16_t)stateUsed->lifeData.compass_heading;
    if(settingsGetPointer()->compassBearing == 0)
        settingsGetPointer()->compassBearing = 360;
    return UPDATE_AND_EXIT_TO_MENU;
}


uint8_t OnAction_BearingClear	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    settingsGetPointer()->compassBearing = 0;
    return UPDATE_AND_EXIT_TO_MENU;
}


uint8_t OnAction_InertiaLevel	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
	uint8_t newLevel = 0;

	newLevel = settingsGetPointer()->compassInertia + 1;
	if(newLevel > MAX_COMPASS_COMP)
	{
		newLevel = 0;
	}
	settingsGetPointer()->compassInertia = newLevel;
    return UPDATE_DIVESETTINGS;
}

/*
uint8_t OnAction_ExitHardw (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    return EXIT_TO_MENU;
}
*/

void refresh_O2Sensors(void)
{
    char text[32];
    uint16_t y_line;

    const SDiveState *pStateReal = stateRealGetPointer();

    text[0] = '\001';
    text[1] = TXT_o2Sensors;
    text[2] = 0;
    write_topline(text);


    text[0] = TXT_2BYTE;
    text[1] = TXT2BYTE_Sensor;
    text[2] = ' ';
    text[3] = '1';
    text[4] = 0;
    write_label_var(  96, 340, ME_Y_LINE1, &FontT48, text);
    text[3] = '2';
    write_label_var(  96, 340, ME_Y_LINE2, &FontT48, text);
    text[3] = '3';
    write_label_var(  96, 340, ME_Y_LINE3, &FontT48, text);

    if(settingsGetPointer()->ppo2sensors_source == O2_SENSOR_SOURCE_OPTIC)
    {
		text[0] = TXT_2BYTE;
		text[1] = TXT2BYTE_HUDbattery;
		text[2] = 0;
		write_label_var(  30, 340, ME_Y_LINE4, &FontT48, text);


		snprintf(text, 20,"%01.3fV", get_HUD_battery_voltage_V());
		write_label_var(  400, 800, ME_Y_LINE4, &FontT48, text);
    }
    else
    {
    	text[0] = TXT_2BYTE;
    	text[1] = TXT2BYTE_O2Calib;
    	text[2] = 0;
    	write_label_var(  30, 340, ME_Y_LINE4, &FontT48, text);
    	snprintf(text, 20,"%d%%", O2_calib_gas);
    	write_label_var(  400, 800, ME_Y_LINE4, &FontT48, text);
    }

	for(int i=0;i<3;i++)
	{
		snprintf(text, 20,"%01.2f, %01.1fmV",  pStateReal->lifeData.ppO2Sensor_bar[i], pStateReal->lifeData.sensorVoltage_mV[i]);
		y_line = ME_Y_LINE1 + (i * ME_Y_LINE_STEP);
		write_label_var(  400, 800, y_line, &FontT48, text);
	}

    if(DataEX_external_ADC_Present())
    {
		text[0] = TXT_2BYTE;
		text[1] = TXT2BYTE_O2Interface;
		text[2] = 0;
		write_label_var(  30, 340, ME_Y_LINE6, &FontT48, text);
		text[0] = TXT_2BYTE;
		switch(settingsGetPointer()->ppo2sensors_source)
		{
			default:
			case O2_SENSOR_SOURCE_OPTIC: text[1] = TXT2BYTE_O2IFOptic;
				break;
			case O2_SENSOR_SOURCE_ANALOG: text[1] = TXT2BYTE_O2IFAnalog;
		}
		text[2] = 0;
		write_label_var(  400, 800, ME_Y_LINE6, &FontT48, text);
    }
    tMenuEdit_refresh_field(StMHARD3_O2_Sensor1);
    tMenuEdit_refresh_field(StMHARD3_O2_Sensor2);
    tMenuEdit_refresh_field(StMHARD3_O2_Sensor3);
    tMenuEdit_refresh_field(StMHARD3_O2_Fallback);

    if(get_globalState() == StMHARD3_O2_Calibrate)
    {
    	write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_O2Calib,TXT2BYTE_ButtonPlus);
    }
    else
    {
    	write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);
    }
}


void openEdit_O2Sensors(void)
{
    char text[2];
    uint8_t sensorActive[3];

    sensorActive[0] = 1;
    sensorActive[1] = 1;
    sensorActive[2] = 1;
    if(settingsGetPointer()->ppo2sensors_deactivated & 1)
        sensorActive[0] = 0;
    if(settingsGetPointer()->ppo2sensors_deactivated & 2)
        sensorActive[1] = 0;
    if(settingsGetPointer()->ppo2sensors_deactivated & 4)
        sensorActive[2] = 0;

    write_field_on_off(StMHARD3_O2_Sensor1,	 30, 95, ME_Y_LINE1,  &FontT48, "", sensorActive[0]);
    write_field_on_off(StMHARD3_O2_Sensor2,	 30, 95, ME_Y_LINE2,  &FontT48, "", sensorActive[1]);
    write_field_on_off(StMHARD3_O2_Sensor3,	 30, 95, ME_Y_LINE3,  &FontT48, "", sensorActive[2]);

    if(settingsGetPointer()->ppo2sensors_source == O2_SENSOR_SOURCE_ANALOG)
    {
        write_label_fix(   30, 800, ME_Y_LINE4, &FontT48, TXT2BYTE_O2Calib);
        write_label_var(  400, 800, ME_Y_LINE4, &FontT48, "\016\016 %\017");

        write_field_toggle(StMHARD3_O2_Calibrate,	400, 800, ME_Y_LINE4, &FontT48, "", 21, 100);
    }

    text[0] = TXT_Fallback;
    text[1] = 1;

    write_field_on_off(StMHARD3_O2_Fallback,	 30, 500, ME_Y_LINE5,  &FontT48, text, settingsGetPointer()->fallbackToFixedSetpoint);

    if(DataEX_external_ADC_Present())
    {
    	write_field_button(StMHARD3_O2_Source,	 30, 800, ME_Y_LINE6,  &FontT48, "");
    }

    setEvent(StMHARD3_O2_Sensor1, (uint32_t)OnAction_Sensor1);
    setEvent(StMHARD3_O2_Sensor2, (uint32_t)OnAction_Sensor2);
    setEvent(StMHARD3_O2_Sensor3, (uint32_t)OnAction_Sensor3);
    if(settingsGetPointer()->ppo2sensors_source == O2_SENSOR_SOURCE_ANALOG)
    {
    	setEvent(StMHARD3_O2_Calibrate, (uint32_t)OnAction_O2_Calibrate);
    }
    setEvent(StMHARD3_O2_Fallback, (uint32_t)OnAction_O2_Fallback);
    if(DataEX_external_ADC_Present())
    {
    	setEvent(StMHARD3_O2_Source, (uint32_t)OnAction_O2_Source);
    }

    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);
}


uint8_t OnAction_Sensor1(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    if(settingsGetPointer()->ppo2sensors_deactivated & 1)
    {
        settingsGetPointer()->ppo2sensors_deactivated &= 4+2;
        tMenuEdit_set_on_off(editId, 1);
    }
    else
    {
        settingsGetPointer()->ppo2sensors_deactivated |= 1;
        tMenuEdit_set_on_off(editId, 0);
    }

    return UPDATE_DIVESETTINGS;
}


uint8_t OnAction_Sensor2(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    if(settingsGetPointer()->ppo2sensors_deactivated & 2)
    {
        settingsGetPointer()->ppo2sensors_deactivated &= 4+1;
        tMenuEdit_set_on_off(editId, 1);
    }
    else
    {
        settingsGetPointer()->ppo2sensors_deactivated |= 2;
        tMenuEdit_set_on_off(editId, 0);
    }

    return UPDATE_DIVESETTINGS;
}


uint8_t OnAction_Sensor3(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    if(settingsGetPointer()->ppo2sensors_deactivated & 4)
    {
        settingsGetPointer()->ppo2sensors_deactivated &= 2+1;
        tMenuEdit_set_on_off(editId, 1);
    }
    else
    {
        settingsGetPointer()->ppo2sensors_deactivated |= 4;
        tMenuEdit_set_on_off(editId, 0);
    }

    return UPDATE_DIVESETTINGS;
}


uint8_t OnAction_O2_Fallback	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t fallback = settingsGetPointer()->fallbackToFixedSetpoint;

    if(fallback)
        fallback = 0;
    else
        fallback = 1;

    settingsGetPointer()->fallbackToFixedSetpoint = fallback;
    tMenuEdit_set_on_off(editId, fallback);
    return UPDATE_DIVESETTINGS;
}
uint8_t OnAction_O2_Calibrate (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
	uint8_t loop;
	const SDiveState *pStateReal = stateRealGetPointer();
	SSettings* pSettings = settingsGetPointer();
	uint8_t retVal = UNSPECIFIC_RETURN;
	float compensatedRef;

    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
    		if(O2_calib_gas == 21)
    		{
    			compensatedRef = O2_CALIB_FRACTION_AIR * pStateReal->lifeData.pressure_ambient_bar / 1.0;
    		}
    		else
    		{
    			compensatedRef = O2_CALIB_FRACTION_O2 * pStateReal->lifeData.pressure_ambient_bar / 1.0;
    		}
			for(loop=0;loop<3;loop++)
			{
				if((pSettings->ppo2sensors_deactivated & (0x1 << loop)) == 0)
				{
					if(pStateReal->lifeData.sensorVoltage_mV[loop] > 0.0001)		/* sensor connected ?*/
					{
						pSettings->ppo2sensors_calibCoeff[loop] =  compensatedRef / pStateReal->lifeData.sensorVoltage_mV[loop];
					}
					else
					{
						pSettings->ppo2sensors_calibCoeff[loop] = 0.0;
						settingsGetPointer()->ppo2sensors_deactivated |= 0x1 << loop;
					}
				}
			}
			tMenuEdit_newInput(editId, O2_calib_gas, 0, 0, 0);
			retVal = UPDATE_DIVESETTINGS;
    }
    if(action == ACTION_BUTTON_NEXT)
    {
		if(O2_calib_gas == 21)
		{
			O2_calib_gas = 100;
		}
		else
		{
			O2_calib_gas = 21;
		}
   	}
   	retVal = O2_calib_gas;

    if(action == ACTION_BUTTON_BACK)
    {
    	exitMenuEditField();
    }

	return retVal;
}
uint8_t OnAction_O2_Source	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t source = settingsGetPointer()->ppo2sensors_source;

    if(source == O2_SENSOR_SOURCE_OPTIC)
    {
    	source = O2_SENSOR_SOURCE_ANALOG;
    }
    else
    {
    	source = O2_SENSOR_SOURCE_OPTIC;
    }

    settingsGetPointer()->ppo2sensors_source = source;

    resetMenuEdit(CLUT_MenuPageHardware); 	/* rebuild menu structure (Hide HUD <=> Show Calibrate) */
    openEdit_O2Sensors();
    return UPDATE_DIVESETTINGS;
}

void openEdit_Brightness(void)
{
    uint8_t actualBrightness;
    SSettings *pSettings = settingsGetPointer();

    actualBrightness = pSettings->brightness;
    actualBrightness++;
    if(actualBrightness > 4)
        actualBrightness = 0;
    pSettings->brightness = actualBrightness;
    exitEditWithUpdate();
}


void buttonBalanceText_helper(uint8_t idOfButton, char *textOutput)
{
    uint8_t txtcount = 0;

    if(idOfButton < 3)
    {
        textOutput[txtcount++] = '@' + settingsGetPointer()->buttonBalance[idOfButton];
        textOutput[txtcount++] = ' ';
        textOutput[txtcount++] = ' ';
        textOutput[txtcount++] = '(';

        switch(settingsGetPointer()->buttonBalance[idOfButton])
    {
        case 1:
            textOutput[txtcount++] = '-';
            textOutput[txtcount++] = '2';
            textOutput[txtcount++] = '0';
            break;
        case 2:
            textOutput[txtcount++] = '-';
            textOutput[txtcount++] = '1';
            textOutput[txtcount++] = '0';
            break;
        case 3:
        default:
            textOutput[txtcount++] = '0';
            break;
        case 4:
            textOutput[txtcount++] = '+';
            textOutput[txtcount++] = '1';
            textOutput[txtcount++] = '0';
            break;
        case 5:
            textOutput[txtcount++] = '+';
            textOutput[txtcount++] = '2';
            textOutput[txtcount++] = '0';
            break;
        }
        textOutput[txtcount++] = ')';
    }
    textOutput[txtcount++] = 0;
}

/**#
  ******************************************************************************
  * @brief   BUTTONS
  * @author  heinrichs weikamp gmbh
  * @version V 01
  * @date    15-Sept-2016
  ******************************************************************************
    *	Button 0 is right, Button 1 is middle, Button 2 is left !!!!
    *   2    1    0    (base value 3)
    * Button 3 is used to store the base value, all others are balanced around this one!
    *
  */

void openEdit_ButtonSens(void)
{
    char text[32];
    uint8_t sens;
    const uint32_t eventListButtonBalance[3] = {StMHARD5_ButtonBalance1,StMHARD5_ButtonBalance2,StMHARD5_ButtonBalance3};

    sens = (uint8_t)settingsGetPointer()->ButtonResponsiveness[3];
    write_field_3digit(StMHARD5_Button1, 360, 780, ME_Y_LINE1,  &FontT48, "###", sens, 0, 0, 0);

    for(int i=2;i>=0;i--)
    {
        buttonBalanceText_helper(i,text);
        write_field_button(eventListButtonBalance[i],360,500,ME_Y_LINE4-(i*ME_Y_LINE_STEP),&FontT48,text);
    }


    setEvent(StMHARD5_Button1, (uint32_t)OnAction_Button);

    for(int i=2;i>=0;i--)
    {
        setEvent(eventListButtonBalance[i], (uint32_t)OnAction_ButtonBalance);
    }

    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);
}


void refresh_ButtonValuesFromPIC(void)
{
    uint8_t sens[3];
    char text[64];

    text[0] = '\001';
    text[1] = TXT_2BYTE;
    text[2] = TXT2BYTE_ButtonSensitivity;
    text[3] = 0;
    write_topline(text);

    write_buttonTextline(TXT2BYTE_ButtonBack,TXT2BYTE_ButtonEnter,TXT2BYTE_ButtonNext);

    text[0] = '\020'; // '\031';
    text[1] = TXT_2BYTE;
    text[2] = TXT2BYTE_LowerIsLess;
    text[3] = 0;
    write_label_var(  20, 780, ME_Y_LINE5, &FontT42, text);

    for(int i=0;i<3;i++)
    {
        text[0] = TXT_2BYTE;
        text[1] = TXT2BYTE_ButtonLeft+i;
        text[2] = 0;
        write_label_var(  20, 300, ME_Y_LINE2+(i*ME_Y_LINE_STEP), &FontT48, text);
    }

    for(int i=0;i<3;i++)
    {
        sens[i] = settingsHelperButtonSens_translate_hwOS_values_to_percentage(stateRealGetPointer()->lifeData.buttonPICdata[i]);
    }
    snprintf(text,64,"(%03u  %03u  %03u)",sens[2],sens[1],sens[0]);
    write_label_var(  20, 340, ME_Y_LINE6, &FontT42, text);

    tMenuEdit_refresh_field(StMHARD5_Button1);
    tMenuEdit_refresh_field(StMHARD5_ButtonBalance1);
    tMenuEdit_refresh_field(StMHARD5_ButtonBalance2);
    tMenuEdit_refresh_field(StMHARD5_ButtonBalance3);
}


uint8_t OnAction_Button(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    uint8_t digitContentNew, remainder;
    uint32_t newSensitivityGlobal;

    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent - '0';
        if(digitContentNew >= MAX_BUTTONRESPONSIVENESS_GUI)
        {
            digitContentNew = MIN_BUTTONRESPONSIVENESS_GUI;
        }
        else
        {
            remainder = digitContentNew%5;
            digitContentNew += 5 - remainder;
            if(digitContentNew >= MAX_BUTTONRESPONSIVENESS_GUI)
                digitContentNew = MAX_BUTTONRESPONSIVENESS_GUI;
        }
        return '0' + digitContentNew;
    }

    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - '0';
        if(digitContentNew <= MIN_BUTTONRESPONSIVENESS_GUI)
            digitContentNew = MAX_BUTTONRESPONSIVENESS_GUI;
        else
        {
            remainder = digitContentNew%5;
            if(remainder)
                digitContentNew -= remainder;
            else
                digitContentNew -= 5;
        }
        return '0' + digitContentNew;
    }

    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &newSensitivityGlobal, 0, 0, 0);
        settingsHelperButtonSens_keepPercentageValues(newSensitivityGlobal, settingsGetPointer()->ButtonResponsiveness);
        setButtonResponsiveness(settingsGetPointer()->ButtonResponsiveness);
        return UNSPECIFIC_RETURN;
    }
    return digitContent;
}


uint8_t OnAction_ButtonBalance(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    int8_t idBalance = -1;
    uint8_t *ptrSetting;
    char text[32];

    const uint32_t eventListButtonBalance[3] = {StMHARD5_ButtonBalance1,StMHARD5_ButtonBalance2,StMHARD5_ButtonBalance3};

    idBalance = -1;
    for(int i=0;i<3;i++)
    {
        if(editId == eventListButtonBalance[i])
        {
            idBalance = i;
            break;
        }
    }

    if((idBalance >= 0) && (idBalance < 3))
    {
        ptrSetting = &settingsGetPointer()->buttonBalance[idBalance];

        *ptrSetting += 1;

        if(*ptrSetting > 5)
            *ptrSetting = 2;

        buttonBalanceText_helper(idBalance,text);
        tMenuEdit_newButtonText(eventListButtonBalance[idBalance],text);
    }

    return UNSPECIFIC_RETURN;
}
