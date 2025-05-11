/**
 ******************************************************************************
 * @file *.lds
 * @version V1.0.2
 * @date 2018-12-3
 * @brief project lds, add some export symbol for rtos used,
 ******************************************************************************
 */



/*****************************************************************************/
/*                         memory used sample chart                          */
/*****************************************************************************
                         +SRAM-----------------+ 0x1ff00000
                         |                     |
                         |       CODE          |
                         |                     |
                         +---------------------+
                         |                     |
                         |                     |
                         |       RW            |	
                         |                     |
                         |                     |
                         +---------------------+
                         |                     |
                         |       heap          |
                         |                     |
                         +---------------------+ 
                         |       stack         |
                         +---------------------+ 0x1ff70000
******************************************************************************/
/* ROM_SIZE      = 1024*(70); */
/* FHEAP_SIZE    = 1024*(60); */
/* ASR_USED_SIZE = 1024*(330); */
/* RAM_SIZE      = 1024*(20); */
STACK_SIZE    = 0xC00;/*inclued in RW*/


/*****************************************************************************/
/*                  don't modify these!!! decided by system                  */
/*****************************************************************************/
INCLUDE common.lds
SRAM_START_ADDR            = 0x1ff51000;
SRAM_END_ADDR              = SRAM_HOST_END_ADDR;
SRAM_SIZE                  = (SRAM_END_ADDR - SRAM_START_ADDR);

/* ASSERT((SRAM_END_ADDR - SRAM_START_ADDR) == (ROM_SIZE + FHEAP_SIZE + ASR_USED_SIZE + RAM_SIZE), "RAM config error!"); */

__stack_size = STACK_SIZE;
__int_stack_size = STACK_SIZE;

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

MEMORY
{
	/* ROM (rx)       : ORIGIN = SRAM_START_ADDR,                                          LENGTH = ROM_SIZE */
	/* FHEAP (xrw)    : ORIGIN = SRAM_START_ADDR + ROM_SIZE,                               LENGTH = FHEAP_SIZE */
	/* ASR_USED (xrw) : ORIGIN = SRAM_START_ADDR + ROM_SIZE + FHEAP_SIZE,                  LENGTH = ASR_USED_SIZE */
	/* RAM (xrw)      : ORIGIN = SRAM_START_ADDR + ROM_SIZE + FHEAP_SIZE + ASR_USED_SIZE,  LENGTH = RAM_SIZE */
    SRAM (xrw)       : ORIGIN = SRAM_START_ADDR,                                          LENGTH = SRAM_SIZE
}

