#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <xprintf.h>
#include <wallclk.h>
#include <mtk_regs.h>
#include <mtk_gpio.h>
#include <mtk_pmic.h>

/* uart */
void uputc(char c) {
	while ((reg32_read(UART1_BASE+0x014) & 0x60) != 0x60);
	reg32_write(UART1_BASE+0x000, c);
}

char ugetc(void) {
	while ((reg32_read(UART1_BASE+0x014) & 0x01) != 0x01);
	return reg32_read(UART1_BASE+0x000);
}

void hexdump(const void *ptr, int len) {
	for (int i = 0; i < len; i += 16) {
		put_dump(ptr + i, (uint32_t)ptr + i, 16, 1);
	}
}

/*----------------------------------------------------------------------------*/

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
	if (EmiData[28] == 0x30000000) tmp |= 0x3<<20; /* 768 MiB rank1 */
	if (EmiData[28] == 0x60000000) tmp |= 0x6<<20; /* 1.5 GiB rank1 */
	reg32_write(EMI_BASE+0x038, tmp);
	
	reg32_write(EMI_BASE+0x100, 0x7F07704A);
	reg32_write(EMI_BASE+0x108, 0xA0A070DB);
	reg32_write(EMI_BASE+0x110, 0xA0A07042);
	reg32_write(EMI_BASE+0x118, 0x07007047);
	reg32_write(EMI_BASE+0x120, 0x2030604B);
	reg32_write(EMI_BASE+0x128, 0xA0A07046);
	reg32_write(EMI_BASE+0x134, 0xA0A07046);
	reg32_write(EMI_BASE+0x008, 0x0D1E293A);
	reg32_write(EMI_BASE+0x010, 0x09190819);
	reg32_write(EMI_BASE+0x030, 0x2B2B282E);
	reg32_write(EMI_BASE+0x018, 0x3657587A);
	reg32_write(EMI_BASE+0x020, 0xFFFF0848);
	reg32_write(EMI_BASE+0x078, 0x34220E17);
	reg32_write(EMI_BASE+0x0D0, 0xCCCCCCCC);
	reg32_write(EMI_BASE+0x0D8, 0xCCCCCCCC);
	reg32_write(EMI_BASE+0x0E8, 0x00020027);
	reg32_write(EMI_BASE+0x0F0, 0x38460000);
	reg32_write(EMI_BASE+0x0F8, 0x00000000);
	reg32_write(EMI_BASE+0x140, 0x20406188);
	reg32_write(EMI_BASE+0x144, 0x20406188);
	reg32_write(EMI_BASE+0x148, 0x9719595E);
	reg32_write(EMI_BASE+0x14c, 0x9719595E);
	reg32_write(EMI_BASE+0x150, 0x64F3FC79);
	reg32_write(EMI_BASE+0x154, 0x64F3FC79);
	reg32_write(EMI_BASE+0x158, 0xFF01FF00);
	reg32_write(EMI_BASE+0x028, 0x00421000);
	reg32_write(EMI_BASE+0x060, 0x000006FF);
	
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
	reg32_write(DRAMC0_BASE+0x048, 0x0001110D);
	reg32_write(DDRPHY_BASE+0x048, 0x0001110D);
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
	reg32_write(DRAMC0_BASE+0x0E0, 0x1B004004);
	reg32_write(DDRPHY_BASE+0x0E0, 0x1B004004);
	reg32_write(DRAMC0_BASE+0x124, 0xAA080033);
	reg32_write(DDRPHY_BASE+0x124, 0xAA080033);
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
	reg32_write(DRAMC0_BASE+0x028, 0xF1200F01);
	reg32_write(DDRPHY_BASE+0x028, 0xF1200F01);
	reg32_write(DRAMC0_BASE+0x1E0, 0x0110017F);
	reg32_write(DDRPHY_BASE+0x1E0, 0x0110017F);
	reg32_write(DRAMC0_BASE+0x1E8, EmiData[24]);
	reg32_write(DDRPHY_BASE+0x1E8, EmiData[24]);
	reg32_write(DRAMC0_BASE+0x158, 0xF0F0F0F0);
	reg32_write(DDRPHY_BASE+0x158, 0xF0F0F0F0);
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
	reg32_write(DRAMC0_BASE+0x110, 0x0B052311);
	reg32_write(DDRPHY_BASE+0x110, 0x0B052311);
	reg32_write(DRAMC0_BASE+0x0E4, 0x00000005);
	reg32_write(DDRPHY_BASE+0x0E4, 0x00000005);
	
	asm volatile ("dsb #0xf" ::: "memory");
	usleep(200);
	
	reg32_write(DRAMC0_BASE+0x088, EmiData[46]);
	reg32_write(DDRPHY_BASE+0x088, EmiData[46]);
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000001);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000001);
	
	asm volatile ("dsb #0xf" ::: "memory");
	usleep(10);
	
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000000);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000000);
	reg32_wsmask(DRAMC0_BASE+0x110, 0, 0x7, 0x0);
	reg32_wsmask(DDRPHY_BASE+0x110, 0, 0x7, 0x0);
	reg32_write(DRAMC0_BASE+0x088, EmiData[45]);
	reg32_write(DDRPHY_BASE+0x088, EmiData[45]);
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000001);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000001);
	
	asm volatile ("dsb #0xf" ::: "memory");
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
		
		asm volatile ("dsb #0xf" ::: "memory");
		usleep(1);
		
		reg32_write(DRAMC0_BASE+0x1E4, 0x00000000);
		reg32_write(DDRPHY_BASE+0x1E4, 0x00000000);
		reg32_wsmask(DRAMC0_BASE+0x110, 3, 1, 0);
		reg32_wsmask(DDRPHY_BASE+0x110, 3, 1, 0);
		reg32_wsmask(DRAMC0_BASE+0x110, 0, 1, 1);
		reg32_wsmask(DDRPHY_BASE+0x110, 0, 1, 1);
	}
	
	reg32_write(DRAMC0_BASE+0x088, EmiData[41]);
	reg32_write(DDRPHY_BASE+0x088, EmiData[41]);
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000001);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000001);
	
	asm volatile ("dsb #0xf" ::: "memory");
	usleep(1);
	
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000000);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000000);
	reg32_write(DRAMC0_BASE+0x088, EmiData[42]);
	reg32_write(DDRPHY_BASE+0x088, EmiData[42]);
	reg32_write(DRAMC0_BASE+0x1E4, 0x00000001);
	reg32_write(DDRPHY_BASE+0x1E4, 0x00000001);
	
	asm volatile ("dsb #0xf" ::: "memory");
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
	reg32_write(DRAMC0_BASE+0x084, 0x00000A56);
	reg32_write(DDRPHY_BASE+0x084, 0x00000A56);
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
	reg32_write(DRAMC0_BASE+0x008, EmiData[18] | (1<<28));
	reg32_write(DDRPHY_BASE+0x008, EmiData[18] | (1<<28));
	reg32_write(DRAMC0_BASE+0x1DC, EmiData[19]);
	reg32_write(DDRPHY_BASE+0x1DC, EmiData[19]);
	reg32_write(DRAMC0_BASE+0x008, EmiData[18]);
	reg32_write(DDRPHY_BASE+0x008, EmiData[18]);
	reg32_write(DRAMC0_BASE+0x010, 0x00000000);
	reg32_write(DDRPHY_BASE+0x010, 0x00000000);
	reg32_write(DRAMC0_BASE+0x0F8, 0xEDCB000F);
	reg32_write(DDRPHY_BASE+0x0F8, 0xEDCB000F);
	reg32_write(DRAMC0_BASE+0x020, 0x00000000);
	reg32_write(DDRPHY_BASE+0x020, 0x00000000);
	reg32_write(DRAMC0_BASE+0x110, 0x00113381);
	reg32_write(DDRPHY_BASE+0x110, 0x00113381);
	reg32_wsmask(DRAMC0_BASE+0x07C, 4, 0x7, 0x7);
	reg32_wsmask(DDRPHY_BASE+0x07C, 4, 0x7, 0x7);
	reg32_wsmask(DRAMC0_BASE+0x0E4, 4, 1, 0);
	reg32_wsmask(DDRPHY_BASE+0x0E4, 4, 1, 0);
	reg32_write(DRAMC0_BASE+0x210, 0x0A090C08);
	reg32_write(DDRPHY_BASE+0x210, 0x0A090C08);
	reg32_write(DRAMC0_BASE+0x214, 0x03000300);
	reg32_write(DDRPHY_BASE+0x214, 0x03000300);
	reg32_write(DRAMC0_BASE+0x218, 0x0A090807);
	reg32_write(DDRPHY_BASE+0x218, 0x0A090807);
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
	reg32_write(DRAMC0_BASE+0x018, 0x242B252C);
	reg32_write(DDRPHY_BASE+0x018, 0x242B252C);
	reg32_write(DRAMC0_BASE+0x01C, 0x242B252C);
	reg32_write(DDRPHY_BASE+0x01C, 0x242B252C);
	reg32_write(DRAMC0_BASE+0x0CC, 0x000000CC);
	reg32_write(DDRPHY_BASE+0x0CC, 0x000000CC);
	reg32_write(DRAMC0_BASE+0x648, 0x0FF00FF0);
	reg32_write(DDRPHY_BASE+0x648, 0x0FF00FF0);
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
	
	reg32_wsmask(DRAMC0_BASE+0x1DC, 0, 0xa000000, 0xa000000);
	reg32_wsmask(DDRPHY_BASE+0x1DC, 0, 0xa000000, 0xa000000);
	
	xputs("[EMI] We Ending!!\n");
}

