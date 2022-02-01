#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "xprintf.h"
#include "mtkregs.h"

/* uart */
void uputc(char c) {
	while ((reg32_read(UART1_BASE+0x014) & 0x60) != 0x60);
	reg32_write(UART1_BASE+0x000, c);
}

char ugetc(void) {
	while ((reg32_read(UART1_BASE+0x014) & 0x01) != 0x01);
	return reg32_read(UART1_BASE+0x000);
}

/* wall clock */
uint64_t micros() {
	/* TODO: handle wraparounds */
	return reg32_read(APXGPT_BASE+APXGPT_CNT(3));
}

void usleep(uint64_t us) {
	for (uint64_t target = micros() + us; micros() < target; );
}

/* etc utils */
void hexdump(const void *ptr, int len) {
	for (int i = 0; i < len; i += 16) {
		put_dump(ptr + i, (uint32_t)ptr + i, 16, 1);
	}
}

/* gpio */
void gpio_iomux_cfg(int pad, int mode) {
	if (pad < 0 || pad > 96) return; /* sanity check */
	reg32_wsmask(GPIO_BASE+GPIO_MODEn(pad/8), (pad%8)*4, 0xf, mode);
}

void gpio_set_direction(int pad, bool output) {
	if (pad < 0 || pad > 96) return; /* sanity check */
	reg32_wsmask(GPIO_BASE+GPIO_DIRn(pad/32), pad%32, 1, output);
}

void gpio_set_level(int pad, bool level) {
	if (pad < 0 || pad > 96) return; /* sanity check */
	reg32_wsmask(GPIO_BASE+GPIO_OUTn(pad/32), pad%32, 1, level);
}

bool gpio_get_level(int pad) {
	if (pad < 0 || pad > 96) return false; /* sanity check */
	return reg32_rsmask(GPIO_BASE+GPIO_INn(pad/32), pad%32, 1);
}




int EmiDramc_PDrive, EmiDramc_NDrive;

void EmiDramcExtdnCalib(void) {
	uint32_t tmp;
	
	/* ~~~~~~~~~~~~~ Enable P-Drive ~~~~~~~~~~~~~ */
	tmp  = reg32_read(DRAMC0_BASE+0x644);
	tmp |= reg32_read(DDRPHY_BASE+0x644);
	tmp |= reg32_read(DRAMC_NAO_BASE+0x644);
	tmp |= (1<<9);
	reg32_write(DRAMC0_BASE+0x644, tmp);
	reg32_write(DDRPHY_BASE+0x644, tmp);
	reg32_write(DRAMC_NAO_BASE+0x644, tmp);
	
	usleep(1);
	
	for (EmiDramc_PDrive = 0; EmiDramc_PDrive < 16; EmiDramc_PDrive++) {
		/* set P-Drive [DRVP] */
		tmp  = reg32_read(DRAMC0_BASE+0x0C0);
		tmp |= reg32_read(DDRPHY_BASE+0x0C0);
		tmp |= reg32_read(DRAMC_NAO_BASE+0x0C0);
		tmp = (tmp & ~(0xf << 12)) | (EmiDramc_PDrive << 12);
		reg32_write(DRAMC0_BASE+0x0C0, tmp);
		reg32_write(DDRPHY_BASE+0x0C0, tmp);
		reg32_write(DRAMC_NAO_BASE+0x0C0, tmp);
		
		usleep(1);
		
		/* check PDrive status [CMPOP] */
		tmp  = reg32_read(DRAMC0_BASE+0x3DC);
		tmp |= reg32_read(DDRPHY_BASE+0x3DC);
		tmp |= reg32_read(DRAMC_NAO_BASE+0x3DC);
		if (tmp & (1<<31)) break;
	}
	
	if (EmiDramc_PDrive >= 16) {
		EmiDramc_PDrive = 10;
		xputs("[DRAMC EXTDN Calib] Invalid P-Drive, set to 10!\n");
	}
	
	/* ~~~~~~~~~~~~~ Enable N-Drive ~~~~~~~~~~~~~ */
	tmp  = reg32_read(DRAMC0_BASE+0x644);
	tmp |= reg32_read(DDRPHY_BASE+0x644);
	tmp |= reg32_read(DRAMC_NAO_BASE+0x644);
	tmp |= (1<<8);
	reg32_write(DRAMC0_BASE+0x644, tmp);
	reg32_write(DDRPHY_BASE+0x644, tmp);
	reg32_write(DRAMC_NAO_BASE+0x644, tmp);
	
	usleep(1);
	
	for (EmiDramc_NDrive = 0; EmiDramc_NDrive < 16; EmiDramc_NDrive++) {
		/* set P-Drive [DRVN] */
		tmp  = reg32_read(DRAMC0_BASE+0x0C0);
		tmp |= reg32_read(DDRPHY_BASE+0x0C0);
		tmp |= reg32_read(DRAMC_NAO_BASE+0x0C0);
		tmp = (tmp & ~(0xf << 8)) | (EmiDramc_NDrive << 8);
		reg32_write(DRAMC0_BASE+0x0C0, tmp);
		reg32_write(DDRPHY_BASE+0x0C0, tmp);
		reg32_write(DRAMC_NAO_BASE+0x0C0, tmp);
		
		usleep(1);
		
		/* check PDrive status [CMPON] */
		tmp  = reg32_read(DRAMC0_BASE+0x3DC);
		tmp |= reg32_read(DDRPHY_BASE+0x3DC);
		tmp |= reg32_read(DRAMC_NAO_BASE+0x3DC);
		if (tmp & (1<<30)) break;
	}
	
	if (EmiDramc_NDrive >= 16) {
		EmiDramc_NDrive = 10;
		xputs("[DRAMC EXTDN Calib] Invalid N-Drive, set to 10!\n");
	}
	
	xprintf("[DRAMC EXTDN Calib] Pdrive=%d, Ndrive=%d\n", EmiDramc_PDrive, EmiDramc_NDrive);
}

