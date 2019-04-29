///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Common/Inc/calc_crush.h
/// \brief  VPM Desaturation code
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

#ifndef CALC_CRUSH_H
#define CALC_CRUSH_H

#include "data_central.h"

void vpm_init(SVpm* pVpm,
							short conservatism, short repetitive_dive,
							long seconds_since_last_dive);

int calc_crushing_pressure(SLifeData* lifeData,  SVpm* vpm,
													 float * initial_helium_pressure,
													 float * initial_nitrogen_pressure,
													 float starting_ambient_pressure,
													 float rate );

#endif // CALC_CRUSH_H
