# OSTC4 Resources #

## Hardware ##

There are two processors in the OSTC4:

* an [STM32F427IIT6](http://www.st.com/content/st_com/en/products/microcontrollers/stm32-32-bit-arm-cortex-mcus/stm32-high-performance-mcus/stm32f4-series/stm32f427-437/stm32f427ii.html) as main processor. It is a Cortex-M4 processor with FPU, running at 180 MHz.
It crunches most of the decompression code, the graphic display operations and the user interface.

* an [STM32F411RET6](http://www.st.com/en/microcontrollers/stm32f411re.html) as the RTE (Real-Time-Environment) processor.
This is another processor of the ARM Cortex M4 family, but with extremely low power modes and plenty of communication ports.
It runs real-time gas exchange simulation during dive, and interface to most peripherals.

Various peripherals are connected to them:

- A 800x480 pixels _LCD_ screen, with a live stream of color pixels send by an auto-refresh SDRAM.
- A pressure sensor.
- A magnetic compass.
- An ambient light sensor.
- Three buttons.
- A battery, and a battery gauge.

_OSTC4_ programming is based on the _HAL_ (for _Hardware Abstraction Layer_) provided by _ST_.
This allows to have a more readable C code, and to evolve more easily to another processors of the same familly.

## Drivers Used by the Main CPU - aka CPU1-Discovery ##

The main _ARM-Cortex 4_ CPU uses the following resources provided by the
_HAL_ (_Hardware Abstraction Layer_)
provided by the processors makers _ST_ to ease code developement, security and portability:

- **HAL_DMA** _Direct Memory Accelerator_: automated memory transfers, used to manage SPI reception and transmission channels.
- **HAL_DMA2D** : a _specialized DMA dedicated to images manipulation_, used by `gfx_engine`to clear screens.
- **HAL_FMC** _Flexible Memory Controller_ to drice the external SDRAM used for display.
- **HAL_GPIO** _General Purpose Input/Output_: this is the programmable pins of the chip, used to connect nearly evrything.
- **HAL_LTDC** _LCD TFT Display Controller_: to transfer images to the screen memory ?
- **HAL_NVIC** _Nested Vector Interrupt Controller_: CORTEX IRQs used in `base.c` to handle buttons interrupts, in `gfx_engine.c` to manage VSYNC (screen's vertical sync) and in `stm32f4xx_hal_msp_hw2.c` to manage all other interrupts (ticks, DMA, DMA2D, TIM and USART).
- **HAL_RCC** _Reset and Clock Control_: used in `base.c` to setup system, PLL and screen clocks.
- **HAL_SDRAM** RAM controller used in `stm32f4xx_hal_msp_hw2.c` and in `base.c` to interface the external memory used by the screen.
- **HAL_SPI** _Serial Peripheral Interface_: implement the [standard SPI protocol](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus). Used to do full duplex communication between the two CPU in `data_exchange_main.c` to exchange dive settings and state, and in `externCPU2bootloader.c` to send firmware to _CPU2-RTE_. 
- **HAL_TIM** basic _Timers_ used in `base.c` and PWM (_Pulse Width Modulation_) used to control backlight intensity.
- **HAL_UART** _Universal Asynchronous Receiver Transmitter_: serial communications used in `bonnexConnect.c`, `ostc.c`, `stm32f4xx_hal_msp_hw2.c`, `tCCR.c`, `tCpmm.c` and `tDebug.c`

## Drivers Used by the Secondary CPU - aka CPU2-RTE ##

- **HAL_ADC** _Analog to Digital Converter_ channel 1 is used in `adc.c` for the ambient light sensor.
- **HAL_DMA** _Direct Memory Accelerator_: automated memory transfers, used to manage SPI reception and transmission channels when communicating to main CPU.
- **HAL_FLASH** allow erasing, programming, and managing read/write protection mechanisms of the internal FLASH memory.
- **HAL_GPIO** _General Purpose Input/Output_: this is the programmable pins of the chip, used to connect nearly evrything.
- **HAL_I2C** _Inter-Integrated Circuit_ [bus controller](https://en.wikipedia.org/wiki/I2C) is another standard bus, used to connect the pressure sensor in `pressure.c`, the magnetic compass in `compass.c` and the battery state in `batteryGauge.c`. Initialisations are in `baseCPU2.c` and `stm32f4xx_hal_msp_v3.c`.
- **HAL_NVIC** _Nested Vector Interrupt Controller_: CORTEX IRQs used in `baseCPU2.c` to handle clock ticks, buttons and wireless state. In `spi.c` to know end of SPI transmissions. And in `stm32f4xx_hal_msp_v3.c` to setup various _I2C_ interupts.
- **HAL_PWR** _Power Controller_: to manage sleep mode, low power and wakeup. And in `rtc.c` to manage FLASH power down.
- **HAL_RCC** _Reset and Clock Control_: used in `baseCPU2.c` to setup system, PLL clocks for ...
- **HAL_RTC** _Real Time Clock_ to handle date and time in `scheduler.c`
- **HAL_SPI** _Serial Peripheral Interface_: implement the [standard protocol](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus). Used to do full duplex communication between the two CPUs. In `spi.c`, SPI1 is used to communicate to the maon CPU, and SPI3 to commicate to buttons.
- **HAL_UART** _Universal Asynchronous Receiver Transmitter_: serial communications used in `uart.c`