void EmiInitLPDDR2(const uint32_t *EmiData) {
	xprintf("=====> EMI Init LPDDR2 [%x]\n", EmiData);
	
	/* TODO */
}

void EmiInitLPDDR3(const uint32_t *EmiData) {
	xprintf("=====> EMI Init LPDDR3 [%x]\n", EmiData);
	uint32_t tmp;
	
	reg32_write(EMI_BASE+0x000, EmiData[10]);
	
	tmp = 0;
	if (EmiData[27] == 0x30000000) tmp |= 0x3<<16; /* 768 MiB rank0 */
	if (EmiData[27] == 0x60000000) tmp |= 0x6<<16; /* 1.5 GiB rank0 */
	if (EmiData[28] == 0x30000000) tmp |= 0x3<<24; /* 768 MiB rank1 */
	if (EmiData[28] == 0x60000000) tmp |= 0x6<<24; /* 1.5 GiB rank1 */
	reg32_write(EMI_BASE+0x038, tmp);
	
	reg32_write(EMI_BASE+0x100, 0x7f07704a);
	
	reg32_write(EMI_BASE+0x108, 0xa0a070db);
	reg32_write(EMI_BASE+0x110, 0xa0a070db - 0x99);
	
	reg32_write(EMI_BASE+0x118, 0x07007047);
	reg32_write(EMI_BASE+0x120, 0x07007047 + 0x192ff004);
	
	reg32_write(EMI_BASE+0x128, 0xa0a07046);
	reg32_write(EMI_BASE+0x134, 0xa0a07046);
	
	reg32_write(EMI_BASE+0x008, 0x0d1e293a);
	
	reg32_write(EMI_BASE+0x010, 0x09190819);
	reg32_write(EMI_BASE+0x030, 0x09190819 + 0x22122015);
	reg32_write(EMI_BASE+0x018, 0x09190819 + 0x2d3e5061);
	
	reg32_write(EMI_BASE+0x020, 0xffff0848);
	
	reg32_write(EMI_BASE+0x078, 0x34220e17);
	
	reg32_write(EMI_BASE+0x0D0, 0xcccccccc);
	reg32_write(EMI_BASE+0x0D8, 0xcccccccc);
	
	reg32_write(EMI_BASE+0x0E8, 0x00020027);
	
	reg32_write(EMI_BASE+0x0F0, 0x38460000);
	
	reg32_write(EMI_BASE+0x0F8, 0x00000000);
	
	reg32_write(EMI_BASE+0x140, 0x20406188);
	reg32_write(EMI_BASE+0x144, 0x20406188);
	reg32_write(EMI_BASE+0x148, 0x20406188 + 0x76d8f7d6u);
	reg32_write(EMI_BASE+0x14c, 0x20406188 + 0x76d8f7d6u);
	
	reg32_write(EMI_BASE+0x150, 0x64f3fc79);
	reg32_write(EMI_BASE+0x154, 0x64f3fc79);
	
	reg32_write(EMI_BASE+0x158, 0xff01ff00);
	
	reg32_write(EMI_BASE+0x028, 0x00421000);
	
	reg32_write(EMI_BASE+0x060, 0x000006ff);
	
	reg32_write(DRAMC0_BASE+0x00C, 0x00000000);
	reg32_write(DDRPHY_BASE+0x00C, 0x00000000);
	
	tmp = (0x10001000 * EmiDramc_PDrive) + (0x01000100 * EmiDramc_NDrive);
	reg32_write(DRAMC0_BASE+0x0B4, tmp);
	reg32_write(DDRPHY_BASE+0x0B4, tmp);
	reg32_write(DRAMC0_BASE+0x0B8, tmp);
	reg32_write(DDRPHY_BASE+0x0B8, tmp);
	reg32_write(DRAMC0_BASE+0x0BC, tmp);
	reg32_write(DDRPHY_BASE+0x0BC, tmp);
	
	reg32_wsmask(DRAMC0_BASE+0x644, 0, 1, 0);
	reg32_wsmask(DDRPHY_BASE+0x644, 0, 1, 0);
	
	reg32_write(DRAMC0_BASE+0x048, 0x0001110d);
	reg32_write(DDRPHY_BASE+0x048, 0x0001110d);
	
	reg32_write(DRAMC0_BASE+0x0D8, 0x00500900);
	reg32_write(DDRPHY_BASE+0x0D8, 0x00500900);
	
	reg32_write(DRAMC0_BASE+0x0E4, 0x00000000);
	reg32_write(DDRPHY_BASE+0x0E4, 0x00000000);
	
	reg32_write(DRAMC0_BASE+0x08C, 0x00000001);
	reg32_write(DDRPHY_BASE+0x08C, 0x00000001);
	
	reg32_write(DRAMC0_BASE+0x090, 0x00000000);
	reg32_write(DDRPHY_BASE+0x090, 0x00000000);
	
	reg32_write(DRAMC0_BASE+0x094, 0x80000000);
	reg32_write(DDRPHY_BASE+0x094, 0x80000000);
	
	reg32_write(DRAMC0_BASE+0x0DC, 0x83004004);
	reg32_write(DDRPHY_BASE+0x0DC, 0x83004004);
	reg32_write(DRAMC0_BASE+0x0E0, 0x83004004 - 0x68000000);
	reg32_write(DDRPHY_BASE+0x0E0, 0x83004004 - 0x68000000);
	
	reg32_write(DRAMC0_BASE+0x124, 0xaa080033);
	reg32_write(DDRPHY_BASE+0x124, 0xaa080033);
	
	reg32_write(DRAMC0_BASE+0x0F0, 0x40000000);
	reg32_write(DDRPHY_BASE+0x0F0, 0x40000000);
	
	reg32_write(DRAMC0_BASE+0x0F4, EmiData[14]);
	reg32_write(DDRPHY_BASE+0x0F4, EmiData[14]);
	
	reg32_wsmask(DRAMC0_BASE+0x0F4, 28, 1, 1);
	reg32_wsmask(DDRPHY_BASE+0x0F4, 28, 1, 1);
	
	reg32_wsmask(DRAMC0_BASE+0x0F4, 20, 0xf, 0xf);
	reg32_wsmask(DDRPHY_BASE+0x0F4, 20, 0xf, 0xf);
	
	reg32_write(DRAMC0_BASE+0x168, 0x00000080);
	reg32_write(DDRPHY_BASE+0x168, 0x00000080);
	
	reg32_write(DRAMC0_BASE+0x0D8, 0x00700900);
	reg32_write(DDRPHY_BASE+0x0D8, 0x00700900);
	
	reg32_write(DRAMC0_BASE+0x028, 0xf1200f01);
	reg32_write(DDRPHY_BASE+0x028, 0xf1200f01);
	
	reg32_write(DRAMC0_BASE+0x1E0, 0x0110017f);
	reg32_write(DDRPHY_BASE+0x1E0, 0x0110017f);
	
	reg32_write(DRAMC0_BASE+0x1E8, EmiData[24]);
	reg32_write(DDRPHY_BASE+0x1E8, EmiData[24]);
	
	reg32_write(DRAMC0_BASE+0x158, 0xf0f0f0f0);
	reg32_write(DDRPHY_BASE+0x158, 0xf0f0f0f0);
	
	reg32_write(DRAMC0_BASE+0x400, 0x00111100);
	reg32_write(DDRPHY_BASE+0x400, 0x00111100);
	
	reg32_write(DRAMC0_BASE+0x404, 0x00000002);
	reg32_write(DDRPHY_BASE+0x404, 0x00000002);
	
	reg32_write(DRAMC0_BASE+0x408, 0x00222222);
	reg32_write(DDRPHY_BASE+0x408, 0x00222222);
	
	reg32_write(DRAMC0_BASE+0x40C, 0x33330000);
	reg32_write(DDRPHY_BASE+0x40C, 0x33330000);
	reg32_write(DRAMC0_BASE+0x410, 0x33330000);
	reg32_write(DDRPHY_BASE+0x410, 0x33330000);
	
	reg32_write(DRAMC0_BASE+0x110, 0x0b052311);
	reg32_write(DDRPHY_BASE+0x110, 0x0b052311);
	
	reg32_write(DRAMC0_BASE+0x0E4, 0x00000005);
	reg32_write(DDRPHY_BASE+0x0E4, 0x00000005);
	
	asm volatile ("dsb" ::: "memory");
	usleep(200);
	
	reg32_write(DRAMC0_BASE+0x088, EmiData[46]);
	reg32_write(DDRPHY_BASE+0x088, EmiData[46]);
	
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000001);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000001);
	
	asm volatile ("dsb" ::: "memory");
	usleep(10);
	
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000000);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000000);
	
	reg32_wsmask(DRAMC0_BASE+0x110, 0, 0x7, 0x0);
	reg32_wsmask(DDRPHY_BASE+0x110, 0, 0x7, 0x0);
	
	reg32_write(DRAMC0_BASE+0x088, EmiData[45]);
	reg32_write(DDRPHY_BASE+0x088, EmiData[45]);
	
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000001);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000001);
	
	asm volatile ("dsb" ::: "memory");
	usleep(1);
	
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000000);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000000);
	
	if (reg32_rsmask(EMI_BASE+0x000, 17, 1)) {
		/* We have Dual Rank */
		reg32_wsmask(DRAMC0_BASE+0x110, 3, 1, 1);
		reg32_wsmask(DDRPHY_BASE+0x110, 3, 1, 1);
		
		reg32_write(DRAMC0_BASE+0x088, EmiData[45]);
		reg32_write(DDRPHY_BASE+0x088, EmiData[45]);
		
		reg32_write(DRAMC0_BASE+0x1E4, 0x00000001);
		reg32_write(DDRPHY_BASE+0x1E4, 0x00000001);
		
		asm volatile ("dsb" ::: "memory");
		usleep(1);
		
		reg32_write(DRAMC0_BASE+0x1E4, 0x00000000);
		reg32_write(DDRPHY_BASE+0x1E4, 0x00000000);
		
		reg32_wsmask(DRAMC0_BASE+0x110, 3, 1, 0);
		reg32_wsmask(DDRPHY_BASE+0x110, 3, 1, 0);
		
		reg32_wsmask(DRAMC0_BASE+0x110, 3, 1, 1);
		reg32_wsmask(DDRPHY_BASE+0x110, 3, 1, 1);
	}
	
	reg32_write(DRAMC0_BASE+0x088, EmiData[41]);
	reg32_write(DDRPHY_BASE+0x088, EmiData[41]);
	
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000001);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000001);
	
	asm volatile ("dsb" ::: "memory");
	usleep(1);
	
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000000);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000000);
	
	reg32_write(DRAMC0_BASE+0x088, EmiData[42]);
	reg32_write(DDRPHY_BASE+0x088, EmiData[42]);
	
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000001);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000001);
	
	asm volatile ("dsb" ::: "memory");
	usleep(1);
	
	reg32_write(DRAMC0_BASE+0x1E4, 0x00001100);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00001100);
	
	tmp = 0x00112390 | reg32_rsmask(EMI_BASE+0x000, 17, 1);
	reg32_write(DRAMC0_BASE+0x110, tmp);
	reg32_write(DDRPHY_BASE+0x110, tmp);
	
	reg32_write(DRAMC0_BASE+0x0E4, 0x00000001);
	reg32_write(DDRPHY_BASE+0x0E4, 0x00000001);
	
	reg32_write(DRAMC0_BASE+0x1EC, 0x00000001);
	reg32_write(DDRPHY_BASE+0x1EC, 0x00000001);
	
	reg32_write(DRAMC0_BASE+0x084, 0x00000a56);
	reg32_write(DDRPHY_BASE+0x084, 0x00000a56);
	
	reg32_write(DRAMC0_BASE+0x000, EmiData[13]);
	reg32_write(DDRPHY_BASE+0x000, EmiData[13]);
	
	reg32_write(DRAMC0_BASE+0x004, EmiData[15]);
	reg32_write(DDRPHY_BASE+0x004, EmiData[15]);
	
	reg32_write(DRAMC0_BASE+0x044, EmiData[17]);
	reg32_write(DDRPHY_BASE+0x044, EmiData[17]);
	
	reg32_write(DRAMC0_BASE+0x07C, EmiData[20]);
	reg32_write(DDRPHY_BASE+0x07C, EmiData[20]);
	
	reg32_write(DRAMC0_BASE+0x0FC, EmiData[25]);
	reg32_write(DDRPHY_BASE+0x0FC, EmiData[25]);
	
	reg32_write(DRAMC0_BASE+0x1E8, EmiData[24]);
	reg32_write(DDRPHY_BASE+0x1E8, EmiData[24]);
	
	reg32_write(DRAMC0_BASE+0x1F8, EmiData[26]);
	reg32_write(DDRPHY_BASE+0x1F8, EmiData[26]);
	
	reg32_wsmask(DRAMC0_BASE+0x008, 28, 1, 1);
	reg32_wsmask(DDRPHY_BASE+0x008, 28, 1, 1);
	
	reg32_write(DRAMC0_BASE+0x1DC, EmiData[19]);
	reg32_write(DDRPHY_BASE+0x1DC, EmiData[19]);
	
	reg32_write(DRAMC0_BASE+0x008, EmiData[18]);
	reg32_write(DDRPHY_BASE+0x008, EmiData[18]);
	
	reg32_write(DRAMC0_BASE+0x010, 0x00000000);
	reg32_write(DDRPHY_BASE+0x010, 0x00000000);
	
	reg32_write(DRAMC0_BASE+0x0F8, 0xedcb000f);
	reg32_write(DDRPHY_BASE+0x0F8, 0xedcb000f);
	
	reg32_write(DRAMC0_BASE+0x020, 0x00000000);
	reg32_write(DDRPHY_BASE+0x020, 0x00000000);
	
	reg32_write(DRAMC0_BASE+0x110, 0x00113381);
	reg32_write(DDRPHY_BASE+0x110, 0x00113381);
	
	reg32_wsmask(DRAMC0_BASE+0x07C, 4, 0x7, 0x7);
	reg32_wsmask(DDRPHY_BASE+0x07C, 4, 0x7, 0x7);
	
	reg32_wsmask(DRAMC0_BASE+0x0E4, 4, 1, 0);
	reg32_wsmask(DDRPHY_BASE+0x0E4, 4, 1, 0);
	
	reg32_write(DRAMC0_BASE+0x210, 0x0a090c08);
	reg32_write(DDRPHY_BASE+0x210, 0x0a090c08);
	
	reg32_write(DRAMC0_BASE+0x214, 0x03000300);
	reg32_write(DDRPHY_BASE+0x214, 0x03000300);
	
	reg32_write(DRAMC0_BASE+0x218, 0x0a090807);
	reg32_write(DDRPHY_BASE+0x218, 0x0a090807);
	
	reg32_write(DRAMC0_BASE+0x21C, 0x04040602);
	reg32_write(DDRPHY_BASE+0x21C, 0x04040602);
	
	reg32_write(DRAMC0_BASE+0x220, 0x06090909);
	reg32_write(DDRPHY_BASE+0x220, 0x06090909);
	
	reg32_write(DRAMC0_BASE+0x224, 0x02000100);
	reg32_write(DDRPHY_BASE+0x224, 0x02000100);
	
	reg32_write(DRAMC0_BASE+0x228, 0x07070705);
	reg32_write(DDRPHY_BASE+0x228, 0x07070705);
	
	reg32_write(DRAMC0_BASE+0x22C, 0x01020203);
	reg32_write(DDRPHY_BASE+0x22C, 0x01020203);
	reg32_write(DRAMC0_BASE+0x018, 0x01020203 + 0x23292329);
	reg32_write(DDRPHY_BASE+0x018, 0x01020203 + 0x23292329);
	reg32_write(DRAMC0_BASE+0x01C, 0x01020203 + 0x23292329);
	reg32_write(DDRPHY_BASE+0x01C, 0x01020203 + 0x23292329);
	
	reg32_write(DRAMC0_BASE+0x0CC, 0x000000cc);
	reg32_write(DDRPHY_BASE+0x0CC, 0x000000cc);
	
	reg32_write(DRAMC0_BASE+0x648, 0x0ff00ff0);
	reg32_write(DDRPHY_BASE+0x648, 0x0ff00ff0);
}

