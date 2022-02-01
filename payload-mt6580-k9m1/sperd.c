#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <xprintf.h>
#include <wallclk.h>
#include <mtk_regs.h>
#include <mtk_gpio.h>
#include <mtk_pmic.h>

void uputc(char c) {
	while ((reg32_read(UART1_BASE+0x014) & 0x60) != 0x60);
	reg32_write(UART1_BASE+0x000, c);
}

char ugetc(void) {
	while ((reg32_read(UART1_BASE+0x014) & 0x01) != 0x01);
	return reg32_read(UART1_BASE+0x000);
}

void *_sbrk(int amount) {
	extern char end;
	static void *heap_top = &end;
	
	void *heap_ptr = heap_top;
	heap_top += amount;
	
	xprintf("sBRK %x %x %d\n", heap_ptr, heap_top, amount);
	
	return heap_ptr;
}


void ArmExceptionHandler(uint32_t *stack, int type) {
	static char *types[3] = {"Undefined Instruction", "Prefetch Abort", "Data Abort"};
	xprintf("Program Dead! %s\n", types[type]);
	xprintf("  r0:<%08x>   r1:<%08x>   r2:<%08x>   r3:<%08x>\n", stack[0],  stack[1],  stack[2],  stack[3] );
	xprintf("  r4:<%08x>   r5:<%08x>   r6:<%08x>   r7:<%08x>\n", stack[4],  stack[5],  stack[6],  stack[7] );
	xprintf("  r8:<%08x>   r9:<%08x>   sl:<%08x>   fp:<%08x>\n", stack[8],  stack[9],  stack[10], stack[11]);
	xprintf("  ip:<%08x>   sp:<%08x>   lr:<%08x>   pc:<%08x>\n", stack[12], stack[13], stack[14], stack[15]);
	xprintf("  lr:<%08x> spsr:<%08x>                        \n", stack[16], stack[17]                      );
}

void ArmIrqHandler(void) {
	xputs("[[ IRQ ]]\n");
	
}

void ArmFiqHandler(void) {
	xputs("[[ FIQ ]]\n");
}


/*
 * From https://github.com/wtarreau/mhz
 *
 * Note that this doesnt calculate the clock frequency right when ran in this enviroment.
 * Under Linux it calculates the correct frequency
 * while there it is ~3 times lower! (1.3 GHz ==> ~426 MHz)
 */

#define microseconds() micros()
#define rdtsc() 0


/* performs read-after-write operations that the CPU is not supposed to be able
 * to parallelize. The "asm" statements are here to prevent the compiler from
 * reordering this code.
 */
#define dont_move(var) do { asm volatile("" : "=r"(var) : "0" (var)); } while (0)

#define run1cycle_ae()   do { a ^= e; dont_move(a); } while (0)
#define run1cycle_ba()   do { b ^= a; dont_move(b); } while (0)
#define run1cycle_cb()   do { c ^= b; dont_move(c); } while (0)
#define run1cycle_dc()   do { d ^= c; dont_move(d); } while (0)
#define run1cycle_ed()   do { e ^= d; dont_move(e); } while (0)
#define run1cycle_eb()   do { e ^= b; dont_move(e); } while (0)

#define run5cycles()                                    \
	do {                                            \
		run1cycle_ae();				\
		run1cycle_ba();				\
		run1cycle_cb();				\
		run1cycle_dc();				\
		run1cycle_ed();				\
	} while (0)

#define run10cycles()          \
	do {                   \
		run5cycles();  \
		run5cycles();  \
	} while (0)

#define run100cycles()          \
	do {                    \
		run10cycles();  \
		run10cycles();  \
		run10cycles();  \
		run10cycles();  \
		run10cycles();  \
		run10cycles();  \
		run10cycles();  \
		run10cycles();  \
		run10cycles();  \
		run10cycles();  \
	} while (0)


/* performs 50 operations in a loop, all dependant on each other, so that the
 * CPU cannot parallelize them, hoping to take 50 cycles per loop, plus the
 * loop counter overhead.
 */
static __attribute__((noinline)) void loop50(unsigned int n)
{
	unsigned int a = 0, b = 0, c = 0, d = 0, e = 0;

	do {
		run10cycles();
		run10cycles();
		run10cycles();
		run10cycles();
		run10cycles();
	} while (__builtin_expect(--n, 1));
}

/* performs 250 operations in a loop, all dependant on each other, so that the
 * CPU cannot parallelize them, hoping to take 250 cycles per loop, plus the
 * loop counter overhead. Do not increase this loop so that it fits in a small
 * 1 kB L1 cache on 32-bit instruction sets.
 */
