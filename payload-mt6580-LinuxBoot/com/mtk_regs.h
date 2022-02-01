#ifndef _MTKREGS_H
#define _MTKREGS_H

#include <stdint.h>

/* 8 bit */
static inline uint8_t reg8_read(uint32_t addr) {
	return *(volatile uint8_t*)addr;
}
static inline void reg8_write(uint32_t addr, uint8_t val) {
	*(volatile uint8_t*)addr = val;
}
static inline void reg8_wsmask(uint32_t addr, int shift, uint8_t mask, uint8_t val) {
	*(volatile uint8_t*)addr =
		(*(volatile uint8_t*)addr & ~(mask << shift)) | ((val & mask) << shift);
}
static inline uint8_t reg8_rsmask(uint32_t addr, int shift, uint8_t mask) {
	return (*(volatile uint8_t*)addr >> shift) & mask;
}
/* 16 bit */
static inline uint16_t reg16_read(uint32_t addr) {
	return *(volatile uint16_t*)addr;
}
static inline void reg16_write(uint32_t addr, uint16_t val) {
	*(volatile uint16_t*)addr = val;
}
static inline void reg16_wsmask(uint32_t addr, int shift, uint16_t mask, uint16_t val) {
	*(volatile uint16_t*)addr =
		(*(volatile uint16_t*)addr & ~(mask << shift)) | ((val & mask) << shift);
}
static inline uint16_t reg16_rsmask(uint32_t addr, int shift, uint16_t mask) {
	return (*(volatile uint16_t*)addr >> shift) & mask;
}
/* 32bit */
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
#define PWRAP_BASE		0x1000F000
#define IOCFG_B_BASE		0x10015000
#define IOCFG_R_BASE		0x10017000
#define APMIXED_BASE		0x10018000
#define MCUCFG_BASE		0x10200000
#define EMI_BASE		0x10205000
#define DRAMC_NAO_BASE		0x10206000
#define DRAMC0_BASE		0x10207000
#define DDRPHY_BASE		0x10208000
#define CORE_BASE		0x10210000
#define UART0_BASE		0x11005000
#define UART1_BASE		0x11006000
#define MUSB0_BASE		0x11100000
#define MSDC0_BASE		0x11120000
#define MSDC1_BASE		0x11130000
#define AUDIO_BASE		0x11140000
#define G3D_CONFIG_BASE		0x13000000
#define MMSYS_CONFIG_BASE	0x14000000