void EmiDramcCalib(void) {
	xputs("=====> EMI DRAMC Calibration\n");
	uint32_t tmp;
	
	/* ----------- PART1 ----------- */
	reg32_wsmask(DRAMC0_BASE+0x0E0, 28, 1, 1);
	reg32_wsmask(DDRPHY_BASE+0x0E0, 28, 1, 1);
	reg32_wsmask(DRAMC_NAO_BASE+0x0E0, 28, 1, 1);
	
	reg32_wsmask(DRAMC0_BASE+0x1E4, 8, 1, 1);
	reg32_wsmask(DDRPHY_BASE+0x1E4, 8, 1, 1);
	reg32_wsmask(DRAMC_NAO_BASE+0x1E4, 8, 1, 1);
	
	/* kagami hiiragi 20210101 */
	
	reg32_wsmask(DRAMC0_BASE+0x1E4, 8, 1, 0);
	reg32_wsmask(DDRPHY_BASE+0x1E4, 8, 1, 0);
	reg32_wsmask(DRAMC_NAO_BASE+0x1E4, 8, 1, 0);
	
	/* >>> */
	tmp  = reg32_read(DRAMC0_BASE+0x0F0);
	tmp |= reg32_read(DDRPHY_BASE+0x0F0);
	tmp |= reg32_read(DRAMC_NAO_BASE+0x0F0);
	tmp |= (1<<28);
	reg32_write(DRAMC0_BASE+0x0F0, tmp);
	reg32_write(DDRPHY_BASE+0x0F0, tmp);
	reg32_write(DRAMC_NAO_BASE+0x0F0, tmp);
	
	tmp  = reg32_read(DRAMC0_BASE+0x0F0);
	tmp |= reg32_read(DDRPHY_BASE+0x0F0);
	tmp |= reg32_read(DRAMC_NAO_BASE+0x0F0);
	tmp |= (1<<25);
	reg32_write(DRAMC0_BASE+0x0F0, tmp);
	reg32_write(DDRPHY_BASE+0x0F0, tmp);
	reg32_write(DRAMC_NAO_BASE+0x0F0, tmp);
	
	asm volatile ("dsb" ::: "memory");
	usleep(1);
	
	tmp  = reg32_read(DRAMC0_BASE+0x0F0);
	tmp |= reg32_read(DDRPHY_BASE+0x0F0);
	tmp |= reg32_read(DRAMC_NAO_BASE+0x0F0);
	tmp &= ~(1<<28);
	reg32_write(DRAMC0_BASE+0x0F0, tmp);
	reg32_write(DDRPHY_BASE+0x0F0, tmp);
	reg32_write(DRAMC_NAO_BASE+0x0F0, tmp);
	
	tmp  = reg32_read(DRAMC0_BASE+0x0F0);
	tmp |= reg32_read(DDRPHY_BASE+0x0F0);
	tmp |= reg32_read(DRAMC_NAO_BASE+0x0F0);
	tmp &= ~(1<<25);
	reg32_write(DRAMC0_BASE+0x0F0, tmp);
	reg32_write(DDRPHY_BASE+0x0F0, tmp);
	reg32_write(DRAMC_NAO_BASE+0x0F0, tmp);
	
	/* ----------- PART2 ----------- */
}

