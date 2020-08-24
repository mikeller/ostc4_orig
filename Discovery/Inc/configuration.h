///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/configuration.h
/// \brief  Header file for variant specific firmware adaptations at compile time
/// \author heinrichs weikamp gmbh
/// \date   29-February-2020
///
/// $Id$
///////////////////////////////////////////////////////////////////////////////
/// \par Copyright (c) 2014-2020 Heinrichs Weikamp gmbh
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


#ifndef CONFIGURATION_HEADER
#define CONFIGURATION_HEADER

/* Enable this to make the simulator write a logbook entry */
/* #define SIM_WRITES_LOGBOOK 1 */

/* Enable this for support of optical bottle pressure interface */
/* #define ENABLE_BOTTLE_SENSOR */

/* Enable this to show voltage in parallel to charge state */
/* #define ALWAYS_SHOW_VOLTAGE */

/* Enable this to skip coplete scan of dive log during startup */
/* #define TRUST_LOG_CONSISTENCY */

/* Enable this to transfer additional data list last dive ID and last sample index during raw data requests */
/* define SEND_DATA_DETAILS */

/* Enable to activate a menu item in reset menu which provide sample ring analysis / repair functionality */
/* #define ENABLE_ANALYSE_SAMPLES */

/* Enable to have access to the debug view options (turn on / off via menu instead of compile switch) */
/* #define HAVE_DEBUG_VIEW */

/* Enable to add new BIG font layout to option menu */
#define ENABLE_BIGFONT_VX
#endif