SECTIONS
{
    __stack_size = DEFINED(__stack_size) ? __stack_size : 2K;
    __int_stack_size = DEFINED(__int_stack_size) ? __int_stack_size : 1K;

    .init :
    {
        *(.vtable)
        KEEP (*(.load_addr))
        KEEP (*(SORT_NONE(.init)))
    } >SRAM AT>SRAM 


    .ilalign :
    {
        . = ALIGN(4);
        PROVIDE( _ilm_lma = . );
    } >SRAM AT>SRAM 

    .ialign :
    {
        PROVIDE( _ilm = . );
    } >SRAM AT>SRAM 

    .text :
    {
        *(.text.unlikely .text.unlikely.*)
        *(.text.startup .text.startup.*)
        *(.text .text.*)
        *(.gnu.linkonce.t.*)
    } >SRAM AT>SRAM 

    .fini :
    {
        KEEP (*(SORT_NONE(.fini)))
    } >SRAM AT>SRAM 

    . = ALIGN(4);

    PROVIDE (__etext = .);
    PROVIDE (_etext = .);
    PROVIDE (etext = .);
    PROVIDE( _eilm = . );


    .preinit_array :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >SRAM AT>SRAM 

    .init_array :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
        PROVIDE_HIDDEN (__init_array_end = .);
    } >SRAM AT>SRAM 

    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >SRAM AT>SRAM 

    .ctors :
    {
        /* gcc uses crtbegin.o to find the start of
         * the constructors, so we make sure it is
         * first.  Because this is a wildcard, it
         * doesn't matter if the user does not
         * actually link against crtbegin.o; the
         * linker won't look for a file to match a
         * wildcard.  The wildcard also means that it
         * doesn't matter which directory crtbegin.o
         * is in.  
         */
        KEEP (*crtbegin.o(.ctors))
        KEEP (*crtbegin?.o(.ctors))
        /* We don't want to include the .ctor section from
         * the crtend.o file until after the sorted ctors.
         * The .ctor section from the crtend file contains the
         * end of ctors marker and it must be last
         */
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
    } >SRAM AT>SRAM 

    .dtors :
    {
        KEEP (*crtbegin.o(.dtors))
        KEEP (*crtbegin?.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
    } >SRAM AT>SRAM 

    .ro_data :
    {
        *(.rdata)
        *(.rodata .rodata.*)
    } >SRAM AT>SRAM 
    
    .lalign :
    {
        . = ALIGN(4);        
        PROVIDE( _data_lma = . );
    } >SRAM AT>SRAM 
    
    .dalign :
    {
        . = ALIGN(4);
        PROVIDE( _data = . );
    } >SRAM AT>SRAM 

    .data :
    {
        *(.gnu.linkonce.r.*)
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(8);
        PROVIDE( __global_pointer$ = . + 0x800 );
        *(.sdata .sdata.* .sdata*)
        *(.gnu.linkonce.s.*)
        . = ALIGN(8);
        *(.srodata.cst16)
        *(.srodata.cst8)
        *(.srodata.cst4)
        *(.srodata.cst2)
        *(.srodata .srodata.*)
    } >SRAM AT>SRAM
    
    . = ALIGN(4);
    PROVIDE( _edata = . );
    PROVIDE( edata = . );

    PROVIDE( _fbss = . );
    PROVIDE( __bss_start = . );

    .bss :
    {
        *(.sbss*)
        *(.gnu.linkonce.sb.*)
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
    } >SRAM AT>SRAM 
    
    . = ALIGN(8);
    PROVIDE( __bss_end = . );
    
    .no_init (NOLOAD) :
    {
    	*(.no_init)
        . = ALIGN(4);
    } >SRAM AT>SRAM 

    . = ALIGN(8);
    PROVIDE( _end = . );
    PROVIDE( end = . );

    .stack :
    {
        . = __stack_size;
        PROVIDE( _sp = . );
    } >SRAM AT>SRAM 

    

    .heap_start :
    {
        PROVIDE(heap_start = .);
    } >SRAM AT>SRAM

    .heap_end ORIGIN(SRAM) + LENGTH(SRAM) :
    {
        PROVIDE(heap_end = .);
        PROVIDE(__befor_os_int_sp = .);
    } >SRAM AT>SRAM
}


/*****************************************************************************/
/*                                export symbol                              */
/*****************************************************************************/
/* __FREERTOSHEAP_START = SRAM_START_ADDR + ROM_SIZE; */
/* __FREERTOSHEAP_SIZE  = FHEAP_SIZE; */

START_ADDR                          = SRAM_START_ADDR; 

LOAD_TYPE                           = 0x0;             /*0:ram mode 1:tcm mode 2:cache mode*/
ITCM_EN                             = 0x0;             /*enable itcm*/
STCM_EN                             = 0x0;             /*enable stcm*/
ICACHE_EN                           = 0x0;             /*disable icache*/
ICACHE_START_ADDR                   = 0xFFFFFFFF;      /*invaild addr*/
ICACHE_END_ADDR                     = 0xFFFFFFFF;      /*invaild addr*/
ICACHE_ALIAS_ADDR                   = 0xFFFFFFFF;      /*invaild addr*/
SCACHE_EN                           = 0x0;             /*disable scache*/
SCACHE_START_ADDR                   = 0xFFFFFFFF;      /*invaild addr*/
SCACHE_END_ADDR                     = 0xFFFFFFFF;      /*invaild addr*/
SCACHE_ALIAS_ADDR                   = 0xFFFFFFFF;      /*invaild addr*/



