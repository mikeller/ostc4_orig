///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Common/Inc/FirmwareData.h
/// \brief  Firmware Signature (version, makers, date, etc.)
/// \author Heinrichs Weikamp, jDG
/// \date   2017-12-31
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

#ifndef FIRMWARE_DATA_H
#define FIRMWARE_DATA_H

#include <stdint.h>

/// @brief Firmware Signature
///
/// Contains
typedef struct {
	uint8_t versionFirst;				///< Major version number
	uint8_t versionSecond;				///< Minor version number
	uint8_t versionThird;
	uint8_t versionBeta;
	uint8_t signature[4];
	uint8_t release_year;
	uint8_t release_month;
	uint8_t release_day;
	uint8_t release_sub;
	char release_info[48];
	char magic[4];
} SFirmwareData;

enum FirmwareMagic {
		FIRMWARE_MAGIC_FIRST     = 0x00, ///< type[0]
		FIRMWARE_MAGIC_SECOND    = 0x00, ///< type[1]
		FIRMWARE_MAGIC_FIRMWARE  = 0xEE,
		FIRMWARE_MAGIC_CPU2_RTE  = 0xE2,
		FIRMWARE_MAGIC_FONT      = 0xF0,
		FIRMWARE_MAGIC_END       = 0xFF  ///< type[2]
};

extern const SFirmwareData font_FirmwareData;
extern const SFirmwareData bootloader_FirmwareData;
extern const SFirmwareData firmware_FirmwareData;

extern const SFirmwareData cpu2_FirmwareData;

#endif /* FIRMWARE_DATA_H */
