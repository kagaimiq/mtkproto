MEMORY {
	/* RAM (rwx): ORIGIN = 0x40200000, LENGTH = 1024M-0x200000 */
	RAM (rwx): ORIGIN = 0x200000, LENGTH = 256k
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
		. = . + 16384;
		. = ALIGN(8);
		__sp_system = .;
		
		/* exceptions stack */
		. = . + 4096;
		. = ALIGN(8);
		__sp_irq = .;
		__sp_fiq = .;
		__sp_svc = .;
		__sp_abort = .;
		__sp_undef = .;
		__sp_system2 = .;
	} >RAM
	
	end = .;
}
