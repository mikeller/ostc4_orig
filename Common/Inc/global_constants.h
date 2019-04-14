///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Common/Inc/global_constants.h
/// \brief
/// \author Dmitry Romanov<kitt@bk.ru>
/// \date   11.2018
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

#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H
//Rename it ahead.
//Buttons section
#define DEFAULT_BUTTONRESPONSIVENESS_GUI 85
#define MIN_BUTTONRESPONSIVENESS_GUI 30 //50 (-10correction)
#define MAX_BUTTONRESPONSIVENESS_GUI 110//100 (+10correction)
#define MIN_BUTTONRESPONSIVENESS MIN_BUTTONRESPONSIVENESS_GUI-20 //MIN_BUTTONRESPONSIVENESS_GUI-20 correction
#define MAX_BUTTONRESPONSIVENESS MAX_BUTTONRESPONSIVENESS_GUI+20//MIN_BUTTONRESPONSIVENESS_GUI+20correction
#define BUTTON_DEBOUNCE_DELAY 50 //Delay for debounce filter for piezo buttons


// Spi sync data debug
#define SPI_SHOW_SYNC_STATS 0
#define SPI_MIN_ERROR_SHOW 10

/* Define INDEX for information exchanged within the header */
#define SPI_HEADER_INDEX_RX_STATE  (1)
#define SPI_HEADER_INDEX_TX_TICK   (2)

#define SPI_RX_STATE_OK 		(0)
#define SPI_RX_STATE_SHIFTED	(1)
#define SPI_RX_STATE_OFFLINE	(2)
#define SPI_RX_STATE_INVALID	(3)

//Text data
#define TEXT_PRESSURE_UNIT "hPa"
#endif