void EmiInit(const uint32_t *EmiData) {
	xprintf("## INIT EMI ! [%x]\n", EmiData);
	
	reg32_wsmask(EMI_BASE+0x040, 0, 0x0000cc00, 0x0000cc00);
	
	switch (EmiData[1] & 0xf) {
	case 2: /* LPDDR2 */
		xputs("[EMI] DRAM is the LPDDR2\n");
		EmiDramcExtdnCalib();
		EmiInitLPDDR2(EmiData);
		break;
	case 3: /* LPDDR3 */
		xputs("[EMI] DRAM is the LPDDR3\n");
		EmiDramcExtdnCalib();
		EmiInitLPDDR3(EmiData);
		break;
	case 4: /* PCDDR3 */
		xputs("[EMI] DRAM is the PCDDR3\n");
		EmiDramcExtdnCalib();
		break;
	default: /* unknown */
		xputs("[EMI] Unknown DRAM Type!\n");
		return;
	}
	
	reg32_wsmask(EMI_BASE+0x060, 10, 1, 1);
	
	EmiDramcCalib();
	
	if (reg32_rsmask(EMI_BASE+0x000, 17, 1)) {
		xputs("[EMI] We are supporting the Dual Ranks!\n");
		
		/* RANKINCTL = DQSINCTL + 2 */
		reg32_wsmask(DRAMC0_BASE+0x1C4, 16, 0xf, 2 + reg32_rsmask(DRAMC0_BASE+0x0E0, 24, 0x7));
	}
	
	reg32_wsmask(DRAMC0_BASE+0x1C0, 31, 1, 1);
	reg32_wsmask(DDRPHY_BASE+0x1C0, 31, 1, 1);
	
	reg32_wsmask(DRAMC0_BASE+0x0E4, 12, 0x7, 0x3);
	reg32_wsmask(DDRPHY_BASE+0x0E4, 12, 0x7, 0x3);
	
	reg32_wsmask(DRAMC0_BASE+0x0F0, 27, 1, 1);
	reg32_wsmask(DDRPHY_BASE+0x0F0, 27, 1, 1);
	
	reg32_wsmask(DRAMC0_BASE+0x1EC, 0, 0x00004f10, 0x00004f10);
	reg32_wsmask(DDRPHY_BASE+0x1EC, 0, 0x00004f10, 0x00004f10);
	
	reg32_wsmask(DRAMC0_BASE+0x0FC, 25, 0x3, 0x0);
	reg32_wsmask(DDRPHY_BASE+0x0FC, 25, 0x3, 0x0);
	
	reg32_wsmask(DDRPHY_BASE+0x640, 0, 0xfeb88200, 0xfeb88200);
	
	reg32_wsmask(DRAMC0_BASE+0x1DC, 0, 0xa0000000, 0xa0000000);
	reg32_wsmask(DDRPHY_BASE+0x1DC, 0, 0xa0000000, 0xa0000000);
	
	xputs("[EMI] We Ending!!\n");
}



