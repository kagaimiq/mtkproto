MEMORY {
	RAM (rwx): ORIGIN = 0x00200300, LENGTH = 4k - 0x300
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
		. = . + 256;
		. = ALIGN(8);
		__sp_system = .;
		
		/* exceptions stack */
		. = . + 256;
		. = ALIGN(8);
		__sp_irq = .;
		__sp_fiq = .;
		__sp_svc = .;
		__sp_abort = .;
		__sp_undef = .;
	} >RAM
	
	.whatever : {
		*(.comment)
		. = ALIGN(8);
	} >RAM
	
	end = .;
}
