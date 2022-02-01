#include <stdint.h>
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


void main2(void) {
	uint32_t tmp;
	asm volatile ("mrc p15, 0, %0, c0, c0, 5" :: "r"(tmp));
	
	xprintf("Who i am? %08x!\n", tmp);
}


int main(void) {
	xdev_out(uputc); xdev_in(ugetc);
	xputs("\e[1;40;33;7m <<    Mediatek MT6762V 32-bits    >> \e[0m\n");
	
	wallclk_init();
	
	{
		uint32_t tmp;
		asm volatile("mov %0, pc" :: "r"(tmp));
		xprintf("We are at %08x!\n", tmp);
	}
	
	xputs("======================================\n");
	
	/*{
		reg32_write(INFRA_MBIST_BASE+0x00C, 0x06C4E4F3);
		xprintf("SRAM Desel => %08x\n", reg32_read(INFRA_MBIST_BASE+0x00C));
		
		
		
	}
	
	hexdump((void *)0x40000000, 256);*/
	
	#if 0
	{
		/* Unlock? */
		reg32_write(SLEEP_BASE+0x000, (0xb16<<16)|(1<<0));
		
		xprintf("1.1---%08x\n", reg32_read(SLEEP_BASE+0x208));
		xprintf("1.2---%08x\n", reg32_read(SLEEP_BASE+0x20C));
		xprintf("1.3---%08x\n", reg32_read(SLEEP_BASE+0x210));
		xprintf("1.4---%08x\n", reg32_read(SLEEP_BASE+0x214));
		xprintf("2.1---%08x\n", reg32_read(SLEEP_BASE+0x21C));
		xprintf("2.2---%08x\n", reg32_read(SLEEP_BASE+0x220));
		xprintf("2.3---%08x\n", reg32_read(SLEEP_BASE+0x224));
		xprintf("2.4---%08x\n", reg32_read(SLEEP_BASE+0x228));
		
		/* CPU3 */
		reg32_wsmask(SLEEP_BASE+0x214, 2, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x214, 3, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x214, 1, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x254, 0, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x214, 6, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x214, 5, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x214, 4, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x214, 0, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x214, 8, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x214, 7, 1, 0);
		
		/* CPU4 */
		reg32_wsmask(SLEEP_BASE+0x21C, 2, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x21C, 3, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x21C, 1, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x260, 0, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x21C, 6, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x21C, 5, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x21C, 4, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x21C, 0, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x21C, 8, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x21C, 7, 1, 0);
		
		xprintf("1.1---%08x\n", reg32_read(SLEEP_BASE+0x208));
		xprintf("1.2---%08x\n", reg32_read(SLEEP_BASE+0x20C));
		xprintf("1.3---%08x\n", reg32_read(SLEEP_BASE+0x210));
		xprintf("1.4---%08x\n", reg32_read(SLEEP_BASE+0x214));
		xprintf("2.1---%08x\n", reg32_read(SLEEP_BASE+0x21C));
		xprintf("2.2---%08x\n", reg32_read(SLEEP_BASE+0x220));
		xprintf("2.3---%08x\n", reg32_read(SLEEP_BASE+0x224));
		xprintf("2.4---%08x\n", reg32_read(SLEEP_BASE+0x228));
		
		main2();
		reg32_write(SLEEP_BASE+0x8c0, 0xffffffff);
		xprintf("PCTc---%08x\n", reg32_read(SLEEP_BASE+0x8C0));
		xprintf("PCT0---%08x\n", reg32_read(SLEEP_BASE+0x8C4));
		xprintf("PCT1---%08x\n", reg32_read(SLEEP_BASE+0x8C8));
		xprintf("PCT2---%08x\n", reg32_read(SLEEP_BASE+0x8CC));
		xprintf("PCT3---%08x\n", reg32_read(SLEEP_BASE+0x8D0));
		xprintf("PCT4---%08x\n", reg32_read(SLEEP_BASE+0x8D4));
		xprintf("PCT5---%08x\n", reg32_read(SLEEP_BASE+0x8D8));
		xprintf("PCT6---%08x\n", reg32_read(SLEEP_BASE+0x8DC));
		xprintf("PCT7---%08x\n", reg32_read(SLEEP_BASE+0x8E0));
		
		extern char _start2;
		reg32_write(SRAMROM_BASE+0x034, (uint32_t)&_start2);
		xprintf("Addr is %x!\n", reg32_read(SRAMROM_BASE+0x034));
		
		delay(1000);
		xputc('3');
		reg32_write(SRAMROM_BASE+0x038, 0x41534c33);
		asm volatile ("sev");
		
		delay(1000);
		xputc('4');
		reg32_write(SRAMROM_BASE+0x038, 0x534c4134);
		asm volatile ("sev");
	}
	#endif
	
	long addr;
	
	for (;;) {
		xprintf("=> ");
		
		char str[256];
		xgets(str, sizeof(str));
		
		char *sstr = str;
		char c;
		
		switch ((c = *sstr++)) {
		case 'm':
			switch ((c = *sstr++)) {
			case 'a': /* set/get address */
				xatoi(&sstr, &addr);
				xprintf("Addr is %08x\n", addr);
				break;
			
			case 'r': /* read memory address */
				{
					if (!xatoi(&sstr, &addr)) break;
					xprintf("%08x: %08x\n", addr, reg32_read(addr));
				}
				break;
			
			case 'w': /* write memory address */
				{
					long val;
					if (!xatoi(&sstr, &addr)) break;
					if (!xatoi(&sstr, &val)) break;
					reg32_write(addr, val);
					xprintf("%08x: %08x\n", addr, reg32_read(addr));
				}
				break;
			
			case 'm': /* edit memory with auto incrementign address */
			case 'n': /* edit memory with the same address */
				xatoi(&sstr, &addr);
				
				for (;;) {
					xprintf("%08x: %08x ? ", addr, reg32_read(addr));
					
					xgets(str, sizeof(str));
					if (str[0] != 0) {
						char *xsstr = str; long val;
						if (!xatoi(&xsstr, &val)) break;
						reg32_write(addr, val);
					}
					
					if (c == 'm') addr += 4;
				}
				
				break;
			}
			break;
		}
	}
	
	return 0;
}
