/*
 * The NewTech Mediatek MT6580 Linux Loader.
 * Intended to be loaded  through usb or put in place of The Little Kernel.
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <xprintf.h>
#include <mtk_regs.h>
#include <mtk_pmic.h>
#include <mtk_gpio.h>
#include <wallclk.h>


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

void *memcpy(void *dst, const void *src, size_t len) {
	for (size_t i = 0; i < len; i++)
		*(uint8_t*)(dst+i) = *(uint8_t*)(src+i);
	return dst;
}

void *_sbrk(int amount) {
	/* right into konata's memmap ! */
	static void *heap_top = (void*)0x88000000;
	
	void *heap_ptr = heap_top;
	heap_top += amount;
	
	return heap_ptr;
}

#define MT6350_SPK_CON0				0x0052
#define MT6350_SPK_CON1				0x0054
#define MT6350_SPK_CON2				0x0056
#define MT6350_SPK_CON6				0x005E
#define MT6350_SPK_CON7				0x0060
#define MT6350_SPK_CON8				0x0062
#define MT6350_SPK_CON9				0x0064
#define MT6350_SPK_CON10			0x0066
#define MT6350_SPK_CON11			0x0068
#define MT6350_SPK_CON12			0x006A
#define MT6350_CID				0x0100
#define MT6350_TOP_CKPDN0			0x0102
#define MT6350_TOP_CKPDN0_SET			0x0104
#define MT6350_TOP_CKPDN0_CLR			0x0106
#define MT6350_TOP_CKPDN1			0x0108
#define MT6350_TOP_CKPDN1_SET			0x010A
#define MT6350_TOP_CKPDN1_CLR			0x010C
#define MT6350_TOP_CKPDN2			0x010E
#define MT6350_TOP_CKPDN2_SET			0x0110
#define MT6350_TOP_CKPDN2_CLR			0x0112
#define MT6350_TOP_RST_CON_SET			0x0116
#define MT6350_TOP_RST_CON_CLR			0x0118
#define MT6350_TOP_CKCON1			0x0126
#define MT6350_DRV_CON2				0x0156
#define MT6350_AUDTOP_CON0			0x0700
#define MT6350_AUDTOP_CON1			0x0702
#define MT6350_AUDTOP_CON2			0x0704
#define MT6350_AUDTOP_CON3			0x0706
#define MT6350_AUDTOP_CON4			0x0708
#define MT6350_AUDTOP_CON5			0x070A
#define MT6350_AUDTOP_CON6			0x070C
#define MT6350_AUDTOP_CON7			0x070E
#define MT6350_AUDTOP_CON8			0x0710
#define MT6350_AUDTOP_CON9			0x0712
#define MT6350_ABB_AFE_CON0			0x4000
#define MT6350_ABB_AFE_CON1			0x4002
#define MT6350_ABB_AFE_CON2			0x4004
#define MT6350_ABB_AFE_CON3			0x4006
#define MT6350_ABB_AFE_CON4			0x4008
#define MT6350_ABB_AFE_CON5			0x400A
#define MT6350_ABB_AFE_CON6			0x400C
#define MT6350_ABB_AFE_CON7			0x400E
#define MT6350_ABB_AFE_CON8			0x4010
#define MT6350_ABB_AFE_CON9			0x4012
#define MT6350_ABB_AFE_CON10			0x4014
#define MT6350_ABB_AFE_CON11			0x4016
#define MT6350_ABB_AFE_STA0			0x4018
#define MT6350_ABB_AFE_STA1			0x401A
#define MT6350_ABB_AFE_STA2			0x401C
#define MT6350_AFE_UP8X_FIFO_CFG0		0x401E
#define MT6350_AFE_UP8X_FIFO_LOG_MON0		0x4020
#define MT6350_AFE_UP8X_FIFO_LOG_MON1		0x4022
#define MT6350_AFE_PMIC_NEWIF_CFG0		0x4024
#define MT6350_AFE_PMIC_NEWIF_CFG1		0x4026
#define MT6350_AFE_PMIC_NEWIF_CFG2		0x4028
#define MT6350_AFE_PMIC_NEWIF_CFG3		0x402A
#define MT6350_ABB_AFE_TOP_CON0			0x402C
#define MT6350_ABB_MON_DEBUG0			0x402E

void vibrator_enable(bool enable) {
	if (enable) {
		pmic_wsmask(0x544, 5, 0x7, 0x5); //2.8v
		pmic_wsmask(0x542, 15, 1, 1); //enable
	} else {
		pmic_wsmask(0x542, 15, 1, 0); //disable
	}
}

