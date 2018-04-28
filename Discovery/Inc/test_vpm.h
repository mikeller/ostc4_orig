///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/test_vpm.h
/// \brief
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

#ifndef TEST_VPM_H
#define TEST_VPM_H

#include "buehlmann.h"

//_Bool simulate_descent(SBuehlmann* pInput, float ending_depth_meter, float rate_meter_per_minutes);
//void init_buehlmann(SBuehlmann* pInput);
//void init_buehlmann2(SBuehlmann* pInput);
_Bool test1(void);

//uint8_t test5_unapproved(uint32_t frame1, uint32_t frame2, uint32_t frame3, uint32_t frame4);
uint8_t test5_unapproved(void);
uint8_t test6_unapproved(void);
uint8_t test7_unapproved(void);
void test_log_only(uint8_t max_depth_meter, uint16_t divetime_minutes);

uint8_t test101_buehlmann_unapproved(void);

#endif // TEST_VPM_H