static __attribute__((noinline)) void loop250(unsigned int n)
{
	unsigned int a = 0, b = 0, c = 0, d = 0, e = 0;

	do {
		run10cycles();
		run10cycles();
		run10cycles();
		run10cycles();
		run10cycles();
		run100cycles();
		run100cycles();
	} while (__builtin_expect(--n, 1));
}

void run_once(long count)
{
	long long tsc_begin, tsc_end50, tsc_end250;
	long long us_begin, us_end50, us_end250;

	/* now run the loop */
	us_begin   = microseconds();
	tsc_begin  = rdtsc();
	loop50(count);
	tsc_end50 = rdtsc() - tsc_begin;
	us_end50  = microseconds() - us_begin;

	/* now run the loop */
	us_begin   = microseconds();
	tsc_begin  = rdtsc();
	loop250(count);
	tsc_end250 = rdtsc() - tsc_begin;
	us_end250  = microseconds() - us_begin;

	xprintf("count=%ld us50=%lld us250=%lld diff=%lld cpu_MHz=%.3f",
	       count, us_end50, us_end250, us_end250 - us_end50,
	       count * 200.0 / (us_end250 - us_end50));
#ifdef HAVE_RDTSC
	xprintf(" tsc50=%lld tsc250=%lld diff=%lld rdtsc_MHz=%.3f",
	       tsc_end50, tsc_end250, (tsc_end250 - tsc_end50) / count,
	       (tsc_end250 - tsc_end50) / (float)(us_end250 - us_end50));
#endif
	xputc('\n');
}

/* spend <delay> ms waiting for the CPU's frequency to raise. Will also stop
 * on backwards time jumps if any.
 */
void pre_heat(long delay)
{
	unsigned long long start = microseconds();

	while (microseconds() - start < (unsigned long long)delay)
		;
}

/* determines how long loop50() must be run to reach more than 20 milliseconds.
 * This will ensure that an integral number of clock ticks will have happened
 * on 100, 250, 1000 Hz systems.
 */
unsigned int calibrate()
{
	unsigned long long duration = 0;
	unsigned long long start;
	unsigned int count = 1000;

	while (duration < 20000) {
		count = count * 5 / 4;
		start = microseconds();
		loop50(count);
		duration = microseconds() - start;
	}
	return count;
}

void mhz(long runs, long preheat)
{
	unsigned int count;
	
	xprintf("---- MHZ runs=%ld, preheat=%ld ----\n", runs, preheat);

	if (preheat > 0)
		pre_heat(preheat);

	count = calibrate();
	while (runs--)
		run_once(count);
}




int main(void) {
	/* setup UART2 (again some 16550A-compatible stuff?? great!!!) */ {
		reg32_write(TOPCKGEN_BASE+0x084, (1<<11)); /* enable uart2 clk */
	
		/* wait for all pending transmissions to end, to avoid garbage characters */
		while ((reg32_read(UART1_BASE+0x014) & 0x60) != 0x60);
	
		/* configure the format and baudrate */
		uint16_t dl = 26000000 / 4 / 115200;
		reg32_write(UART1_BASE+0x024, 2); /* HIGHSPEED = 1 */
		reg32_wsmask(UART1_BASE+0x00C, 7, 1, 1); /* access to DLL/DLH */
		reg32_write(UART1_BASE+0x000, dl);
		reg32_write(UART1_BASE+0x004, dl >> 8);
		reg32_wsmask(UART1_BASE+0x00C, 7, 1, 0); /* regular access */
		
		reg32_write(UART1_BASE+0x00C, 0x03); /* 8n1 */
		
		/* config pins */
		gpio_iomux_cfg(9,  4); /* G9  = URXD2 */
		gpio_iomux_cfg(10, 4); /* G10 = UTXD2 */
	}
	
	xdev_out(uputc); xdev_in(ugetc);
	xputs("\n\e[1;33m*******Hello Mediatek!*******\e[0m\n");
	
	wallclk_init();
	
	mhz(10, 1000000);
	
	/* set vproc to 1.25v */
	pmic_wsmask(0x021e, 0, 0x7f, (1250000 - 700000) / 6250); //1 volt [For less current consumption!]
	
	/* --------- ARM --------- */
	/* sel mainpll */
	reg32_wsmask(0x10001000, 2, 0x3, 0x3);
	
	/* set freq ! => 1300 MHz */
	reg32_write(0x10018104, (1<<31) | (0<<24) | ((1300 * 8192 / 13)));
	usleep(1000);
	
	/* sel armpll * 4 / 4 */
	reg32_wsmask(0x10001008, 0, 0x1f, 0x08); //325 MHz [For less current consumption!]
	reg32_wsmask(0x10001000, 2, 0x3,  0x1);
	
	mhz(10, 1000000);
	
	return 0;
}
