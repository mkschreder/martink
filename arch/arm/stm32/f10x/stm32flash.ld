/*
* STM32 Discovery Linker Script
*/

_ramstart = 0x20000000; 
_estack = 0x20004FF0;    /* global needed for startup_stm3210x_md_vl.S */
_minimum_stack_size = 0x100; /* 256 bytes miniumum stack size */

ENTRY(Reset_Handler)


MEMORY
{
  rom (rx)      : ORIGIN = 0x08000000, LENGTH = 128K
  ram (xrw)     : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
  .text : 
  {
    KEEP(*(.isr_vector .isr_vector.*))
    
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    
    . = ALIGN(4);
    __init_array_start = .;
    KEEP(*(.init_array));
    __init_array_end = .;
    . = ALIGN(4);
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
    _etext = .; 
  } >rom
  
  /*now start ram area with initialized data section */
  .data : 
  {   
    _sdata = .;        /* global needed for startup_stm3210x_md_vl.S */
    *(.data)          
    *(.data.*)         
    . = ALIGN(4);
    _edata = .;        /* global needed for startup_stm3210x_md_vl.S */
  } >ram AT>rom
	
	.device_list : 
  {   
    _sdevice_info = .;        
    KEEP(*(.device_info*))
    . = ALIGN(4);
    _edevice_info = .;  
    _sdevice_meta = .;        
    KEEP(*(.device_meta*))      
    . = ALIGN(4);
    _edevice_meta = .;       
  } >rom
  
  /* uninitialized data section bss*/
  .bss (NOLOAD): 
  {
      . = ALIGN(4);
    _sbss = .;         /* global needed for startup_stm3210x_md_vl.S */
    *(.bss)
    *(.bss*)
    . = ALIGN(4);
    _ebss = .;        /* global needed for startup_stm3210x_md_vl.S */
  } >ram

 
  /* Generates an error if minimum stack size is not left in ram */
  ._check_stack_space :
  {
    . = ALIGN(4);
    . = . + _minimum_stack_size;
    . = ALIGN(4);
  } >ram

  .ARM.attributes 0 : { KEEP (*(.ARM.attributes)) }
}

