MEMORY {
	RAM (rwx): ORIGIN = 0x00200000, LENGTH = 256k
}

ENTRY(_start);

SECTIONS {
	.text : {
		KEEP(*(.vectors))
		. = ALIGN(32);
		
		*(.text.startup)
		*(.text)
		*(.text.*)
		. = ALIGN(4);
	} >RAM
	
	.rodata : { 
		*(.rodata)
		*(.rodata.*)
		. = ALIGN(4);
	} >RAM
	
	.data : {
		*(.data)
		*(.data.*)
		. = ALIGN(4);
	} >RAM
	
	.bss : {
		_sbss = .;
		*(.bss)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
	} >RAM
	
	.stack : {
		. = . + 65536;
		. = ALIGN(8);
		__system_sp = .;
	} >RAM
	
	end = .;
}
