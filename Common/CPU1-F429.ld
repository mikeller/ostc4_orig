/*****************************************************************************
 * -*- coding: UTF-8 -*-
 *
 * \file   Common/CPU1-F429.ld
 * \brief  Linker Script for CPU1 alias Discovery
 * \author Ac6 workbench, Heinrichs Weikamp, and JD Gascuel
 * \date   2018
 *
 * \details
 *  Linker script for STM32F429IIT6 Device from STM32F4 series
 *  - 192Kbytes RAM
 *  - 2048Kbytes ROM
 *
 *  Set heap size, stack size and stack location according
 *  to application requirements.
 *
 *  Set memory bank area and size if external memory is used.
 *
 * $Id$
 *****************************************************************************
 * \par Copyright (c) 2014-2018 Heinrichs Weikamp gmbh
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************
 * HISTORY
 *  2017-12-18 jDG: Mix between std AC6 file and chw stm32f429zi_flash.ld settings.
 */

/* Entry Point */
ENTRY(Reset_Handler)

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
SEARCH_DIR(.)

/****************************************************************************/
/***************************** Memory Definition ****************************/
MEMORY
{
    ROM  (rx) : ORIGIN = 0x08000000, LENGTH = 0x100000 /* 1024K */
    UPPER(rx) : ORIGIN = 0x08100000, LENGTH = 0x100000 /* 1024K */
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 192K
  CCRAM (rwx) : ORIGIN = 0x10000000, LENGTH = 64K
}

/* Make sure ROM and UPPER are contiguous, and map 2MB */
ASSERT( ORIGIN(UPPER) - ORIGIN(ROM) == LENGTH(ROM), "ROM lower and upper region shall be continuous" )
ASSERT( LENGTH(ROM) + LENGTH(UPPER) == 2048K, "ROM lower + upper regions = 2MB" )

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Used by the startup to initialize data */
_sidata = LOADADDR(.data);

_Min_Heap_Size  = 0;      	/* no required heap  (malloc'ed space) */
_Min_Stack_Size = 0x8000; 	/* required 32K of stack */

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 * 
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *	 _sbss
 *   __bss_start__
 *	 _ebss
 *   __bss_end__
 *   _edata
 *   end
 *   _end
 *   _estack
 */

/****************************************************************************/
/*************************** Code and Data Sections *************************/
SECTIONS
{
  /******************* NVIC reset and interupt vectors **********************/
  .isr_vector 0x08000000 :
  {
    KEEP( *(.isr_vector) ) 
  } >ROM

  /************* The program code and other data into ROM memory ************/
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >ROM

  .preinit_array     :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN( __preinit_array_start = . );
    KEEP( *(.preinit_array*) )
    PROVIDE_HIDDEN( __preinit_array_end = . );
    . = ALIGN(4);
  } >ROM
  
  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN( __init_array_start = . );
    KEEP( *(SORT(.init_array.*)) )
    KEEP( *(.init_array*) )
    PROVIDE_HIDDEN( __init_array_end = . );
    . = ALIGN(4);
  } >ROM
  
  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN( __fini_array_start = . );
    KEEP( *(SORT(.fini_array.*)) )
    KEEP( *(.fini_array*) )
    PROVIDE_HIDDEN( __fini_array_end = . );
    . = ALIGN(4);
  } >ROM

  /********************** Constant data into ROM memory *********************/
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >ROM

  .ARM.extab   : { 
  	. = ALIGN(4);
  	*(.ARM.extab* .gnu.linkonce.armextab.*)
  	. = ALIGN(4);
  } >ROM
  
  .ARM : {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >ROM
  
  /************* Data and Structures at absolute address in ROM *************/
 
 /* FIXME: this sets the SFirmwareData structure, not the code ... */
 .bootloader_firmware_data 0x0800A000 : {
 	PROVIDE( bootloader_FirmwareData = . );
 	KEEP( *(.bootloader_firmware_data) )
 } > ROM
 	
  /* FIMXE: original address was 0x08050000. But we have a collision */
 .firmware_FirmwareData 0x08060000 : {
 	PROVIDE( firmware_FirmwareData = . );
 	KEEP( *(.firmware_firmware_data) )
  } >ROM

  .font_firmware_data 0x08132000 : {
  	PROVIDE( font_FirmwareData = . );
  	KEEP( *(.font_firmware_data) )
  } >UPPER
  
  /* Put fonts and images in UPPER region */  
  .upper_fonts 	: {
  	__upper_font_data = .;
  	KEEP( *(.upper_fonts) )
  	__upper_font_data_end = .;
  } >UPPER

  .upper_font_directory 0x081E0000 : { /* Last 128 KBytes of UPPER ROM */
  
    /* Define Known Address for Each Font */
  	__font_directory = .;
	PROVIDE( Awe48    = __font_directory + 0 * 0xc );  
	PROVIDE( FontT24  = __font_directory + 1 * 0xc );
	PROVIDE( FontT42  = __font_directory + 2 * 0xc );
	PROVIDE( FontT48  = __font_directory + 3 * 0xc );
	PROVIDE( FontT54  = __font_directory + 4 * 0xc );
	PROVIDE( FontT84  = __font_directory + 5 * 0xc );
	PROVIDE( FontT105 = __font_directory + 6 * 0xc );
	PROVIDE( FontT144 = __font_directory + 7 * 0xc );
	PROVIDE( Batt24   = __font_directory + 8 * 0xc );
	__font_directory_end = __font_directory + 9 * 0xc;

	KEEP( *(.upper_font_directory) )
  	LONG( -1 );	/* Magic End Marker */
	
    /* and for Each Image */
	__image_directory = .;
	PROVIDE( ImgHWcolor = __image_directory + 0 * 8 );
	PROVIDE( ImgOSTC    = __image_directory + 1 * 8 );
	__image_directory_end = __image_directory + 2 * 8;

	/* FIXME: shall KEEP images here */
  	LONG( -1 );	/* Magic End Marker */
	
    /* and for CLUT */
	PROVIDE( indexHWcolor = . );
	. += 0x400;
  	LONG( -1 );	/* Magic End Marker */
  } >UPPER  
  
  /************ Initialized Data Sections into RAM & ROM Memory **************/
  .data : 
  {
    . = ALIGN(4);
    
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
	*(vtable)

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM AT>ROM
  
  /*************** Uninitialized Data Section into RAM Memory ****************/
  .bss :
  {
	. = ALIGN(4);
    
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /********************** User_heap_stack section ****************************/
  /* just to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* Remove information from the compiler libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