bool get_charger_presence(void) {
	return pmic_rsmask(0x0000, 5, 1);
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

void preinit(void) {
	/* config pins functions ! */ {
		/* ------------ PMIC ----------- */
		gpio_iomux_cfg(13, 6); /* PMIC_EINT [EINT13] */
		gpio_iomux_cfg(29, 1); /* PMIC_SPI_CSN */
		gpio_iomux_cfg(30, 1); /* PMIC_SPI_SCK */
		gpio_iomux_cfg(31, 1); /* PMIC_SPI_MOSI */
		gpio_iomux_cfg(32, 1); /* PMIC_SPI_MISO */
		gpio_iomux_cfg(33, 1); /* WATCHDOG */
	
		/* ------------ MSDC ----------- */
		/* msdc0 pads - even though works without it !! its woowo magic !! */
		gpio_iomux_cfg(41, 1); /* CLK */
		gpio_iomux_cfg(42, 1); /* CMD */
		gpio_iomux_cfg(43, 1); /* DAT0 */
		gpio_iomux_cfg(44, 1); /* DAT1 */
		gpio_iomux_cfg(45, 1); /* DAT2 */
		gpio_iomux_cfg(46, 1); /* DAT3 */
		gpio_iomux_cfg(47, 1); /* DAT4 */
		gpio_iomux_cfg(48, 1); /* DAT5 */
		gpio_iomux_cfg(49, 1); /* DAT6 */
		gpio_iomux_cfg(50, 1); /* DAT7 */
		gpio_iomux_cfg(51, 1); /* RSTB */
		
		/* msdc1 pads - just in case !! */
		gpio_iomux_cfg(52, 1); /* CLK */
		gpio_iomux_cfg(53, 1); /* CMD */
		gpio_iomux_cfg(57, 1); /* DAT0 */
		gpio_iomux_cfg(56, 1); /* DAT1 */
		gpio_iomux_cfg(55, 1); /* DAT2 */
		gpio_iomux_cfg(54, 1); /* DAT3 */
		
		/* woowo magic ! msdc0 - apparenly without it Linux fail to set 8bit ! and 4bit !  */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(2),  0, 0x7ff, 0x100); /* a)pull up CLK */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(3),  0, 0x7ff, 0x6ff); /* a)??? */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(4),  0, 0x7ff, 0x100); /* a)enable pull of CLK */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(11), 0, 0x0fc, 0x000); /* a)??? */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(12), 0, 0x0f0, 0x0a0); /* a)??? */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(14), 0, 0x7ff, 0x000); /* a)??? */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(7),  0, 0x3ff, 0x3ff); /* b)schmitt triffer for all pin except clk! */
		/* c) TODO */
		
		/* woowo magic ! msdc1 - apparenly without it SD Card does not work ! */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(2),  0, 0x3f, 0x01); /* a)pull up CLK */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(3),  0, 0x3f, 0x3e); /* a)??? */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(4),  0, 0x3f, 0x01); /* a)enable pull of CLK */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(11), 0, 0x3f, 0x0c); /* a)??? */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(12), 0, 0x0f, 0x0a); /* a)??? */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(13), 0, 0x3f, 0x00); /* a)??? */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(7),  0, 0x3f, 0x3e); /* b)schmitt triffer for all pin except clk! */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(9),  0, 0xfff, 0x249); /* c)drive strength? -> 1,2,0,1,2,0 / 1,1,1,1 ? */
	
		/* ------------ AUDIO ----------- */
		/* Make them PMIC audio pins! */
		gpio_iomux_cfg(26, 1); /* AUD_CLK */
		gpio_iomux_cfg(27, 1); /* AUD_DAT_MOSI */
		gpio_iomux_cfg(28, 1); /* AUD_DAT_MISO */
		
	#if 0 /* Done by Linux */
		/* Make them I2S0 Pins! */
		gpio_iomux_cfg(74, 6); /* G74 =>I2S0_WS */
		gpio_iomux_cfg(75, 6); /* G75 =>I2S0_BCK */
		gpio_iomux_cfg(76, 6); /* G76 =>I2S0_DI */
		
		/* ------------ etc ----------- */
		/* i2c pins as i2c function */
		gpio_iomux_cfg(58, 1);
		gpio_iomux_cfg(59, 1);
		gpio_iomux_cfg(60, 1);
		gpio_iomux_cfg(61, 1);
		gpio_iomux_cfg(62, 1);
		gpio_iomux_cfg(63, 1);
		
		/* lcm pins */
		gpio_iomux_cfg(66, 1); /* DISP_PWM */
		gpio_iomux_cfg(68, 1); /* DSI_TE */
		gpio_iomux_cfg(70, 1); /* LCM_RST */
	#endif
	}
	
	/* PMIC woowo Magic */ {
		xprintf("pwrap_init------%d\n", pwrap_init());
		
		/* disable TOPRGU wdt */
		//reg32_wsmask(TOPRGU_BASE+0x000, 0, (0xff<<24)|(1<<0), (0x22<<24)|(0<<0));
	
	#if 0 /*already done in preloader!*/
		/**** Do PMIC setup as in Preloader */
		pmic_wsmask(0x4c, 4, 1, 1);
		
		/* turn off usbdl (so the pmic wont force voltage on VBAT) */
		pmic_wsmask(0x20, 3, 1, 0); /* USBDL_SET=0 */
		pmic_wsmask(0x20, 2, 1, 1); /* USBDL_RST=1 */
		
		pmic_wsmask(0x11a, 1, 1, 1); /* SYSRSTB_EN=1 */
		pmic_wsmask(0x11a, 3, 1, 1); /* NEWLDO_RSTB_EN=1 */
		
		pmic_wsmask(0x011a, 6, 1, 1); /* PWRKEY_RST_EN=1 */
		pmic_wsmask(0x011a, 5, 1, 0); /* HOMEKEY_RST_EN=0 */
		pmic_wsmask(0x011a, 8, 3, 1); /* PWRKEY_RST_TD=1 */
		
		/* try make the bbwakeup signal go up!  OK THIS ACTUALLY WORKS!  at least for now ... */
		pmic_write(0x8036, 0x586a);
		pmic_rtc_write_trigger();
		pmic_write(0x8036, 0x9136);
		pmic_rtc_write_trigger();
		
		pmic_wsmask(0x802c, 6, 1, 1); // Bypass Power Key thing
		pmic_rtc_write_trigger();
		
		pmic_write(0x8000, 0x430d);
		pmic_rtc_write_trigger();
		
		pmic_wsmask(0x803a, 8, 0x7, 0x5);
		pmic_rtc_write_trigger();
		
		pmic_wsmask(0x802e, 7, 1, 1);
		pmic_rtc_write_trigger();
		
		#if 0
		/* enable cherger */
		if (get_charger_presence()) {
			xputs("Cherge is Insetion, Enable Chargering...\n");
		
			pmic_wsmask(0x0002, 4, 0xf, 0xb);
			pmic_wsmask(0x0000, 0, 1, 1);
			pmic_wsmask(0x0004, 3, 1, 1);
			pmic_wsmask(0x002E, 2, 1, 1);
			if (pmic_rsmask(0x0000, 7, 1)) {
				xputs("Cherger OV, Turn off Cherger !\n");
				pmic_wsmask(0x0000, 4, 1, 0);
			} else {
				xputs("Cherger not OV, ok !\n");
				
				pmic_wsmask(0x0008, 0, 0xf, 0x6); //1A cherger current !
				
				pmic_wsmask(0x002A, 0, 0x7, 0x4);
				pmic_wsmask(0x002A, 4, 0x7, 0x1);
				pmic_wsmask(0x0028, 0, 0x7, 0x1);
				pmic_wsmask(0x0028, 4, 0x7, 0x2);
				pmic_wsmask(0x001A, 0, 0xf, 0x0);
				pmic_wsmask(0x001E, 0, 1, 1);
				pmic_wsmask(0x001A, 4, 1, 1);
				pmic_wsmask(0x001E, 1, 1, 1);
				pmic_wsmask(0x0000, 3, 1, 1);
				pmic_wsmask(0x002E, 6, 1, 1);
				pmic_wsmask(0x0000, 4, 1, 1);
			}
		}
		#endif
	#endif
		
	#if 0
		/* vcn18 enable */
		pmic_wsmask(0x512, 1, 1, 0);
		pmic_wsmask(0x512, 14, 1, 1);
		
		/* vcn28 enable */
		pmic_wsmask(0x41c, 1, 1, 0);
		pmic_wsmask(0x41e, 12, 1, 1);
		
		/* vcn33_bt / vcn33_wifi enable */
		pmic_wsmask(0x416, 2, 3, 0); /* 3.3v */
		pmic_wsmask(0x420, 1, 1, 0);
		pmic_wsmask(0x416, 7, 1, 1); /* vcn33_bt */
		pmic_wsmask(0x418, 12, 1, 1); /* vcn33_wifi */
	#endif
		
		/* set vproc to 1.25v */
		pmic_wsmask(0x021e, 0, 0x7f, (1250000 - 700000) / 6250); //1 volt [For less current consumption!]
	}
	
	/* power up some stuff */ {
		/* en sleep regs control */
		reg32_write(SLEEP_BASE+0x000, (0xb16<<16)|(1<<0));
		
		/* ---------- power up bootrom ---------- */
		reg32_wsmask(INFRACFG_AO_BASE+0x804, 31, 1, 0);
		
		/* ---------- power up core1 ---------- */
		reg32_wsmask(SLEEP_BASE+0x218, 2, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x218, 3, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x218, 1, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x264, 0, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x218, 6, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x218, 5, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x218, 4, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x218, 0, 1, 1);
		
		/* ---------- power up core2 ---------- */
		reg32_wsmask(SLEEP_BASE+0x21C, 2, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x21C, 3, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x21C, 1, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x26C, 0, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x21C, 6, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x21C, 5, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x21C, 4, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x21C, 0, 1, 1);
		
		/* ---------- power up core3 ---------- */
		reg32_wsmask(SLEEP_BASE+0x220, 2, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x220, 3, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x220, 1, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x274, 0, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x220, 6, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x220, 5, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x220, 4, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x220, 0, 1, 1);
		
		#if 0 /* will be done by linux! */
		/* ---------- power up MFG ---------- */
		reg32_wsmask(SLEEP_BASE+0x214, 2, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x214, 3, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x214, 4, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x214, 1, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x214, 0, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x214, 8, 0xf, 0x0);
		reg32_wsmask(INFRACFG_AO_BASE+0x220, 5, 1, 0);
		
		/* ---------- power up ISP ---------- */
		reg32_wsmask(SLEEP_BASE+0x238, 2, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x238, 3, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x238, 4, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x238, 1, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x238, 0, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x238, 8, 0xf, 0x0);
		
		/* ---------- power up DIS ---------- */
		/* (it's already powered up but why not) */
		reg32_wsmask(SLEEP_BASE+0x23C, 2, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x23C, 3, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x23C, 4, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x23C, 1, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x23C, 0, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x23C, 8, 0xf, 0x0);
		
		/* ---------- power up CONN ---------- */
		#if 0 /* It breaks Linux boot process!! */
		reg32_wsmask(SLEEP_BASE+0x280, 2, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x280, 3, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x280, 4, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x280, 1, 1, 0);
		reg32_wsmask(SLEEP_BASE+0x280, 0, 1, 1);
		reg32_wsmask(SLEEP_BASE+0x280, 8, 0x1, 0x0);
		reg32_wsmask(INFRACFG_AO_BASE+0x220, 4, 1, 0);
		reg32_wsmask(INFRACFG_AO_BASE+0x220, 8, 1, 0);
		reg32_wsmask(INFRACFG_AO_BASE+0x220, 9, 1, 0);
		
		/* why not */
		memcpy((void*)0x18080000, KagaminText, sizeof(KagaminText));
		#endif
		#endif
	}
	
	/* Clocks! */ {
		/* -------------------------------------------------------- */
		
		/*
		clksel:
		  0->CLKSQ (26 MHz clock)
		  1->ARMPLL
		  2->UNIVPLL
		  3->MAINPLL / 2
		
		clkdiv:
		  08 => 4/4 [1/1]
		  09 => 3/4
		  0A => 2/4
		  0B => 1/4
		  
		  10 => 5/5 [1/1]
		  11 => 4/5
		  12 => 3/5
		  13 => 2/5
		  14 => 1/5
		  
		  18 => 6/6 [1/1]
		  19 => 5/6
		  1A => 4/6
		  1B => 3/6
		  1C => 2/6
		  1D => 1/6
		  
		  others => 1/1
		*/
		
		/* --------- ARM --------- */
		/* sel mainpll */
		reg32_wsmask(0x10001000, 2, 0x3, 0x3);
		
		/* set freq ! => 1300 MHz */
		reg32_write(0x10018104, (1<<31) | (0<<24) | ((1300 * 16384 / 26)));
		usleep(1000);
		
		/* sel armpll * 4 / 4 */
		reg32_wsmask(0x10001008, 0, 0x1f, 0x08); //325 MHz [For less current consumption!]
		reg32_wsmask(0x10001000, 2, 0x3,  0x1);
		
		/* --------- MFG --------- */
		/* set the MFGSYS clock to 416 MHz */
		reg32_wsmask(TOPCKGEN_BASE+0x000, 8, 0x7, 0x2);
		/* set the MFG_GFMUX into the 500.5 Mhz clock */
		reg32_wsmask(TOPCKGEN_BASE+0x004, 8, 0x38, 0x10);
		/* gate MFG clock thing */
		reg32_write(TOPCKGEN_BASE+0x080, (1<<2));
		/* gate G3D clock thing */
		//reg32_write(0x13000000+0x008, (1<<0)); // done by linux
		
		/* --------- MSDC -------- */
		reg32_wsmask(TOPCKGEN_BASE+0x000, 11, 0x7, 0x7);
		reg32_wsmask(TOPCKGEN_BASE+0x000, 20, 0x7, 0x7);
	}
	
	/* Audio */ {
		/* power up all the AFE stuff! */
		reg32_wsmask(AUDIO_BASE+AUDIO_TOP_CON0, 2,  1, 0); /* PDN_AFE_EN_BIT */
		reg32_wsmask(AUDIO_BASE+AUDIO_TOP_CON0, 6,  1, 0); /* PDN_I2S_EN_BIT */
		reg32_wsmask(AUDIO_BASE+AUDIO_TOP_CON0, 24, 1, 0); /* PDN_ADC_EN_BIT */
		reg32_wsmask(AUDIO_BASE+AUDIO_TOP_CON0, 25, 1, 0); /* PDN_DAC_EN_BIT */
		
		/* why not */
		//memcpy((void *)(AUDIO_BASE + 0x1000), KagaminText, strlen(KagaminText));
	
		/*==================== Interconnect ==================*/
		/* reset all intercons */
		reg32_write(AUDIO_BASE+AUDIO_AFE_CONN0,       0);
		reg32_write(AUDIO_BASE+AUDIO_AFE_CONN1,       0);
		reg32_write(AUDIO_BASE+AUDIO_AFE_CONN2,       0);
		reg32_write(AUDIO_BASE+AUDIO_AFE_CONN3,       0);
		reg32_write(AUDIO_BASE+AUDIO_AFE_CONN4,       0);
		reg32_write(AUDIO_BASE+AUDIO_AFE_GAIN1_CONN,  0);
		reg32_write(AUDIO_BASE+AUDIO_AFE_GAIN2_CONN,  0);
		reg32_write(AUDIO_BASE+AUDIO_AFE_GAIN2_CONN2, 0);

		/* then we do the interconns! */
		/* DL1 -> AWB */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_CONN2, 19, 1, 0);
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_CONN2, 24, 1, 0);
		/* DL1 -> DAC */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_CONN1, 21, 1, 1);
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_CONN2, 6,  1, 1);
		/* ADC -> VUL */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_CONN3, 0,  1, 1);
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_CONN3, 3,  1, 1);
		/* ADC -> AWB */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_CONN2, 18, 1, 1);
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_CONN2, 23, 1, 1);
		/* I2S_I -> AWB */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_CONN2, 16, 1, 1);
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_CONN2, 22, 1, 1);
		
		/*===================== Config  ===================*/
		/* -------- I2S1 -------- */
		/* i2s1 (OUT - I2S1 pad) */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON1, 31, 1, 0); /* lr swap =0 (0:dont 1:do) */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON1, 8, 0xf, 0xa); /* 48000 hz */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON1, 5, 1, 0); /* invert lrck =0 (0:dont 1:do) */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON1, 3, 1, 0); /* format =0 (0:eiaj 1:i2s) */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON1, 1, 1, 0); /* wlen =0 (0:16bit 1:32bit?) */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON1, 0, 1, 1); /* enable */

		/* adc */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_ADDA_TOP_CON0, 0, 1, 0); /* use int adc */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_ADDA_UL_SRC_CON0, 0, 1, 1); /* en i2s adc */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_ADDA_UL_SRC_CON0, 17, 0x3, 0x3); /* Voice mode [1] 48000hz */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_ADDA_UL_SRC_CON0, 19, 0x3, 0x3); /* Voice mode [2] 48000hz */
		reg32_write(AUDIO_BASE+AUDIO_AFE_ADDA_NEWIF_CFG0, 0x03f87201); /* 8x txif sat on */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_ADDA_NEWIF_CFG1, 10, 0x3, 0x3);
		/* dac */
		reg32_write(AUDIO_BASE+AUDIO_AFE_ADDA_DL_SRC2_CON0, (0x8<<28)|(0x3<<24)|(0x03<<11)|(0x00<<5)|(0x1<<1));
		reg32_write(AUDIO_BASE+AUDIO_AFE_ADDA_DL_SRC2_CON1, 0xf74f0000);
		/* overall */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_ADDA_DL_SRC2_CON0, 0, 1, 1); /* Enable the MTK_IF ?? */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_ADDA_UL_DL_CON0, 0, 1, 1); /* en i2s for MTK_IF ?? */

		/* -------- I2S2 -------- */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_DAC_CON1, 8, 0xf, 0xa); /* 48000 hz */
		
		/* i2s2 (IN - I2S0 pad) */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON, 31, 1, 1); /* phase shift fix */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON, 28, 1, 1); /* in pad sel =1 (0:connsys/1:iomux) */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON, 5, 1, 0); /* invert lrck =0 (0:dont/1:do) */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON, 3, 1, 1); /* format =1 (0:eiaj/1:i2s) */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON, 2, 1, 0); /* slave =0 (0:master/1:slave) */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON, 1, 1, 1); /* wlen =1 (0:16bit/1:32bit?) */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_I2S_CON, 0, 1, 1); /* enable */

		/*================= Init PMIC Codec! ================*/
		/* reg init */
		pmic_write(MT6350_TOP_RST_CON_SET, 0x0004);
		pmic_write(MT6350_TOP_RST_CON_CLR, 0x0004);
		pmic_wsmask(MT6350_AUDTOP_CON0, 0, 0xf, 0x2); /* set UL_PGA_L_MUX as open */
		pmic_wsmask(MT6350_AUDTOP_CON1, 4, 0xf, 0x2); /* set UL_PGA_R_MUX as open */
		pmic_write(MT6350_AUDTOP_CON5, 0x1114); /* set audio dac bias to 50% */
		pmic_write(MT6350_AUDTOP_CON6, 0x37a2);
		pmic_write(MT6350_AUDTOP_CON6, 0x37e2); /* enable the depop mux of hp drivers */
		pmic_wsmask(0xC060, 7, 1, 1); /* b7: GPIO_inv, invserse the PMIC gpio clk */
		pmic_write(MT6350_DRV_CON2, 0x08cc);

		/* === UL/DL === */
		pmic_wsmask(MT6350_TOP_CKPDN1_CLR, 8, 1, 1); /* AUD_26M Clock power up */
		
		/* dac */
		pmic_write(MT6350_AFE_PMIC_NEWIF_CFG0, 0x0330 | (0x8<<12)); /* +48000hz */
		pmic_wsmask(MT6350_ABB_AFE_CON1, 0, 0xf, 0xa); /* DL 48000 hz */
		pmic_wsmask(MT6350_ABB_AFE_CON11, 8, 1, !pmic_rsmask(MT6350_ABB_AFE_CON11, 0, 1));
		pmic_wsmask(MT6350_ABB_AFE_CON0, 0, 1, 1); /* DL turn on */
		
		/* adc */
		pmic_write(MT6350_AFE_PMIC_NEWIF_CFG2, 0x302F | (0x3<<10)); /* +48000hz */
		pmic_wsmask(MT6350_ABB_AFE_CON1, 4, 0x1, 0x1); /* UL 48000 hz */
		pmic_wsmask(MT6350_ABB_AFE_CON11, 8, 1, !pmic_rsmask(MT6350_ABB_AFE_CON11, 0, 1));
		pmic_wsmask(MT6350_ABB_AFE_CON0, 1, 1, 1); /* UL turn on */

		/* === MIC === */
		/* mic pga gain */
		// They go in order: 0:-6dB, 1:0dB, 2:6dB, 3:12dB, 4:18dB, 5:24dB
		pmic_wsmask(MT6350_AUDTOP_CON0, 4, 0x7, 0x5); /* mic1 */
		pmic_wsmask(MT6350_AUDTOP_CON1, 8, 0x7, 0x5); /* mic2 */

		/* adc mic thing */
		pmic_wsmask(MT6350_AUDTOP_CON0, 7, 0xFF, 0xF0); /* enable lch 1.4v, 2.4v */
		pmic_wsmask(MT6350_AUDTOP_CON2, 0, 0xFF, 0xF0); /* enable rch 1.4v, 2.4v */

		pmic_wsmask(MT6350_AUDTOP_CON0, 0, 0xf, 0x0); /* left [main mic] */
		pmic_wsmask(MT6350_AUDTOP_CON3, 9, 1, 0);
		pmic_wsmask(MT6350_AUDTOP_CON1, 4, 0xf, 0x1); /* right [headset mic] */

		pmic_wsmask(MT6350_AUDTOP_CON8, 3, 1, 1); /* micbias */
		pmic_wsmask(MT6350_AUDTOP_CON0, 7, 0x3, 0x3); /* enable lch adc, pga */
		pmic_wsmask(MT6350_AUDTOP_CON2, 0, 0x3, 0x3); /* enable rch adc, pga */

		/* === SPK / HP === */
		pmic_write(MT6350_ABB_AFE_CON3, 0); /* lch compensation */
		pmic_write(MT6350_ABB_AFE_CON4, 0); /* rch compensantion */
		pmic_wsmask(MT6350_ABB_AFE_CON10, 0, 1, 1); /* enable dc compensation */
		pmic_wsmask(MT6350_ABB_AFE_CON11, 9, 1, !pmic_rsmask(MT6350_ABB_AFE_CON11, 1, 1)); /* dc compensantion trigger */

		pmic_write(MT6350_AUDTOP_CON6, 0xf7f2); /* enable input short of hp to prevent voice signal leakage; enable 2.4v */
		pmic_wsmask(MT6350_AUDTOP_CON0, 0, 0xf000, 0x7000); /* enable clean 1.35VCM buffer in audioUL */
		pmic_write(MT6350_AUDTOP_CON5, 0x0014); /* set rch/lch buffer gain to smallest -5dB */
		pmic_write(MT6350_AUDTOP_CON4, 0x007c); /* enable audio bias, audio dac, hp buffers */
		usleep(10000);
		pmic_write(MT6350_AUDTOP_CON6, 0xf5ba); /* hp precharge function release, disable depop mux of hp driver, disable depop vcm gen */
		pmic_write(MT6350_AUDTOP_CON5, 0x2214); /* set rch/lch buffer gain to -1dB */

		/* set headphones volume */
		//They go in order: 0:-5dB, 1:-3dB, 2:-1dB, 3:1dB, 4:3dB, 5:5dB, 6:7dB, 7:9dB
		pmic_wsmask(MT6350_AUDTOP_CON5, 12, 0x7, 0x4); /* hpoutl */
		pmic_wsmask(MT6350_AUDTOP_CON5, 8,  0x7, 0x4); /* hpoutr */
	}
	
	/* USB PHY (when you have no USB after Preloader or BootROM!) */ {
		/* ------------ This powers up the PHY ------------- */
		#if 0
		/* switch to usb function */
		reg8_wsmask(0x1111086B, 2, 1, 0);
		reg8_wsmask(0x1111086E, 0, 1, 0);
		reg8_wsmask(0x11110821, 0, 3, 0);
		/* rg_usb20_bc11_sw_en = 0 */
		reg8_wsmask(0x1111081A, 7, 1, 0);
		reg8_wsmask(0x11110822, 2, 1, 1);
		/* rg_usb20_dp_100k_en = 0 */
		reg8_wsmask(0x11110822, 0, 3, 0);
		/* otg enable */
		reg8_wsmask(0x11110820, 4, 1, 1);
		/* release force suspendm */
		reg8_wsmask(0x1111086A, 2, 1, 0);
		usleep(800);
		/* force enter device mode */
		reg8_wsmask(0x1111086C, 4, 1, 0);
		reg8_wsmask(0x1111086C, 0, 0x2f, 0x2f);
		reg8_wsmask(0x1111086D, 0, 0x3f, 0x3f);
		#endif
		
		/* ------------ This disables the PHY ------------- */
		#if 0
		/* switch to usb function */
		reg8_wsmask(0x1111086B, 2, 1, 0);
		reg8_wsmask(0x1111086E, 0, 1, 0);
		reg8_wsmask(0x11110821, 0, 3, 0);
		/* release force suspendm */
		reg8_wsmask(0x1111086A, 2, 1, 0);
		/* rg_dppulldown / rg_dmpulldonw */
		reg8_wsmask(0x11110868, 6, 3, 3);
		/* rg_xcvrsel[1:0] = 1 */
		reg8_wsmask(0x11110868, 4, 3, 0);
		reg8_wsmask(0x11110868, 4, 1, 1);
		/* rg_termsel = 1 */
		reg8_wsmask(0x11110868, 2, 1, 1);
		/* rg_datain[3:0] = 0 */
		reg8_wsmask(0x11110869, 2, 0xf, 0x0);
		/* force dp_dulldown, force dm_pulldown, force_xcversel, force_termsel */
		reg8_wsmask(0x1111086A, 0, 0xba, 0xba);
		/* rg_usb20_bc11_sw_en = 0 */
		reg8_wsmask(0x1111081A, 7, 1, 0);
		/* rg_usb20_otg_vbusscmp_en = 0 */
		reg8_wsmask(0x1111081A, 4, 1, 0);
		usleep(800);
		/* release force suspendm */
		reg8_wsmask(0x1111086A, 2, 1, 0);
		usleep(1);
		#endif
		
		/*------- This "Recovers" PHY (Acutally enables USB!!!) ------*/
		#if 0
		/* pupd_bist_en = 0 */
		reg8_wsmask(0x1111081D, 4, 1, 0);
		/* force_uart_en = 0  <-- HMM ?? */
		reg8_wsmask(0x1111086B, 2, 1, 0);
		/* rg_uart_en = 0   <--- Huh ?? */
		reg8_wsmask(0x1111086E, 0, 1, 0);
		/* furce_uart_en = 0  <--- Ohhh! */
		reg8_wsmask(0x1111086A, 2, 1, 0);
		/* ? */
		reg8_wsmask(0x11110821, 0, 3, 0);
		reg8_wsmask(0x11110868, 0, 0xf4, 0x00);
		/* rg_datain[3:0] = 0 */
		reg8_wsmask(0x11110869, 2, 0xf, 0x0);
		/* ? */
		reg8_wsmask(0x1111086A, 0, 0xba, 0x00);
		/* rg_usb20_bc11_sw_en = 0 */
		reg8_wsmask(0x1111081A, 7, 1, 0);
		/* rg_usb20_otg_vbusscmd_en = 1 */
		reg8_wsmask(0x1111081A, 4, 1, 1);
		usleep(800);
		#endif
	}
	
	/* MATTAKO!!! */ {
		xputs("Mattaku....\n");
		reg32_write(TOPCKGEN_BASE+0x084, (1<<2)); /* enable apdma clk */
		reg32_write(TOPCKGEN_BASE+0x084, (1<<3)); /* enable i2c0 clk */
		reg32_write(TOPCKGEN_BASE+0x084, (1<<4)); /* enable i2c1 clk */
		reg32_write(TOPCKGEN_BASE+0x084, (1<<16)); /* enable i2c2 clk */
		reg32_write(TOPCKGEN_BASE+0x084, (1<<1)); /* enable them clk */
		reg32_write(TOPCKGEN_BASE+0x084, (1<<5)); /* enable auxadc1 clk */
		reg32_write(TOPCKGEN_BASE+0x084, (1<<30)); /* enable aux_sw_adc clk */
		reg32_write(TOPCKGEN_BASE+0x084, (1<<31)); /* enable aux_sw_tp clk */
		
		reg32_write(APMIXED_BASE+0x600, 0<<2); /* ts thing1 sel 0 */
		reg32_write(APMIXED_BASE+0x604, 0<<0); /* ts thing2 sel 0 */
	}
	
	/* Vibrate and beep */ {
		/* dl1 */
		reg32_write(AUDIO_BASE+AUDIO_AFE_DL1_BASE, 0x88000000);
		reg32_write(AUDIO_BASE+AUDIO_AFE_DL1_END,  0x88ffffff);
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_DAC_CON1, 0, 0xf, 0x0); /* 8000 hz */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_DAC_CON1, 21, 1, 1); /* stereo */
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_DAC_CON0, 1, 1, 1); /* en mem path */
		
		{
			int16_t *ptr = (void*)reg32_read(AUDIO_BASE+AUDIO_AFE_DL1_BASE);
			
			for (int i = 0; i < 8000; i++) {
				char l = 0;
				
				if ((i % 1000) < 500) l = (i & 8);
				else                  l = (i & 4);
				
				ptr[i] = l ? 8192 : -8192;
			}
		}
		
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_DAC_CON0, 0, 1, 1); /* start */
		vibrator_enable(true);
		usleep(100000);
		vibrator_enable(false);
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_DAC_CON0, 0, 1, 0); /* stop */
		
		reg32_wsmask(AUDIO_BASE+AUDIO_AFE_DAC_CON0, 1, 1, 0); /* DIS mem path */
	}
}

