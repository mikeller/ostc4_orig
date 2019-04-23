///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/stm32f4xx_it.c
/// \brief  This function handles System tick timer and all interupt vectors.
/// \author Heinrichs Weikamp gmbh
/// \date   18/03/2015 15:35:40
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
///
//////////////////////////////////////////////////////////////////////////////
/// \par Copyright (c) COPYRIGHT(c) 2015 STMicroelectronics
///
///     Redistribution and use in source and binary forms, with or without modification,
///     are permitted provided that the following conditions are met:
///       1. Redistributions of source code must retain the above copyright notice,
///          this list of conditions and the following disclaimer.
///       2. Redistributions in binary form must reproduce the above copyright notice,
///          this list of conditions and the following disclaimer in the documentation
///          and/or other materials provided with the distribution.
///       3. Neither the name of STMicroelectronics nor the names of its contributors
///          may be used to endorse or promote products derived from this software
///          without specific prior written permission.
///
///     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
///     AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
///     IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
///     FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
///     DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
///     SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
///     CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
///     OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
///     OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//////////////////////////////////////////////////////////////////////////////

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
/* USER CODE BEGIN 0 */

#include "ostc.h"

#ifdef DEMOMODE
#	include "demo.h"
    extern TIM_HandleTypeDef    TimDemoHandle;
#endif

extern DMA2D_HandleTypeDef	Dma2dHandle;
extern TIM_HandleTypeDef    TimHandle;

/* USER CODE END 0 */
/* External variables --------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Interruption and Exception Handlers         */
/******************************************************************************/

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void DMA2D_IRQHandler(void)
{
  HAL_DMA2D_IRQHandler(&Dma2dHandle);
}


void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(cpu2DmaSpi.hdmarx);
}


void DMA2_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(cpu2DmaSpi.hdmatx);
}


void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI1_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void EXTI3_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

void EXTI4_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}


void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartHandle);
}


#ifdef USART_IR_HUD
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartIR_HUD_Handle);
}
#endif


void TIM4_IRQHandler(void)
{
    MX_TestPin_Low();
  HAL_TIM_IRQHandler(&TimHandle);
    MX_TestPin_High();
}


#ifdef DEMOMODE
void TIM7_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TimDemoHandle);
}
#endif
