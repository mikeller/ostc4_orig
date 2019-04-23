///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Inc/ostc_discovery.h
/// \brief  Hardware specific configuration
/// \author heinrichs weikamp gmbh
/// \date   05-Dec-2014
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

#ifndef OSTC_DISCOVERY_H
#define OSTC_DISCOVERY_H

#define DISPLAY_RESETB_PIN			GPIO_PIN_6
#define DISPLAY_RESETB_GPIO_PORT	GPIOF
#define DISPLAY_RESETB_GPIO_ENABLE()__GPIOF_CLK_ENABLE()

#define DISPLAY_CSB_PIN             GPIO_PIN_8
#define DISPLAY_CSB_GPIO_PORT		GPIOC
#define DISPLAY_CSB_GPIO_ENABLE()   __GPIOC_CLK_ENABLE()

#define VSYNC_IRQ_PIN               GPIO_PIN_4
#define VSYNC_IRQ_GPIO_PORT			GPIOD
#define VSYNC_IRQ_GPIO_ENABLE()    __GPIOD_CLK_ENABLE()
#define VSYNC_IRQ_EXTI_IRQn         EXTI4_IRQn

#define BUTTON_ENTER_PIN            GPIO_PIN_0
#define BUTTON_ENTER_GPIO_PORT      GPIOA
#define BUTTON_ENTER_GPIO_ENABLE()  __GPIOA_CLK_ENABLE()
#define BUTTON_ENTER_EXTI_IRQn		EXTI0_IRQn

#define BUTTON_DOWN_PIN				GPIO_PIN_1
#define BUTTON_DOWN_GPIO_PORT		GPIOA
#define BUTTON_DOWN_GPIO_ENABLE()   __GPIOA_CLK_ENABLE()
#define BUTTON_DOWN_EXTI_IRQn       EXTI1_IRQn

#define BUTTON_NEXT_PIN             GPIO_PIN_2
#define BUTTON_NEXT_GPIO_PORT		GPIOA
#define BUTTON_NEXT_GPIO_ENABLE()   __GPIOA_CLK_ENABLE()
#define BUTTON_NEXT_EXTI_IRQn       EXTI2_IRQn

#define BUTTON_BACK_PIN             GPIO_PIN_3
#define BUTTON_BACK_GPIO_PORT       GPIOC
#define BUTTON_BACK_GPIO_ENABLE()   __GPIOC_CLK_ENABLE()
#define BUTTON_BACK_EXTI_IRQn       EXTI3_IRQn

#define EXTFLASH_CSB_PIN            GPIO_PIN_1
#define EXTFLASH_CSB_GPIO_PORT		GPIOC
#define EXTFLASH_CSB_GPIO_ENABLE()  __GPIOC_CLK_ENABLE()

#define OSCILLOSCOPE_PIN			GPIO_PIN_14
#define OSCILLOSCOPE_GPIO_PORT		GPIOG
#define OSCILLOSCOPE_GPIO_ENABLE()  __GPIOG_CLK_ENABLE()

#define OSCILLOSCOPE2_PIN			GPIO_PIN_13
#define OSCILLOSCOPE2_GPIO_PORT		GPIOG
#define OSCILLOSCOPE2_GPIO_ENABLE() __GPIOG_CLK_ENABLE()

#define USARTx                      USART1
#define USARTx_CLK_ENABLE()         __USART1_CLK_ENABLE();
#define USARTx_FORCE_RESET()        __USART1_FORCE_RESET()
#define USARTx_RELEASE_RESET()      __USART1_RELEASE_RESET()

#define USARTx_RX_AF                GPIO_AF7_USART1
#define USARTx_RX_PIN               GPIO_PIN_10
#define USARTx_RX_GPIO_PORT         GPIOA
#define USARTx_RX_GPIO_CLK_ENABLE() __GPIOA_CLK_ENABLE()

#define USARTx_TX_AF                GPIO_AF7_USART1
#define USARTx_TX_PIN               GPIO_PIN_9
#define USARTx_TX_GPIO_PORT         GPIOA
#define USARTx_TX_GPIO_CLK_ENABLE() __GPIOA_CLK_ENABLE()
#define USARTx_IRQn                 USART1_IRQn
#define USARTx_IRQHandler           USART1_IRQHandler

#define TIMx                        TIM3
#define TIMx_CLK_ENABLE             __TIM3_CLK_ENABLE
#define TIMx_IRQn                   TIM3_IRQn
#define TIMx_IRQHandler             TIM3_IRQHandler

#define SMALLCPU_CSB_PIN			GPIO_PIN_15
#define SMALLCPU_CSB_GPIO_PORT		GPIOA
#define SMALLCPU_CSB_GPIO_ENABLE()  __GPIOA_CLK_ENABLE()

#endif // OSTC_DISCOVERY_H