const volatile uint8_t blinfo[] __attribute__((section(".blinfo"))) = {
	'E','b','i','n','a','!',  //<--sign DONT ASK
	0xd0,0x0d,0xfe,0xed,      //<--dtb size
	0xde,0xad,0xbe,0xef,      //<--kernel size
};



int main(void) {
	/* setup UART2 (again some 16550A-compatible stuff?? great!!!) */ {
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
	
	xdev_out(uputc);
	xputs("\n\e[1;40;33;7m << Mediatek NewTech MT6580 Second BootLoader!! >> \e[0m\n");
	xputs("compiled " __DATE__ " - " __TIME__ "\n\n");
	
	wallclk_init();
	preinit();
	
	int rc = 0;
	
	/* check the header for validness */
	if ((blinfo[0] != 'E') || (blinfo[1] != 'b') || /* Do it this way because the payload maker needs to find this exact sign from the start! */
	    (blinfo[2] != 'i') || (blinfo[3] != 'n') || /* so if this check-against string wil appear before the actual sign then we'll screw up !!! */
	    (blinfo[4] != 'a') || (blinfo[5] != '!')) { /* so do not complain about this! */
		xputs("\e[1;31m[!!] header sanity check failed! - invalid sign!\e[0m\n");
		
		rc = 1;
		goto failed;
	}
	
	/* get the blinfos */
	uint32_t dtb_size    = blinfo[6 ]<<24 | blinfo[7 ]<<16 | blinfo[8 ]<<8 | blinfo[9 ];
	uint32_t kernel_size = blinfo[10]<<24 | blinfo[11]<<16 | blinfo[12]<<8 | blinfo[13];
	
	/* size sanity check */
	if (dtb_size > 0x100000) {
		xprintf("\e[1;31m[!!] dtb size is bigger than 0x100000! (0x%x)\e[0m\n", dtb_size);
		
		rc = 1;
		goto failed;
	}
	
	if (kernel_size > 0x1000000) {
		xprintf("\e[1;31m[!!] kernel size is bigger than 0x1000000! (0x%x)\e[0m\n", dtb_size);
		
		rc = 1;
		goto failed;
	}
	
	/* calculate addresses */
	extern char end;
	void *kernel_addr = (void*)(((uint32_t)&end        + 0xff)               & ~0xff);
	void *dtb_addr    = (void*)(((uint32_t)kernel_addr + 0xff + kernel_size) & ~0xff);
	
	/* copy dtb into SRAM... Becaused linux doesn't like that the DTB starts/ends in some big address!!! 
	 * and do it only when we can!
	 */
	if (dtb_size <= 0x10000) {
		void *Xdtb_addr = (void*)0x100000;
		memcpy(Xdtb_addr, dtb_addr, dtb_size);
		dtb_addr = Xdtb_addr;
	}
	
	/* some infos & dumps */
	xprintf("\e[1;35m[--] DTB @ 0x%x - %d bytes\e[0m\n", dtb_addr, dtb_size);
	hexdump(dtb_addr, 64);
	
	xprintf("\e[1;35m[--] Kernel @ 0x%x - %d bytes\e[0m\n", kernel_addr, kernel_size);
	hexdump(kernel_addr, 64);
	
	/* jump */
	xprintf("\e[1;32m[**] Jumping into kernel at 0x%x, with dtb at 0x%x!\e[0m\n", kernel_addr, dtb_addr);
	
	rc = ((int (*)(int, int, void *))kernel_addr)(0, -1, dtb_addr);

failed:
	xprintf("\e[1;33m[??] kernel run end! or failed! rc=%d\e[0m\n", rc);
	
	for (;;) {
		xprintf("RTC Time=%04d-%02d-%02d %02d:%02d:%02d (%d)\n",
			pmic_read(0x8016)+1968,
			pmic_read(0x8014),
			pmic_read(0x8010),
			pmic_read(0x800e),
			pmic_read(0x800c),
			pmic_read(0x800a),
			pmic_read(0x8012));
		usleep(100000);
	}
	
	return rc;
}