void EmiDramcSetFreq(int freq, int sth) {
	uint32_t tmp;
	
	reg32_wsmask(DRAMC0_BASE+0x07C, 0, 1, 1);
	reg32_wsmask(DDRPHY_BASE+0x07C, 0, 1, 1);
	
	     if (sth == 3) tmp = 0x10020;
	else if (sth == 2) tmp = 0x10022;
	else               tmp = 0x00007;
	reg32_write(DDRPHY_BASE+0x640, tmp);
	
	reg32_write(DDRPHY_BASE+0x5c0, 0x00000000);
	reg32_write(DDRPHY_BASE+0x5c4, 0x00000000);
	reg32_write(DDRPHY_BASE+0x5c8, 0x1111ff11);
	reg32_write(DDRPHY_BASE+0x5cc, 0x11111111);
	
	reg32_wsmask(DDRPHY_BASE+0x600, 14, 0x3, 0x0);
	reg32_wsmask(DDRPHY_BASE+0x600, 12, 1, 1);
	
	reg32_wsmask(DDRPHY_BASE+0x604, 0, 0xfe000000, 0xa4000000);
	reg32_wsmask(DDRPHY_BASE+0x604, 18, 1, 1);
	
	reg32_wsmask(DDRPHY_BASE+0x608, 10, 0x3, 0x0);
	reg32_wsmask(DDRPHY_BASE+0x60C, 26, 0x3, 0x3);
	reg32_wsmask(DDRPHY_BASE+0x60C, 9, 1, sth < 4);
	
	reg32_wsmask(DDRPHY_BASE+0x610, 10, 0x3, 0x0);
	reg32_wsmask(DDRPHY_BASE+0x614, 26, 0x3, 0x3);
	reg32_wsmask(DDRPHY_BASE+0x614, 9, 1, sth < 4);
	
	reg32_wsmask(DDRPHY_BASE+0x618, 10, 0x3, 0x0);
	reg32_wsmask(DDRPHY_BASE+0x61C, 26, 0x3, 0x3);
	reg32_wsmask(DDRPHY_BASE+0x61C, 9, 1, sth < 4);
	
	/* ..ALL The magic.. */
	if (sth < 4) {
		/* 1066mhz */
		reg32_wsmask(DDRPHY_BASE+0x624, 0, 0xfffffffe, 0x98d16872);
		reg32_wsmask(DDRPHY_BASE+0x608, 0, 0x1fc, 0x02c);
		reg32_wsmask(DDRPHY_BASE+0x610, 0, 0x1fc, 0x02c);
		reg32_wsmask(DDRPHY_BASE+0x618, 0, 0x1fc, 0x02c);
	} else {
		/* We don't have this case, dont care at all */
	}
	
	     if (sth == 3) tmp = 0x10020;
	else if (sth == 2) tmp = 0x10022;
	else               tmp = 0x00007;
	reg32_write(DDRPHY_BASE+0x640, tmp);
	usleep(2);
	
	reg32_wsmask(DDRPHY_BASE+0x604, 14, 1, 1);
	
	usleep(2);
	
	reg32_wsmask(DDRPHY_BASE+0x604, 15, 1, 1);
	
	usleep(1000);
	
	reg32_wsmask(DDRPHY_BASE+0x600, 2, 1, 1);
	
	usleep(20);
	
	reg32_wsmask(DDRPHY_BASE+0x604, 24, 1, 1);
	
	usleep(1);
	
	reg32_wsmask(DDRPHY_BASE+0x60C, 18, 1, 1);
	
	reg32_wsmask(DDRPHY_BASE+0x614, 18, 1, sth < 4);
	reg32_wsmask(DDRPHY_BASE+0x61C, 18, 1, sth < 4);
	
	usleep(23);
	
	     if (sth == 3) tmp = 0x10030;
	else if (sth == 2) tmp = 0x10032;
	else               tmp = 0x00037;
	reg32_write(DDRPHY_BASE+0x640, tmp);
}


