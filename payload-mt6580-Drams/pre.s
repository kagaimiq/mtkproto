	.section .vectors
_vectors:
	b _start       /* Reset */
	b _ex_undefins /* Undefined Instruction */
	b .            /* Supervisor Call */
	b _ex_pfabrt   /* Prefetch Abort */
	b _ex_dtabrt   /* Data Abort */
	b .            /* Not Used */
	b .            /* IRQ */
	b .            /* FIQ */

_ex_undefins:
	ldr r0, =0x11006000
	ldr r1, ='U'
	str r1, [r0, #0x0]
	ldr r1, ='N'
	str r1, [r0, #0x0]
	ldr r1, ='D'
	str r1, [r0, #0x0]
	ldr r1, ='E'
	str r1, [r0, #0x0]
	ldr r1, ='F'
	str r1, [r0, #0x0]
	ldr r1, ='_'
	str r1, [r0, #0x0]
	ldr r1, ='I'
	str r1, [r0, #0x0]
	ldr r1, ='N'
	str r1, [r0, #0x0]
	ldr r1, ='S'
	str r1, [r0, #0x0]
	ldr r1, ='\r'
	str r1, [r0, #0x0]
	ldr r1, ='\n'
	str r1, [r0, #0x0]
	b .

_ex_pfabrt:
	ldr r0, =0x11006000
	ldr r1, ='P'
	str r1, [r0, #0x0]
	ldr r1, ='F'
	str r1, [r0, #0x0]
	ldr r1, ='_'
	str r1, [r0, #0x0]
	ldr r1, ='A'
	str r1, [r0, #0x0]
	ldr r1, ='B'
	str r1, [r0, #0x0]
	ldr r1, ='R'
	str r1, [r0, #0x0]
	ldr r1, ='T'
	str r1, [r0, #0x0]
	ldr r1, ='\r'
	str r1, [r0, #0x0]
	ldr r1, ='\n'
	str r1, [r0, #0x0]
	b .

_ex_dtabrt:
	ldr r0, =0x11006000
	ldr r1, ='D'
	str r1, [r0, #0x0]
	ldr r1, ='T'
	str r1, [r0, #0x0]
	ldr r1, ='_'
	str r1, [r0, #0x0]
	ldr r1, ='A'
	str r1, [r0, #0x0]
	ldr r1, ='B'
	str r1, [r0, #0x0]
	ldr r1, ='R'
	str r1, [r0, #0x0]
	ldr r1, ='T'
	str r1, [r0, #0x0]
	ldr r1, ='\r'
	str r1, [r0, #0x0]
	ldr r1, ='\n'
	str r1, [r0, #0x0]
	b .

	.section .text.startup
	.global _start
_start:
	/* set SP register */
	ldr sp, =__system_sp
	
	/* set the Vector Base Address Register */
	ldr r0, =_vectors
	mcr p15, 0, r0, c12, c0, 0
	
	/* set the vector base to VBAR instead of 0xFFFF0000 */
	mrc p15, 0, r0, c1, c0, 0
	ldr r1, =~(1<<13)
	and r0, r1
	mcr p15, 0, r0, c1, c0, 0
	
	/* clear bss */
	ldr r0, =_sbss
	ldr r1, =#0
	ldr r2, =_ebss
	sub r2, r0
	blx memset
	
	/* goto main */
	blx main
	
	/* halt */
	b .