//------------ TOPCKGEN -------------//
/*
#define CLK_MUX_SEL0		(CLK_TOPCKSYS_BASE + 0x000)
#define CLK_MUX_SEL1		(CLK_TOPCKSYS_BASE + 0x004)
#define TOPBUS_DCMCTL	(CLK_TOPCKSYS_BASE + 0x008)
#define TOPEMI_DCMCTL	(CLK_TOPCKSYS_BASE + 0x00C)
#define FREQ_MTR_CTRL	(CLK_TOPCKSYS_BASE + 0x010)
#define CLK_GATING_CTRL0	(CLK_TOPCKSYS_BASE + 0x020)
#define CLK_GATING_CTRL1	(CLK_TOPCKSYS_BASE + 0x024)
#define INFRABUS_DCMCTL0	(CLK_TOPCKSYS_BASE + 0x028)
#define INFRABUS_DCMCTL1	(CLK_TOPCKSYS_BASE + 0x02C)
#define MPLL_FREDIV_EN	(CLK_TOPCKSYS_BASE + 0x030)
#define UPLL_FREDIV_EN	(CLK_TOPCKSYS_BASE + 0x034)
#define TEST_DBG_CTRL		(CLK_TOPCKSYS_BASE + 0x038)
#define CLK_GATING_CTRL2	(CLK_TOPCKSYS_BASE + 0x03C)
#define SET_CLK_GATING_CTRL0	(CLK_TOPCKSYS_BASE + 0x050)
#define SET_CLK_GATING_CTRL1	(CLK_TOPCKSYS_BASE + 0x054)
#define SET_INFRABUS_DCMCTL0	(CLK_TOPCKSYS_BASE + 0x058)
#define SET_INFRABUS_DCMCTL1	(CLK_TOPCKSYS_BASE + 0x05C)
#define SET_MPLL_FREDIV_EN	(CLK_TOPCKSYS_BASE + 0x060)
#define SET_UPLL_FREDIV_EN	(CLK_TOPCKSYS_BASE + 0x064)
#define SET_TEST_DBG_CTRL	(CLK_TOPCKSYS_BASE + 0x068)
#define SET_CLK_GATING_CTRL2	(CLK_TOPCKSYS_BASE + 0x06C)
#define CLR_CLK_GATING_CTRL0	(CLK_TOPCKSYS_BASE + 0x080)
#define CLR_CLK_GATING_CTRL1	(CLK_TOPCKSYS_BASE + 0x084)
#define CLR_INFRABUS_DCMCTL0	(CLK_TOPCKSYS_BASE + 0x088)
#define CLR_INFRABUS_DCMCTL1	(CLK_TOPCKSYS_BASE + 0x08C)
#define CLR_MPLL_FREDIV_EN	(CLK_TOPCKSYS_BASE + 0x090)
#define CLR_UPLL_FREDIV_EN	(CLK_TOPCKSYS_BASE + 0x094)
#define CLR_TEST_DBG_CTRL	(CLK_TOPCKSYS_BASE + 0x098)
#define CLR_CLK_GATING_CTRL2	(CLK_TOPCKSYS_BASE + 0x09C)
#define LPM_CTRL		(CLK_TOPCKSYS_BASE + 0x100)
#define LPM_TOTAL_TIME	(CLK_TOPCKSYS_BASE + 0x104)
#define LPM_LOW2HIGH_COUNT	(CLK_TOPCKSYS_BASE + 0x108)
#define LPM_HIGH_DUR_TIME	(CLK_TOPCKSYS_BASE + 0x10C)
#define LPM_LONGEST_HIGHTIME	(CLK_TOPCKSYS_BASE + 0x110)
#define LPM_GOODDUR_COUNT	(CLK_TOPCKSYS_BASE + 0x114)
#define CLK_GATING_CTRL0_SEN	(CLK_TOPCKSYS_BASE + 0x220)
#define CLK_GATING_CTRL1_SEN	(CLK_TOPCKSYS_BASE + 0x224)
#define CLK_GATING_CTRL2_SEN	(CLK_TOPCKSYS_BASE + 0x23C)
*/



//-------------- GPIO ---------------//
#define GPIO_DIRn(n)				(0x000 + 0xC * (n))
#define GPIO_DIRn_SET(n)			(0x004 + 0xC * (n))
#define GPIO_DIRn_CLR(n)			(0x008 + 0xC * (n))
#define GPIO_OUTn(n)				(0x024 + 0xC * (n))
#define GPIO_OUTn_SET(n)			(0x028 + 0xC * (n))
#define GPIO_OUTn_CLR(n)			(0x02C + 0xC * (n))
#define GPIO_INn(n)				(0x048 + 0x4 * (n))
#define GPIO_MODEn(n)				(0x054 + 0x10 * (n))
#define GPIO_MODEn_SET(n)			(0x058 + 0x10 * (n))
#define GPIO_MODEn_CLR(n)			(0x05C + 0x10 * (n))

//-------------- APXGPT -------------//
#define APXGPT_IRQEN				0x000
#define APXGPT_IRQSTA				0x004
#define APXGPT_IRQACK				0x008
#define APXGPT_CON(n)				(0x010 + (0x10 * (n)))
#define APXGPT_CLK(n)				(0x014 + (0x10 * (n)))
#define APXGPT_CNT(n)				(0x018 + (0x10 * (n)))
#define APXGPT_CMP(n)				(0x01C + (0x10 * (n)))
/* The 6th timer is routed to the ARM Arch Timer !!!!! */