void InitPMIC(void) {
	xprintf("pwrap_init--->%d\n", pwrap_init());
	
	/* en VMem */
	pmic_wsmask(0x552, 1,  1, 0);  // no sleep
	pmic_wsmask(0x554, 4,  3, 0);  // 1.2v
	pmic_wsmask(0x552, 14, 1, 1);  // en
}

void InitPLL(void) {
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
	
	reg32_write(APMIXED_BASE+0x104, (1<<31)|(0<<24)|(1001000ll * 0x4000 / 26000));
	reg32_write(APMIXED_BASE+0x124, (1<<31)|(0<<24)|(1501500ll * 0x4000 / 26000));
	reg32_write(APMIXED_BASE+0x144, (1<<31)|(0<<24)|(1248000ll / 26000));
	
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
	reg32_write(SLEEP_BASE+0x000, 0x0b160001);  // Unlock Sleep regs
	reg32_write(TOPCKGEN_BASE+0x020, 0xbfcffdf2);
	
	reg32_wsmask(SLEEP_BASE+0x23C, 2, 1, 1);
	reg32_wsmask(SLEEP_BASE+0x23C, 3, 1, 1);
	while (!reg32_rsmask(SLEEP_BASE+0x60C, 3, 1));
	while (!reg32_rsmask(SLEEP_BASE+0x610, 3, 1));
	reg32_wsmask(SLEEP_BASE+0x23C, 4, 1, 0);
	reg32_wsmask(SLEEP_BASE+0x23C, 1, 1, 0);
	reg32_wsmask(SLEEP_BASE+0x23C, 0, 1, 1);
	reg32_wsmask(SLEEP_BASE+0x23C, 8, 0xf, 0x0);
	reg32_wsmask(INFRACFG_AO_BASE+0x220, 0, 0x802, 0x000);
	while (reg32_read(INFRACFG_AO_BASE+0x228) & 0x802);
	
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
	
	reg32_wsmask(MMSYS_CONFIG_BASE+0x100, 0, 0x203, 0x0);
	reg32_write(MMSYS_CONFIG_BASE+0x110, 0x00000000);
	reg32_write(G3D_CONFIG_BASE+0x008, 0x00000001);

	EmiDramcSetFreq(1066, 1);
	
	reg32_wsmask(APMIXED_BASE+0x004, 0, 0x03031033, 0x00000000);
	reg32_wsmask(APMIXED_BASE+0x008, 0, 0x00000003, 0x00000000);
}

