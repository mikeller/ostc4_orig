/**
  ******************************************************************************
	* @copyright	heinrichs weikamp
  * @file   		settings_bootloader.c
  * @author 		heinrichs/weikamp, Christian Weikamp
  * @date				21-March-2016
  * @version		V1.0.1
  * @since			24-Oct-2016
  * @brief			mini version for firmwareDataGetPointer() etc.
	*							1.0.1 getLicence() included
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "settings.h"


/* always at 0x8080000, do not move -> bootloader access */
const SFirmwareData* firmwareDataGetPointer(void)
{
	return (const SFirmwareData*)(0x08040000 + 0x00010000);
}	


void getActualRTEandFONTversion(uint8_t *RTEhigh, uint8_t *RTElow, uint8_t *FONThigh, uint8_t *FONTlow)
{
	if(RTEhigh && RTElow)
	{
		*RTEhigh = 0;
		*RTElow = 0;
	}
	if(FONThigh && FONTlow)
	{
		*FONThigh = *(uint8_t *)0x08132000;
		*FONTlow = *(uint8_t *)0x08132001;
	}
}


uint8_t getLicence(void)
{
//return 0xFF;
//return LICENCEBONEX;
	return hardwareDataGetPointer()->primaryLicence;
}