//--------------- MUSB --------------//
#define MUSB_FADDR				0x0000
#define MUSB_POWER				0x0001
#define MUSB_INTRTX				0x0002
#define MUSB_INTRRX				0x0004
#define MUSB_INTRTXE				0x0006
#define MUSB_INTRRXE				0x0008
#define MUSB_INTRUSB				0x000A
#define MUSB_INTRUSBE				0x000B
#define MUSB_FRAME				0x000C
#define MUSB_INDEX				0x000E
#define MUSB_TESTMODE				0x000F

//-------------- PWRAP --------------//
#define PWRAP_MUX_SEL				0x000
#define PWRAP_WRAP_EN				0x004
#define PWRAP_DIO_EN				0x008
#define PWRAP_SIDLY				0x00C
#define PWRAP_RDDMY				0x018
#define PWRAP_SI_CK_CON				0x01C
#define PWRAP_HIPRIO_ARB_EN			0x050
#define PWRAP_MAN_EN				0x05C
#define PWRAP_MAN_CMD				0x060
#define PWRAP_WACS2_EN				0x094
#define PWRAP_INIT_DONE2			0x098
#define PWRAP_WACS2_CMD				0x09C
#define PWRAP_WACS2_RDATA			0x0A0
#define PWRAP_WACS2_VLDCLR			0x0A4
#define PWRAP_DCM_EN				0x13C
#define PWRAP_DCM_DBC_PRD			0x140
#define PWRAP_SWRST				0x180

//-------------- IOCFG --------------//
#define IOCFG_XXXn(n)				(0x000 + 0xC * (n))
#define IOCFG_XXXn_SET(n)			(0x004 + 0xC * (n))
#define IOCFG_XXXn_CLR(n)			(0x008 + 0xC * (n))

//--------------- MSDC --------------//
#define MSDC_CFG				0x000
#define MSDC_IOCON				0x004
#define MSDC_PS					0x008
#define MSDC_INT				0x00C
#define MSDC_INTEN				0x010
#define MSDC_FIFOCS				0x014
#define MSDC_TXDATA				0x018
#define MSDC_RXDATA				0x01C
#define MSDC_SDC_CFG				0x030
#define MSDC_SDC_CMD				0x034
#define MSDC_SDC_ARG				0x038
#define MSDC_SDC_STS				0x03C
#define MSDC_SDC_RESP0				0x040
#define MSDC_SDC_RESP1				0x044
#define MSDC_SDC_RESP2				0x048
#define MSDC_SDC_RESP3				0x04C
#define MSDC_SDC_BLK_NUM			0x050
#define MSDC_SDC_CSTS				0x058
#define MSDC_SDC_CSTS_EN			0x05C
#define MSDC_SDC_DCRC_STS			0x060
#define MSDC_EMMC_CFG0				0x070
#define MSDC_EMMC_CFG1				0x074
#define MSDC_EMMC_STS				0x078
#define MSDC_EMMC_IOCON				0x07C
#define MSDC_SDC_ACMD_RESP			0x080
#define MSDC_SDC_ACMD19_TRG			0x084
#define MSDC_SDC_ACMD19_STS			0x088
#define MSDC_DMA_SA				0x090
#define MSDC_DMA_CA				0x094
#define MSDC_DMA_CTRL				0x098
#define MSDC_DMA_CFG				0x09C
#define MSDC_DBG_SEL				0x0A0
#define MSDC_DBG_OUT				0x0A4
#define MSDC_DMA_LEN				0x0A8
#define MSDC_PATCH_BIT				0x0B0
#define MSDC_PATCH_BIT1				0x0B4
#define MSDC_DAT0_TUNE_CRC			0x0C0
#define MSDC_DAT1_TUNE_CRC			0x0C4
#define MSDC_DAT2_TUNE_CRC			0x0C8
#define MSDC_DAT3_TUNE_CRC			0x0CC
#define MSDC_CMD_TUNE_CRC			0x0D0
#define MSDC_SDIO_TUNE_WIND			0x0D4
#define MSDC_PAD_CTL0				0x0E0
#define MSDC_PAD_CTL1				0x0E4
#define MSDC_PAD_CTL2				0x0E8
#define MSDC_PAD_TUNE				0x0EC
#define MSDC_DAT_RDDLY0				0x0F0
#define MSDC_DAT_RDDLY1				0x0F4
#define MSDC_HW_DBG				0x0F8
#define MSDC_VERSION				0x100
#define MSDC_ECO_VER				0x104

