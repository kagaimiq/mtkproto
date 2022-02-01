#ifndef _MTKREGS_H
#define _MTKREGS_H

#include <stdint.h>

static inline uint32_t reg32_read(uint32_t addr) {
	return *(volatile uint32_t*)addr;
}
static inline void reg32_write(uint32_t addr, uint32_t val) {
	*(volatile uint32_t*)addr = val;
}
static inline void reg32_wsmask(uint32_t addr, int shift, uint32_t mask, uint32_t val) {
	*(volatile uint32_t*)addr =
		(*(volatile uint32_t*)addr & ~(mask << shift)) | ((val & mask) << shift);
}
static inline uint32_t reg32_rsmask(uint32_t addr, int shift, uint32_t mask) {
	return (*(volatile uint32_t*)addr >> shift) & mask;
}

//--------- Base addresses ----------//
#define TOPCKGEN_BASE		0x10000000
#define INFRACFG_AO_BASE	0x10001000
#define GPIO_BASE		0x10005000
#define SLEEP_BASE		0x10006000
#define TOPRGU_BASE		0x10007000
#define APXGPT_BASE		0x10008000
#define APMIXED_BASE		0x1000C000
#define PWRAP_BASE		0x1000D000
#define MCUCFG_BASE		0x10200000
#define INFRA_MBIST_BASE	0x1020D000
#define INFRACFG_BASE		0x1020E000
#define SRAMROM_BASE		0x10214000
#define AUXADC_BASE		0x11001000
#define UART0_BASE		0x11002000
#define UART1_BASE		0x11003000
#define USB0_BASE		0x11200000
#define USB_SIF_BASE		0x11210000
#define AUDIO_BASE		0x11220000
#define MSDC0_BASE		0x11230000
#define MSDC1_BASE		0x11240000
#define MMSYS_CONFIG_BASE	0x14000000

//-------------- APXGPT -------------//
#define APXGPT_IRQEN				0x000
#define APXGPT_IRQSTA				0x004
#define APXGPT_IRQACK				0x008
#define APXGPT_CON(n)				(0x010 + (0x10 * (n)))
#define APXGPT_CLK(n)				(0x014 + (0x10 * (n)))
#define APXGPT_CNT(n)				(0x018 + (0x10 * (n)))
#define APXGPT_CMP(n)				(0x01C + (0x10 * (n)))
/* The 6th timer is routed to the ARM Arch Timer !!!!! */

#endif
