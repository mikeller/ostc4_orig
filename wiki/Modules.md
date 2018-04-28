# OSTC4 Code Modules #

## Main CPU - aka CPU1-Discovery ##

For historical reasons, the firmware of the main CPu was divided in three parts.
It helps development, allow smalled upgrades, and improve security as a bad manipulation won't kill the bootloader, responsible fot the next firmware upgrade.

### Bootloader ##

The code responsible to upload new firmwares for both CPU.

### Font Pack ###

This module does not contains any code. Just font data and images.

### Firmware code ###

The main OSTC4 firmware. The more important modules are:

- Buehlmann decompression algorithm.
- VPM decompression algorithm.
- The simulator and dive planner.
- Logbook display (surface and dive mode).
- All user interface code, during dive or surface mode (`t*.c` files).
- Graphics (gfx) engine.
- Plus interface to the other CPU, the management of the _LCD_ screen, and the optional _Bonnex_ interface.

## Small CPU - aka CPU2-RTE ##

The _Real Time_ CPU handle everything that requires precise timming to function perperly.

The most important modules are:

- Real time computation of gas exchanges during dive (aka the _LifeData_ structure).
- Analog to digital conversion for the ... ambient light sensor.
- I2C bus interface to the pressure sensor, the magnetic compass and the battery gauge.
- SPI bus interface to the buttons and to the other CPU.
- Serial interface to the _RS232-over-Wireless_ connection.