//--------------- AUDIO --------------//
#define AUDIO_TOP_CON0				0x000
#define AUDIO_TOP_CON1				0x004
#define AUDIO_TOP_CON2				0x008
#define AUDIO_TOP_CON3				0x00C
#define AUDIO_AFE_DAC_CON0			0x010
#define AUDIO_AFE_DAC_CON1			0x014
#define AUDIO_AFE_I2S_CON			0x018
#define AUDIO_AFE_CONN0				0x020
#define AUDIO_AFE_CONN1				0x024
#define AUDIO_AFE_CONN2				0x028
#define AUDIO_AFE_CONN3				0x02C
#define AUDIO_AFE_CONN4				0x030
#define AUDIO_AFE_I2S_CON1			0x034
#define AUDIO_AFE_I2S_CON2			0x038
#define AUDIO_AFE_DL1_BASE			0x040
#define AUDIO_AFE_DL1_CUR			0x044
#define AUDIO_AFE_DL1_END			0x048
#define AUDIO_AFE_I2S_CON3			0x04C
#define AUDIO_AFE_DL2_BASE			0x050
#define AUDIO_AFE_DL2_CUR			0x054
#define AUDIO_AFE_DL2_END			0x058
#define AUDIO_AFE_AWB_BASE			0x070
#define AUDIO_AFE_AWB_END			0x078
#define AUDIO_AFE_AWB_CUR			0x07C
#define AUDIO_AFE_VUL_BASE			0x080
#define AUDIO_AFE_VUL_END			0x088
#define AUDIO_AFE_VUL_CUR			0x08C
#define AUDIO_AFE_MEMIF_MON0			0x0D0
#define AUDIO_AFE_MEMIF_MON1			0x0D4
#define AUDIO_AFE_MEMIF_MON2			0x0D8
#define AUDIO_AFE_MEMIF_MON4			0x0E0
#define AUDIO_AFE_ADDA_DL_SRC2_CON0		0x108
#define AUDIO_AFE_ADDA_DL_SRC2_CON1		0x10C
#define AUDIO_AFE_ADDA_UL_SRC_CON0		0x114
#define AUDIO_AFE_ADDA_UL_SRC_CON1		0x118
#define AUDIO_AFE_ADDA_TOP_CON0			0x120
#define AUDIO_AFE_ADDA_UL_DL_CON0		0x124
#define AUDIO_AFE_ADDA_SRC_DEBUG		0x12C
#define AUDIO_AFE_ADDA_SRC_DEBUG_MON0		0x130
#define AUDIO_AFE_ADDA_SRC_DEBUG_MON1		0x134
#define AUDIO_AFE_ADDA_NEWIF_CFG0		0x138
#define AUDIO_AFE_ADDA_NEWIF_CFG1		0x13C
#define AUDIO_AFE_SIDETONE_DEBUG		0x1D0
#define AUDIO_AFE_SIDETONE_MON			0x1D4
#define AUDIO_AFE_SIDETONE_CON0			0x1E0
#define AUDIO_AFE_SIDETONE_COEFF		0x1E4
#define AUDIO_AFE_SIDETONE_CON1			0x1E8
#define AUDIO_AFE_SIDETONE_GAIN			0x1EC
#define AUDIO_AFE_SGEN_CON0			0x1F0
#define AUDIO_AFE_TOP_CON0			0x200
#define AUDIO_AFE_ADDA_PREDIS_CON0		0x260
#define AUDIO_AFE_ADDA_PREDIS_CON1		0x264
#define AUDIO_AFE_MOD_DAI_BASE			0x330
#define AUDIO_AFE_MOD_DAI_END			0x338
#define AUDIO_AFE_MOD_DAI_CUR			0x33C
#define AUDIO_AFE_IRQ_MCU_CON			0x3A0
#define AUDIO_AFE_IRQ_MCU_STATUS		0x3A4
#define AUDIO_AFE_IRQ_MCU_CLR			0x3A8
#define AUDIO_AFE_IRQ_MCU_CNT1			0x3AC
#define AUDIO_AFE_IRQ_MCU_CNT2			0x3B0
#define AUDIO_AFE_IRQ_MCU_MON2			0x3B8
#define AUDIO_AFE_IRQ1_MCU_CNT_MON		0x3C0
#define AUDIO_AFE_IRQ2_MCU_CNT_MON		0x3C4
#define AUDIO_AFE_IRQ1_MCU_EN_CNT_MON		0x3C8
#define AUDIO_AFE_MEMIF_MINLEN			0x3D0
#define AUDIO_AFE_MEMIF_MAXLEN			0x3D4
#define AUDIO_AFE_MEMIF_PBUF_SIZE		0x3D8
#define AUDIO_AFE_GAIN1_CON0			0x410
#define AUDIO_AFE_GAIN1_CON1			0x414
#define AUDIO_AFE_GAIN1_CON2			0x418
#define AUDIO_AFE_GAIN1_CON3			0x41C
#define AUDIO_AFE_GAIN1_CONN			0x420
#define AUDIO_AFE_GAIN1_CUR			0x424
#define AUDIO_AFE_GAIN2_CON0			0x428
#define AUDIO_AFE_GAIN2_CON1			0x42C
#define AUDIO_AFE_GAIN2_CON2			0x430
#define AUDIO_AFE_GAIN2_CON3			0x434
#define AUDIO_AFE_GAIN2_CONN			0x438
#define AUDIO_AFE_GAIN2_CUR			0x43C
#define AUDIO_AFE_GAIN2_CONN2			0x440
#define AUDIO_FPGA_CFG2				0x4B8
#define AUDIO_FPGA_CFG3				0x4BC
#define AUDIO_FPGA_CFG0				0x4C0
#define AUDIO_FPGA_CFG1				0x4C4
#define AUDIO_FPGA_VER				0x4C8
#define AUDIO_AFE_ASRC_CON0			0x500
#define AUDIO_AFE_ASRC_CON1			0x504
#define AUDIO_AFE_ASRC_CON2			0x508
#define AUDIO_AFE_ASRC_CON3			0x50C
#define AUDIO_AFE_ASRC_CON4			0x510
#define AUDIO_AFE_ASRC_CON5			0x514
#define AUDIO_AFE_ASRC_CON6			0x518
#define AUDIO_AFE_ASRC_CON7			0x51C
#define AUDIO_AFE_ASRC_CON8			0x520
#define AUDIO_AFE_ASRC_CON9			0x524
#define AUDIO_AFE_ASRC_CON10			0x528
#define AUDIO_AFE_ASRC_CON11			0x52C
#define AUDIO_PCM_INTF_CON			0x530
#define AUDIO_PCM_INTF_CON2			0x538
#define AUDIO_PCM2_INTF_CON			0x53C
#define AUDIO_AFE_ASRC_CON13			0x550
#define AUDIO_AFE_ASRC_CON14			0x554
#define AUDIO_AFE_ASRC_CON15			0x558
#define AUDIO_AFE_ASRC_CON16			0x55C
#define AUDIO_AFE_ASRC_CON17			0x560
#define AUDIO_AFE_ASRC_CON18			0x564
#define AUDIO_AFE_ASRC_CON19			0x568
#define AUDIO_AFE_ASRC_CON20			0x56C
#define AUDIO_AFE_ASRC_CON21			0x570

#endif
