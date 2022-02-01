#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <xprintf.h>
#include <wallclk.h>
#include <mtk_regs.h>


void uputc(int c) {
	while ((reg32_read(0x11002014) & 0x60) != 0x60);
	reg32_write(0x11002000, c);
}

int ugetc(void) {
	while ((reg32_read(0x11002014) & 0x01) != 0x01);
	return reg32_read(0x11002000);
}


void hexdump(void *ptr, int len) {
	for (int i = 0; i < len; i += 16) {
		xprintf("%08x: ", ptr+i);
		
		for (int j = 0; j < 16; j++) {
			if (i+j < len)
				xprintf("%02X ", *(uint8_t*)(ptr+i+j));
			else
				xputs("-- ");
		}
		
		xputs(" |");
		
		for (int j = 0; j < 16; j++) {
			uint8_t c = ' ';
			if (i+j < len) {
				c = *(uint8_t*)(ptr+i+j);
				if (c < 0x20 || c >= 0x7f) c = '.';
			}
			xputc(c);
		}
		
		xputs("|\n");
	}
}

void ArmExceptionHandler(uint32_t *stack, int type) {
	static char *types[3] = {"Undefined Instruction", "Prefetch Abort", "Data Abort"};
	xprintf("Program Dead! %s\n", types[type]);
	xprintf("  r0:<%08x>   r1:<%08x>   r2:<%08x>   r3:<%08x>\n", stack[0],  stack[1],  stack[2],  stack[3] );
	xprintf("  r4:<%08x>   r5:<%08x>   r6:<%08x>   r7:<%08x>\n", stack[4],  stack[5],  stack[6],  stack[7] );
	xprintf("  r8:<%08x>   r9:<%08x>   sl:<%08x>   fp:<%08x>\n", stack[8],  stack[9],  stack[10], stack[11]);
	xprintf("  ip:<%08x>   sp:<%08x>   lr:<%08x>   pc:<%08x>\n", stack[12], stack[13], stack[14], stack[15]);
	xprintf("  lr:<%08x> spsr:<%08x>                        \n", stack[16], stack[17]                      );
	
	xputs("=== press any key to try again ===\n");
	ugetc();
}


void main2(void) {
	uint32_t tmp1, tmp2;
	asm volatile ("mrc p15, 0, %0, c0, c0, 5" :: "r"(tmp1));
	asm volatile ("mrc p15, 0, %0, c0, c0, 0" :: "r"(tmp2));
	
	xprintf("Who i am? CPUID->%08x, IDCode->%08x!\n", tmp1, tmp2);
}


void gpio_iomux_cfg(int pad, int mode) {
	if (pad < 0 || pad > 179) return;
	reg32_wsmask(GPIO_BASE + 0x300 + ((pad / 8) * 0x10), (pad % 8) * 4, 0xf, mode);
}

void gpio_set_direction(int pad, bool output) {
	if (pad < 0 || pad > 179) return;
	reg32_wsmask(GPIO_BASE + 0x000 + ((pad / 32) * 0x10), pad % 32, 1, output);
}

void gpio_set_level(int pad, bool level) {
	if (pad < 0 || pad > 179) return;
	reg32_wsmask(GPIO_BASE + 0x100 + ((pad / 32) * 0x10), pad % 32, 1, level);
}

bool gpio_get_level(int pad) {
	if (pad < 0 || pad > 179) return false;
	return reg32_rsmask(GPIO_BASE + 0x200 + ((pad / 32) * 0x10), pad % 32, 1);
}

//=================================//