void AllThePLLThings(void) {
	reg32_wsmask(APMIXED_BASE+0x000, 0, 1, 1);
	usleep(100);
	reg32_wsmask(APMIXED_BASE+0x000, 1, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x110, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x130, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x150, 0, 1, 1);
	usleep(30);
	reg32_wsmask(APMIXED_BASE+0x110, 1, 1, 0);
	reg32_wsmask(APMIXED_BASE+0x130, 1, 1, 0);
	reg32_wsmask(APMIXED_BASE+0x150, 1, 1, 0);
	reg32_write(APMIXED_BASE+0x104, 0x8009a000);
	reg32_write(APMIXED_BASE+0x124, 0x8009a000 + 0x4d000);
	reg32_write(APMIXED_BASE+0x144, 0x80000030);
	reg32_wsmask(APMIXED_BASE+0x100, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x120, 0, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x140, 0, 1, 1);
	usleep(20);
	reg32_wsmask(APMIXED_BASE+0x120, 27, 1, 1);
	reg32_wsmask(APMIXED_BASE+0x140, 27, 1, 1);
	reg32_write(TOPCKGEN_BASE+0x02C, 0x80000000);
	reg32_write(TOPCKGEN_BASE+0x000, 0x0b60b446);
	usleep(20);
	reg32_write(MCUCFG_BASE+0x060, 0x00000012);
	reg32_write(MCUCFG_BASE+0x064, 0x00000015);
	reg32_wsmask(INFRACFG_AO_BASE+0x000, 0, 0x7, 0x4);
	reg32_write(TOPCKGEN_BASE+0x000, 0x27653246);
	reg32_write(TOPCKGEN_BASE+0x004, 0x00a95008);
	reg32_write(SLEEP_BASE+0x000, 0x0b160001);
	reg32_write(TOPCKGEN_BASE+0x020, 0xbfcffdf2);
	
	/* Ultra Slozhniy Things */
	/* [ DIS power up (whatever it is) ] */
	reg32_wsmask(SLEEP_BASE+0x23C, 2, 1, 1);
	reg32_wsmask(SLEEP_BASE+0x23C, 3, 1, 1);
	while (!reg32_rsmask(SLEEP_BASE+0x60C, 3, 1));
	while (!reg32_rsmask(SLEEP_BASE+0x610, 3, 1));
	reg32_wsmask(SLEEP_BASE+0x23C, 4, 1, 0);
	reg32_wsmask(SLEEP_BASE+0x23C, 1, 1, 0);
	reg32_wsmask(SLEEP_BASE+0x23C, 0, 1, 1);
	reg32_wsmask(SLEEP_BASE+0x23C, 8, 0xf, 0x0);
	/* FMM Clk? */
	reg32_wsmask(INFRACFG_AO_BASE+0x220, 0, 0x802, 0x000);
	while (reg32_read(INFRACFG_AO_BASE+0x228) & 0x802);
	
	/* [ GPU Power Up MFG ] */
	reg32_wsmask(SLEEP_BASE+0x214, 2, 1, 1);
	reg32_wsmask(SLEEP_BASE+0x214, 3, 1, 1);
	while (!reg32_rsmask(SLEEP_BASE+0x60C, 4, 1));
	while (!reg32_rsmask(SLEEP_BASE+0x610, 4, 1));
	reg32_wsmask(SLEEP_BASE+0x214, 4, 1, 0);
	reg32_wsmask(SLEEP_BASE+0x214, 1, 1, 0);
	reg32_wsmask(SLEEP_BASE+0x214, 0, 1, 1);
	reg32_wsmask(SLEEP_BASE+0x214, 8, 0xf, 0x0);
	while (reg32_rsmask(SLEEP_BASE+0x214, 16, 1));
	reg32_wsmask(INFRACFG_AO_BASE+0x220, 0, 0x820, 0x000);
	while (reg32_read(INFRACFG_AO_BASE+0x228) & 0x820);
	
	/* 3333333333333 */
	reg32_wsmask(SLEEP_BASE+0x280, 2, 1, 1);
	reg32_wsmask(SLEEP_BASE+0x280, 3, 1, 1);
	while (!reg32_rsmask(SLEEP_BASE+0x60C, 1, 1));
	while (!reg32_rsmask(SLEEP_BASE+0x610, 1, 1));
	reg32_wsmask(SLEEP_BASE+0x280, 4, 1, 0);
	reg32_wsmask(SLEEP_BASE+0x280, 1, 1, 0);
	reg32_wsmask(SLEEP_BASE+0x280, 0, 1, 1);
	reg32_wsmask(SLEEP_BASE+0x280, 8, 0x3, 0x0);
	reg32_wsmask(INFRACFG_AO_BASE+0x220, 0, 0x310, 0x000);
	while (reg32_read(INFRACFG_AO_BASE+0x228) & 0x310);
	
	/* ------------- */
	
	reg32_wsmask(MMSYS_CONFIG_BASE+0x100, 0, 0x203, 0x0);
	reg32_write(MMSYS_CONFIG_BASE+0x110, 0x00000000);
	reg32_write(G3D_CONFIG_BASE+0x008, 0x00000001);
}



