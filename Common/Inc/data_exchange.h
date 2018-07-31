///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Common/Inc/data_exchange.h
/// \brief	Data exchange between RTE and Discovery processors.
/// \author Heinrichs Weikamp
/// \date   2018
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

#ifndef DATA_EXCHANGE_H
#define DATA_EXCHANGE_H

#include "data_central.h"
#include "settings.h"
#include "stm32f4xx_hal.h"

enum MODE
{
	MODE_SURFACE	= 0,
	MODE_DIVE 		= 1,
	MODE_CALIB 		= 2,
	MODE_SLEEP 		= 3,
	MODE_SHUTDOWN = 4,
	MODE_ENDDIVE	= 5,
	MODE_BOOT			= 6,
	MODE_CHARGESTART = 7,
	MODE_TEST			= 8,
	MODE_POWERUP 	= 9,
};

enum ACCIDENT_BITS
{
	ACCIDENT_DECOSTOP = 0x01,
	ACCIDENT_CNS			= 0x02,
	ACCIDENT_CNSLVL2	= 0x02 + 0x04,
	ACCIDENT_SPARE2		= 0x08,
	ACCIDENT_SPARE3		= 0x10,
	ACCIDENT_SPARE4		= 0x20,
	ACCIDENT_SPARE5		= 0x40,
	ACCIDENT_SPARE6		= 0x80
};

typedef struct{
uint8_t button:1;
uint8_t date:1;
uint8_t time:1;
uint8_t clearDeco:1;
uint8_t compass:1;
uint8_t devicedata:1;
uint8_t batterygauge:1;
uint8_t accident:1;
} confirmbit8_t;

#define CRBUTTON 			(0x01)
#define CRDATE 				(0x02)
#define CRTIME 				(0x04)
#define CRCLEARDECO		(0x08)
#define CRCOMPASS 		(0x10)
#define CRDEVICEDATA 	(0x20)

typedef union{
confirmbit8_t ub;
uint8_t uw;
} confirmbit8_Type;

typedef struct
{
		uint8_t checkCode[4];

} 	SDataExchangeHeader;

typedef struct
{
		uint8_t checkCode[4];

} 	SDataExchangeFooter;

typedef struct
{
	SDataExchangeHeader header;
	SLifeData lifeData;
} SDataExchangeMasterToSlave;

typedef struct
{
		//pressure
		float temperature;
		float pressure_mbar;
		float surface_mbar;
		float ascent_rate_meter_per_min;
		//toxic
		float otu;
		float cns;
		uint16_t desaturation_time_minutes;
		uint16_t no_fly_time_minutes;
		//tisssue
		float tissue_nitrogen_bar[16];
		float tissue_helium_bar[16];
		//maxcrushingpressure
		float  max_crushing_pressure_he[16];
		float  max_crushing_pressure_n2[16];
		float  adjusted_critical_radius_he[16];
		float  adjusted_critical_radius_n2[16];
		// Compass
		float compass_heading;
		float compass_roll;
		float compass_pitch;
		int16_t compass_DX_f;
		int16_t compass_DY_f;
		int16_t compass_DZ_f;
		//time
		uint16_t counterSecondsShallowDepth;
		uint32_t localtime_rtc_tr;
		uint32_t localtime_rtc_dr;
		uint32_t divetime_seconds;
		uint32_t surfacetime_seconds;
		uint32_t dive_time_seconds_without_surface_time;
		//battery /* take care of uint8_t count to be in multiplies of 4 */
		float battery_voltage;
		float battery_charge;
		//ambient light
		uint16_t ambient_light_level;
		// wireless data
		SDataWireless wireless_data[4];
		// PIC data
		uint8_t button_setting[4]; /* see dependency to SLiveData->buttonPICdata */
		uint8_t SPARE1;
		//debug
		uint32_t pressure_uTick;
		uint32_t compass_uTick;

} 	SExchangeData;

typedef struct
{
		uint8_t VPMconservatism;
		SGas actualGas;

		int8_t offsetPressureSensor_mbar;
		int8_t offsetTemperatureSensor_centiDegree;

		uint8_t SPARE1;
		uint8_t SPARE2;

		float UNUSED1[16-1];//VPM_adjusted_critical_radius_he[16];
		float UNUSED2[16];//VPM_adjusted_critical_radius_n2[16];
		float UNUSED3[16];//VPM_adjusted_crushing_pressure_he[16];
		float UNUSED4[16];//VPM_adjusted_crushing_pressure_n2[16];
		float UNUSED5[16];//VPM_initial_allowable_gradient_he[16];
		float UNUSED6[16];//VPM_initial_allowable_gradient_n2[16];
		float UNUSED7[16];//VPM_max_actual_gradient[16];

		RTC_TimeTypeDef newTime;
		RTC_DateTypeDef newDate;

		float ambient_pressure_mbar_ceiling;
		float descend_rate_bar_per_minute;
		float ascend_rate_bar_per_minute;

		uint16_t timeoutDiveReachedZeroDepth;
		uint16_t divetimeToCreateLogbook;

		uint8_t buttonResponsiveness[4];

		SDevice DeviceData;

		float newBatteryGaugePercentageFloat;

} 	SReceiveData;