void InitPLL(void) {
	reg32_wsmask(APMIXED_BASE+0x000, 0, 1, 1);
	usleep(100);
	reg32_wsmask(APMIXED_BASE+0x000, 1, 1, 1);
	
	reg32_wsmask(APMIXED_BASE+0x218, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x228, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x238, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x258, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x248, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x278, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x288, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x268, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x29C, 0, 1, 1);
	
	usleep(30);
	
	reg32_wsmask(APMIXED_BASE+0x218, 1, 1, 0);
	reg32_wsmask(APMIXED_BASE+0x228, 1, 1, 0);
	reg32_wsmask(APMIXED_BASE+0x238, 1, 1, 0);
	reg32_wsmask(APMIXED_BASE+0x258, 1, 1, 0);
	reg32_wsmask(APMIXED_BASE+0x248, 1, 1, 0);
	reg32_wsmask(APMIXED_BASE+0x278, 1, 1, 0);
	reg32_wsmask(APMIXED_BASE+0x288, 1, 1, 0);
	reg32_wsmask(APMIXED_BASE+0x268, 1, 1, 0);
	reg32_wsmask(APMIXED_BASE+0x29C, 1, 1, 0);
	
	usleep(1);
	
	/* ARMPLL */
	reg32_write(APMIXED_BASE+0x210, 0x8111B13B);
	/* ARMPLL_L */
	reg32_write(APMIXED_BASE+0x220, 0x811CD89D);
	/* CCIPLL */
	reg32_write(APMIXED_BASE+0x230, 0x821713B1);
	/* MFGPLL */
	reg32_write(APMIXED_BASE+0x250, 0x8211B13B);
	/* MAINPLL */
	reg32_write(APMIXED_BASE+0x240, 0x81150000);
	/* UNIV2PLL */
	reg32_write(APMIXED_BASE+0x270, 0x80180000);
	/* MSDCPLL */
	reg32_write(APMIXED_BASE+0x280, 0x820EC4EC);
	/* MMPLL */
	reg32_write(APMIXED_BASE+0x260, 0x821193B1);
	/* APLL1 */
	reg32_write(APMIXED_BASE+0x290, 0x83000000);
	reg32_write(APMIXED_BASE+0x294, 0x3C7EA932);
	
	reg32_wsmask(APMIXED_BASE+0x20C, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x21C, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x22C, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x24C, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x26C, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x27C, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x25C, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x28C, 0, 1, 1);
	
	reg32_wsmask(APMIXED_BASE+0x23C, 24, 0xd4, 0xd4);
	reg32_wsmask(APMIXED_BASE+0x26C, 24, 0xd4, 0xd4);
	
	usleep(20);
	
	reg32_wsmask(APMIXED_BASE+0x23C, 23, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x26C, 23, 1, 1);
	
	usleep(20);
	
	reg32_wsmask(APMIXED_BASE+0x048, 22, 1, 0);
	
	reg32_wsmask(MCUCFG_BASE+0x7A0, 9, 3, 1);
	reg32_wsmask(MCUCFG_BASE+0x7A4, 9, 3, 1);
	reg32_wsmask(MCUCFG_BASE+0x7C0, 9, 3, 1);
	
	reg32_wsmask(INFRACFG_AO_BASE+0x070, 20, 3, 3);
	
	reg32_write(TOPCKGEN_BASE+0x040, 0x00000001);
	reg32_write(TOPCKGEN_BASE+0x004, 0x00000001);
	reg32_write(TOPCKGEN_BASE+0x040, 0x00060101);
	reg32_write(TOPCKGEN_BASE+0x050, 0x01010201);
	reg32_write(TOPCKGEN_BASE+0x060, 0x01000101);
	reg32_write(TOPCKGEN_BASE+0x070, 0x00010101);
	reg32_write(TOPCKGEN_BASE+0x080, 0x00030101);
	reg32_write(TOPCKGEN_BASE+0x090, 0x01010201);
	reg32_write(TOPCKGEN_BASE+0x0A0, 0x01030202);
	reg32_write(TOPCKGEN_BASE+0x0B0, 0x00000300);
	reg32_write(TOPCKGEN_BASE+0x004, 0xFFFFFFFC);
	
	reg32_wsmask(INFRACFG_AO_BASE+0x200, 31, 1, 0);
	reg32_wsmask(INFRACFG_AO_BASE+0x074, 2, 1, 0);
	reg32_write(INFRACFG_AO_BASE+0x084, 0x98FFFF7F);
	reg32_write(INFRACFG_AO_BASE+0x08C, 0x879C7D96);
	reg32_write(INFRACFG_AO_BASE+0x088, 0x18000200);
	reg32_write(INFRACFG_AO_BASE+0x0A8, 0x2FFC87DD);
	reg32_write(INFRACFG_AO_BASE+0x0C4, 0x00038FFF);
	
	reg32_wsmask(APMIXED_BASE+0x25C, 0, 1, 0);
	
	reg32_wsmask(APMIXED_BASE+0x268, 1, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x268, 0, 1, 0);
	
	reg32_wsmask(APMIXED_BASE+0x28C, 0, 1, 0);
	
	reg32_wsmask(APMIXED_BASE+0x29C, 1, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x29C, 0, 1, 0);
}


void InitPMIC(void) {
	
}

void InitDRAM(void) {
	
}



int main(void) {
	xdev_out(uputc); xdev_in(ugetc);
	xputs("\n\e[1;40;33;7m <<    Mediatek MT6762V 32-bits    >> \e[0m\n");
	
	wallclk_init();
	
	{
		uint32_t tmp;
		asm volatile("mov %0, pc" :: "r"(tmp));
		xprintf("We are at %08x!\n", tmp);
	}
	
	xputs("======================================\n");
	
	InitPLL();
	InitPMIC();
	InitDRAM();
	
	hexdump((void *)0x40000000, 0x1000);
	
	extern void testsmc(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3);
	extern char _start;
	testsmc(0x82000115, (uint32_t)&_start, 0, 0);
	
	for (bool x = false;; x = !x) {
		xputs(x?"it wont work\n":"what you are doing\n");
		delay(1000);
	}
	
	return 0;
}