int main(void) {
	/* setup GPT4 to tick once per microsecond */
	reg32_write(APXGPT_BASE+APXGPT_CLK(3), 26/2);
	reg32_write(APXGPT_BASE+APXGPT_CON(3), (3<<4)|(1<<1)|(1<<0)); /* opmode=3, clr=1, en=1 */

	/* setup UART2 (again some 16550A-compatible stuff?? great!!!) */ {
		reg32_write(TOPCKGEN_BASE+0x084, (1<<11)); /* enable uart2 clk */
	
		/* wait for all pending transmissions to end, to avoid garbage characters */
		while ((reg32_read(UART1_BASE+0x014) & 0x60) != 0x60);
	
		/* configure the format and baudrate */
		uint16_t dl = 26000000 / 8 / 460800;
		reg32_write(UART1_BASE+0x024, 1); /* HIGHSPEED = 1 */
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
	
	/* --------------------- */
	
	const uint32_t EmiSetting[] = {
		0x00000000,
		0x00000003,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00025052,
		0xaa00aa00,
		0xaa00aa00,
		0x44584493,
		0x01000000,
		0xf0048683,
		0xa00631d1,
		0xbf080401,
		0x01806c3f,
		0xd1642342,
		0x00008888,
		0x88888888,
		0x00000000,
		0x00000000,
		0x11000510,
		0x07800000,
		0x04002600,
		0x20000000,
		0x20000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00c30001,
		0x00060002,
		0x00020003,
		0x00000003,
		0x00ff000a,
		0x0000003f,
	};
	AllThePLLThings();
	EmiInit(EmiSetting);
	
	/* Test */
	strcpy((void*)0x80000000, "Hatsune Miku 1234567 && Hiiragi Kagami ==> Hatsuragi Kagamiku ?!?!?? ^^^ Emiri Kato");
	hexdump((void*)0x80000000, 256);
	
	return 0;
}
