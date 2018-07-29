/* ---------------------------------------------------------------------------- */
/*                  Em::Blocks embedded development Support                     */
/* ---------------------------------------------------------------------------- */
/* Copyright (c) 2014, EmBlocks                                                 */
/*                                                                              */
/* All rights reserved.                                                         */
/*                                                                              */
/* Redistribution and use in source and binary forms, with or without           */
/* modification, are permitted provided that the following condition is met:    */
/*                                                                              */
/* - Redistributions of source code must retain the above copyright notice,     */
/* this list of conditions and the disclaimer below.                            */
/*                                                                              */
/* EmBlocks's name may not be used to endorse or promote products derived from  */
/* this software without specific prior written permission.                     */
/*                                                                              */
/* DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY EBLOCKS "AS IS" AND ANY EXPRESS OR */
/* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE   */
/* DISCLAIMED. IN NO EVENT SHALL EMBLOCKS BE LIABLE FOR ANY DIRECT, INDIRECT,   */
/* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,  */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    */
/* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING         */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, */
/* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                           */
/* ---------------------------------------------------------------------------- */

/*------------------------------------------------------------------------------
 *      Linker script for running in internal FLASH on the STM32F401RE
 *----------------------------------------------------------------------------*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
SEARCH_DIR(.)

/* Memory Spaces Definitions */
MEMORY
{
    ROM  (rx) : ORIGIN = 0x08000000, LENGTH = 512K /* 80000 */
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 96K
}
/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 * 
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 */


SECTIONS
{
  .isr_vector 0x08000000 :
  {
  . = ALIGN(4);
    KEEP( *(.isr_vector) ) 
    KEEP(*(.init))
    KEEP(*(.fini))
  } >ROM
  
	/* Place FirmwareData at absolute address */
	.firmware_data 0x08005000:
	{
		cpu2_FirmwareData = 0; 
		KEEP( *(.firmware_data) )
	} > ROM

	.text 0x08005100 :
	{	
		. = ALIGN(4);
		*(.text)           /* .text sections (code) */
		*(.text*)
		*(.eh_frame*)
        . = ALIGN(4); 
  
	} > ROM

/********************** Constant data into ROM memory *********************/
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >ROM
  
	.ARM.extab :
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > ROM

	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > ROM
	__exidx_end = .;

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
    
    __etext = .;        /* define a global symbols at end of code */
  } >ROM

 /* Used by the startup to initialize data */
 	_sidata = LOADADDR(.data);

	.data : 
	{
		. = ALIGN(4);
		__data_start__ = .;
		_sdata = .;        /* create a global symbol at data start */
    	*(.data)           /* .data sections */
		*(.data*)
		*(vtable)
		. = ALIGN(4);
		/* All data end */
		__data_end__ = .;
	} >RAM AT>ROM

	.bss :
	{
		__bss_start__ = .;
		*(.bss*)
		*(COMMON)
		__bss_end__ = .;
	} >RAM
	
	.heap :
	{
		__end__ = .;
		end = __end__;
		*(.heap*)
		__HeapLimit = .;
	} > RAM

	/* .stack_dummy section doesn't contains any symbols. It is only
	 * used for linker to calculate size of stack sections, and assign
	 * values to stack symbols later */
	.stack_dummy :
	{
		*(.stack)
	} > RAM

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
	__StackTop = ORIGIN(RAM) + LENGTH(RAM);
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __StackTop);
	
	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
}
