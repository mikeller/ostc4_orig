# How to Create From Scratch a Project for _CPU2-RTE_ Code #

Follow the same procedure than for [CPU1](Detailed CPU1-Discovery Project.md), with changes:

- Create a new _board_ for processor:  
>   Series: `STM32F4`  
>   Mcu: `STM32F411xE`  
>   Debug: `JTAG`  

- Add `Small_CPU` linked source directory (instead of _Discovery_)

- Use linker script `OSTC4/Small_CPU/CPU2-F411.ld`