typedef struct
{
	SDataExchangeHeader header;

	uint8_t mode;
	uint8_t power_on_reset;
	uint8_t RTE_VERSION_high;
	uint8_t RTE_VERSION_low;

	uint8_t chargeStatus;
	uint8_t boolPICdata;
	confirmbit8_Type confirmRequest; // confirmbit8_Type
	uint8_t boolWirelessData;

	uint8_t boolPressureData;
	uint8_t boolCompassData;
	uint8_t boolTisssueData;
	uint8_t boolCrushingData;

	uint8_t boolToxicData;
	uint8_t boolTimeData;
	uint8_t boolBatteryData;
	uint8_t boolAmbientLightData;

	uint8_t accidentFlags;
	uint8_t sensorErrors;
	uint8_t spare2;
	uint8_t spare3;

	SExchangeData data[2];
	SDataExchangeFooter footer;
	uint8_t CRC_feature_by_SPI[4];
} SDataExchangeSlaveToMaster;


typedef struct
{
	SDataExchangeHeader header;

	uint8_t mode;
	uint8_t power_on_reset;
	uint8_t RTE_VERSION_high;
	uint8_t RTE_VERSION_low;

	uint8_t chargeStatus;
	uint8_t spare1;
	uint8_t spare2;
	uint8_t spare3;

	uint8_t boolDeviceData;
	uint8_t boolVpmRepetitiveDataValid;
	uint8_t bool3;
	uint8_t bool4;

	uint8_t spare1_1;
	uint8_t spare1_2;
	uint8_t spare1_3;
	uint8_t spare1_4;

	uint8_t spare2_1;
	uint8_t spare2_2;
	uint8_t spare2_3;
	uint8_t spare2_4;

	SDevice DeviceData[2];
	SVpmRepetitiveData VpmRepetitiveData;

	uint8_t arraySizeOfMinimumSExChangeDate[(2 * sizeof(SExchangeData)) - ((2 * sizeof(SDevice)) + sizeof(SVpmRepetitiveData))];
	SDataExchangeFooter footer;
	uint8_t CRC_feature_by_SPI[4];
} SDataExchangeSlaveToMasterDeviceData;


typedef struct
{
	SDataExchangeHeader header;

	uint8_t mode;
	uint8_t getDeviceDataNow;
	uint8_t diveModeInfo;
	uint8_t setEndDive;

	uint8_t bool4;
	uint8_t setButtonSensitivityNow;
	uint8_t setDateNow;
	uint8_t setTimeNow;

	uint8_t calibrateCompassNow;
	uint8_t clearDecoNow;
	uint8_t setBatteryGaugeNow;
	uint8_t bool9;

	uint8_t revisionHardware;
	uint8_t revisionCRCx0x7A;
	uint8_t spare1_3;
	uint8_t spare1_4;

	uint8_t setAccidentFlag;
	uint8_t spare2_1;
	uint8_t spare2_2;
	uint8_t spare2_3;

	SReceiveData data;
	uint8_t arraySizeOfMinimumSExChangeDate[(2 * sizeof(SExchangeData)) - sizeof(SReceiveData)];
	SDataExchangeFooter footer;
	uint8_t CRC_feature_by_SPI[4];
} SDataReceiveFromMaster;


/* Size of Transmission buffer */
#define EXCHANGE_BUFFERSIZE			(sizeof(SDataExchangeSlaveToMaster) - 2)
#define EXCHANGE_BUFFERSIZE2			(sizeof(SDataReceiveFromMaster) - 2)
// header: 	  	5
// mode+bool:   5
// data				552 ( 69 * float/4 * 2 )
// footer:		  4
// ______________
// SUM				566
// CRC_feature does not count into BUFFERSIZE!

//(COUNTOF(struct SDataExchangeSlaveToMaster) + 1)

/* Exported macro ------------------------------------------------------------*/
//#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

#endif /* DATA_EXCHANGE_H */

