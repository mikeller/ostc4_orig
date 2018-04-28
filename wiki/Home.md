# Open Source Code for the OSTC4 Dive Computer #


## Documentation ##

[Modules](Modules.md) page details the code organisation.    
[Hardware Ressources](Hardware Ressources.md) page details the hardware parts,
the low-level, and the middle-ware drivers used to manage them.    

## How to Compile ? ##

Most of the code is written in _C-99_. A small glue is in assembly. Everything is compatible with
the _GCC_ toolchain for ARM processors.

* [Using OpenSTM32](Using OpenSTM32.md), the free integrated IDE from the OpenSTM32 community, based on GCC cross compiler within an _Eclipse_ environment. Project management and configuration. Compile & Link. Hardware debugger through a JTAG/SWD probe.

## Other Tools ##

* [STM32F429I-DISC1](http://www.st.com/en/evaluation-tools/32f429idiscovery.html)
  It is a chip (~ 20€) evaluation board, that contains a _STM32F429ZIT6_ CPU, 2MB flash ROM, 256KB RAM, a 320x240 LCD touch screen a few LEDs and a pair of buttons. Very cheap (~ 25 euro).  
  It should allows to run _OSTC4 Discovery code_, but just for basic testing as it lacks all specific sensors, LCD screen 800x640 resolution and the 3 interface buttons.  
  _An interesting project would be to add a special compilation option to get some reasonable support on this chip debugging platform, emulating the screen and buttons..._

* [STM32 ST-LINK Utility](http://www.st.com/en/development-tools/stsw-link004.html) (**Windows only**) allow to do many things to the _discovery kit_ through its JTAG/SWD interface: upload & verify application code, dump memory. Provides both a _Graphical_ and a _Command Line_ interface.  

* [STMStudio](http://www.st.com/en/development-tools/stm-studio-stm32.html) (**WINDOWS ONLY**) allows real-time inspection of an application running on the _discovery kit_.  
  It is connected through the JTAG/SWD interface.
  It allows to read and write variables defined in the application's binary (ELF file). To display or to trace graphs (eg. to display gas loading during dive simulation ?)

* [OpenOCD](http://openocd.org/) is another command utility to speak JTAG/SWD. Seems hard to use directly (very low level commands). But _OpenSTM32 IDE_ do use it for programming and debugging, and you can copy configuration files from a working project. Note version 0.10 (or above) seems mandatory to dump firmware contents.

* _**OSTC4 with 2 SWD connectors**_, the ultimate debugging tool: a specially made OSTC4, with a pair of extra SWD connectors you can connect to any JTAG/SWD probe. Cannot dive (absolutely not water tight), but allows to debug code on the real hardware (ie. set break points, single step the code and read/write variables), using a standard JTAG/SWD debuggers (eg. a low cost [ST-Link/V2](http://www.st.com/stlinkv2) for ~ 25 euro).  
  _Note that H&W will freely provide SWD OSTC4 platform to programmer serious about involving in code development._

## Other Resources ##

* The **Ultimate Reference** for everyone serious about the OSTC4 firmware details
and _HAL_ programming (1800 pages): [UM1725 HAL Users Manual](www.st.com/resource/en/user_manual/dm00105879.pdf)
* Reserving memory from the linker script: https://mcuoneclipse.com/2012/11/01/defining-variables-at-absolute-addresses-with-gcc/
* Placing a variable at an absolute address: https://sourceware.org/binutils/docs-2.24/ld/Source-Code-Reference.html#Source-Code-Reference