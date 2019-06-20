///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/timer.h
/// \brief  Contains timer related functionality like stopwatch and security stop
/// \author Peter Ryser
/// \date
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TIMER_H
#define TIMER_H

void timer_init(void);
void timer_UpdateSecond(_Bool checkOncePerSecond);
void timer_Stopwatch_Restart(void);
void timer_Stopwatch_Stop(void);
long timer_Stopwatch_GetTime(void);
float timer_Stopwatch_GetAvarageDepth_Meter(void);
long timer_Safetystop_GetCountDown(void);
uint8_t timer_Safetystop_GetDepthUpperLimit(void);

#endif // TIMER_H