void InitDRAM(void) {
	static const uint32_t EmiSetting[] = {
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
	
	EmiInit(EmiSetting);
}





int main(void) {
	/* setup UART2 */ {
		reg32_write(TOPCKGEN_BASE+0x084, (1<<11)); /* enable uart2 clk */
	
		/* wait for all pending transmissions to end, to avoid garbage characters */
		while ((reg32_read(UART1_BASE+0x014) & 0x60) != 0x60);
	
		/* configure the format and baudrate */
		uint16_t dl = 26000000 / 4 / 921600;
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
	xputs("\n\e[1;33m*******Hello Mediatek! (We do the DRAM!)*******\e[0m\n");
	
	wallclk_init();
	
	/* release dramc conf iso */
	reg32_wsmask(TOPRGU_BASE+0x040, 0, 0xff000400, 0x59000000);
	/* release dramc iso */
	reg32_wsmask(TOPRGU_BASE+0x040, 0, 0xff000200, 0x59000000);
	/* release dramc sref */
	reg32_wsmask(TOPRGU_BASE+0x040, 0, 0xff000100, 0x59000000);
	/* disable wdt */
	reg32_wsmask(TOPRGU_BASE+0x000, 0, 0xff000001, 0x22000000);
	
	InitPMIC();
	InitPLL();
	InitDRAM();
	
	/* Test */
	strcpy((void*)0x80000000, "Hatsune Miku 1234567 && Hiiragi Kagami ==> Hatsuragi Kagamiku ?!?!?? ^^^ Emiri Kato");
	hexdump((void*)0x80000000, 256);
	
	return 0;
}
