	.section .vectors
_vectors:
	b _start       /* Reset */
	b _ex_undefins /* Undefined Instruction */
	b .            /* Supervisor Call */
	b _ex_pfabrt   /* Prefetch Abort */
	b _ex_dtabrt   /* Data Abort */
	b .            /* Not Used */
	b _ex_irq      /* IRQ */
	b _ex_fiq      /* FIQ */

_ex_undefins:
	sub lr, lr, #4
	srsfd #0x1b!
	push {r0-r15}
	mov r0, sp
	ldr r1, =0
	bl ArmExceptionHandler
	b .

_ex_pfabrt:
	sub lr, lr, #4
	srsfd #0x17!
	push {r0-r15}
	mov r0, sp
	ldr r1, =1
	bl ArmExceptionHandler
	b .

_ex_dtabrt:
	sub lr, lr, #4
	srsfd #0x17!
	push {r0-r15}
	mov r0, sp
	ldr r1, =2
	bl ArmExceptionHandler
	b .

_ex_irq:
	sub lr, lr, #4
	srsfd #0x12!
	push {r0-r3, r4, r12}
	bl ArmIrqHandler
	pop {r0-r3, r4, r12}
	rfefd sp!

_ex_fiq:
	sub lr, lr, #4
	srsfd #0x12!
	push {r0-r3, r4, r12}
	bl ArmFiqHandler
	pop {r0-r3, r4, r12}
	rfefd sp!

/*=================================================================================== */

	.section .text.startup
	.global _start
_start:
	/* disable interrupts */
	cpsid i
	cpsid f

	/* set SP register */
	cps #0x11
	ldr sp, =__sp_fiq
	cps #0x12
	ldr sp, =__sp_irq
	cps #0x13
	ldr sp, =__sp_svc
	cps #0x17
	ldr sp, =__sp_abort
	cps #0x1b
	ldr sp, =__sp_undef
	cps #0x1f
	ldr sp, =__sp_system
	
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
	
	/* enable interrupts */
	cpsie i
	cpsie f
	
	/* goto main */
	blx main
	
	/* halt */
	b .
