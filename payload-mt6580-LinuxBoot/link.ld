MEMORY {
	RAM (rwx): ORIGIN = 0x80001000, LENGTH = 1024M-4k
}

ENTRY(_start);

SECTIONS {
	.vectors : {
		KEEP(*(.vectors))
		. = ALIGN(32);
	} >RAM

	.text : {
		*(.text.startup)
		*(.text)
		*(.text.*)
	} >RAM
	
	.rodata : { 
		*(.rodata)
		*(.rodata.*)
	} >RAM
	
	.data : {
		*(.data)
		*(.data.*)
		*(.sdata)
		*(.sdata.*)
	} >RAM
	
	.bss : {
		_sbss = .;
		*(.sbss)
		*(.sbss.*)
		*(.scommon)
		*(.bss)
		*(.bss.*)
		*(COMMON)
		_ebss = .;
	} >RAM
	
	.stack : {
		/* system stack */
		. = . + 8192;
		. = ALIGN(8);
		__sp_system = .;
		
		/* exceptions stack */
		. = . + 1024;
		. = ALIGN(8);
		__sp_irq = .;
		__sp_fiq = .;
		__sp_svc = .;
		__sp_abort = .;
		__sp_undef = .;
	} >RAM
	
	/*
	 * must be placed after stack and bss to 
	 * not corrupt itself and the data that follows it 
	 */
	.blinfo : {
		KEEP(*(.blinfo))
		. = ALIGN(256);
	} >RAM
	
	end = .;
}
