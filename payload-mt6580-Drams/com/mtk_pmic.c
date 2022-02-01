#include <xprintf.h>
#include <wallclk.h>
#include "mtk_regs.h"
#include "mtk_pmic.h"

/* ============== PWRAP ============ */
int pwrap_init(void) {
	reg32_write(TOPCKGEN_BASE+0x054, (1<<20)|(1<<27)|(1<<28)|(1<<29));
		
	/******* toggle module reest */
	reg32_write(PWRAP_BASE+PWRAP_SWRST, 1);
	reg32_wsmask(TOPRGU_BASE+0x018, 0, (0xff<<24)|(1<<11), (0x88<<24)|(1<<11));
	reg32_wsmask(TOPRGU_BASE+0x018, 0, (0xff<<24)|(1<<11), (0x88<<24)|(0<<11));
	reg32_write(PWRAP_BASE+PWRAP_SWRST, 0);
	
	/******* turn on module clock */
	reg32_write(TOPCKGEN_BASE+0x084, (1<<20)|(1<<27)|(1<<28)|(1<<29));
	
	/******* turn on module clock dcm */
	reg32_write(TOPCKGEN_BASE+0x05C, (1<<2)|(1<<1));
	
	/******* set spi_ck_freq = 26mhz */
	reg32_wsmask(TOPCKGEN_BASE+0x000, 24, 0x7, 0x3);
	
	/******* enable dcm */
	reg32_write(PWRAP_BASE+PWRAP_DCM_EN, 1);
	reg32_write(PWRAP_BASE+PWRAP_DCM_DBC_PRD, 0);
	
	/******* reset spislv */
	reg32_write(PWRAP_BASE+PWRAP_HIPRIO_ARB_EN, 0);
	reg32_write(PWRAP_BASE+PWRAP_WRAP_EN, 0);
	reg32_write(PWRAP_BASE+PWRAP_MUX_SEL, 1);
	reg32_write(PWRAP_BASE+PWRAP_MAN_EN, 1);
	reg32_write(PWRAP_BASE+PWRAP_DIO_EN, 0);

	reg32_write(PWRAP_BASE+PWRAP_MAN_CMD, (1<<13)|(1<<8));
	reg32_write(PWRAP_BASE+PWRAP_MAN_CMD, (1<<13)|(8<<8));
	reg32_write(PWRAP_BASE+PWRAP_MAN_CMD, (1<<13)|(0<<8));
	reg32_write(PWRAP_BASE+PWRAP_MAN_CMD, (1<<13)|(8<<8));
	reg32_write(PWRAP_BASE+PWRAP_MAN_CMD, (1<<13)|(8<<8));
	reg32_write(PWRAP_BASE+PWRAP_MAN_CMD, (1<<13)|(8<<8));
	reg32_write(PWRAP_BASE+PWRAP_MAN_CMD, (1<<13)|(8<<8));
	
	for (int i = 1000; i >= 0; i--) {
		if (i == 0) {
			xputs("[PWRAP-INIT] Wait for the FSM IDLE is Over!\n");
			return -1;
		}
		
		if (reg32_rsmask(PWRAP_BASE+PWRAP_WACS2_RDATA, 16, 7) == 0)
			break;
	
		usleep(10);
	}

	reg32_write(PWRAP_BASE+PWRAP_MUX_SEL, 0);
	reg32_write(PWRAP_BASE+PWRAP_MAN_EN, 0);

	/******** enable wacs2 */
	reg32_write(PWRAP_BASE+PWRAP_WRAP_EN, 1);
	reg32_write(PWRAP_BASE+PWRAP_HIPRIO_ARB_EN, (1<<3));
	reg32_write(PWRAP_BASE+PWRAP_WACS2_EN, 1);
	
	/******** set dummy cucle */
	reg32_write(PWRAP_BASE+PWRAP_RDDMY, 0xf);
	
	/******** init sistrobe */
	{
		uint32_t sistrobe_good = 0;
		
		for (int i = 0; i < 24; i++) {
			reg32_write(PWRAP_BASE+PWRAP_SI_CK_CON, (i >> 2) & 7);
			reg32_write(PWRAP_BASE+PWRAP_SIDLY, ~i & 3);
			
			int t = pmic_read(0x018c);
			//xprintf("Test%-2d=%04x\n", i, t);
			if (t < 0) {
				xprintf("[PWRAP-INIT] Failed to read the Test Register !\n");
				return -1;
			}
			
			if (t == 0x5aa5) sistrobe_good |= (1<<i);
		}
		
		//xprintf("Good====> %08x\n", sistrobe_good);
		
		int sistrobe_s = -1, sistrobe_e = -1;
		for (int i = 0;  i < 32; i++) if (sistrobe_good & (1<<i)) sistrobe_s = i;
		for (int i = 31; i >= 0; i--) if (sistrobe_good & (1<<i)) sistrobe_e = i;
		
		//xprintf("Good -> %d~%d\n", sistrobe_s, sistrobe_e);
		if ((sistrobe_s < 0) || (sistrobe_e < 0)) {
			xputs("[PWRAP-INIT] Failed to calculate the good sistrobe range !!\n");
			return -1;
		}
		
		int sistrobe_m = ((sistrobe_e - sistrobe_s) / 2) + sistrobe_s;
		//xprintf("Middle point ===%d\n", sistrobe_m);

		reg32_write(PWRAP_BASE+PWRAP_SI_CK_CON, (sistrobe_m >> 2) & 7);
		reg32_write(PWRAP_BASE+PWRAP_SIDLY, ~sistrobe_m & 3);
		
		//xprintf("TEST REGISTER===%04x\n", pmic_read(0x018c));
	}
	
	/****** spi waveform config */
	
	/****** enable dio mode */
	pmic_write(0x018A, 1);
	reg32_write(PWRAP_BASE+PWRAP_DIO_EN, 1);
	
	/****** fdsfs */
	//reg32_write(PWRAP_BASE+
	
	/****** init done */
	reg32_write(PWRAP_BASE+PWRAP_INIT_DONE2, 1);
	
	return 0;
}

int pwrap_wacs2_io(uint16_t addr, bool write, uint16_t val) {
	for (int i = 1000; i >= 0; i--) {
		if (i == 0) {
			xprintf("[PWRAP WACS2 IO] Wait for the FSM IDLE is Over!\n");
			//errno = ETIMEDOUT;
			return -1;
		}

		if (reg32_rsmask(PWRAP_BASE+PWRAP_WACS2_RDATA, 16, 7) == 0)
			break;
		
		usleep(10);
	}

	reg32_write(PWRAP_BASE+PWRAP_WACS2_CMD, (write<<31)|((addr>>1)<<16)|(val<<0));

	if (!write) {
		for (int i = 1000; i >= 0; i--) {
			if (i == 0) {
				xprintf("[PWRAP WACS2 IO] Wait for the FSM WFVLDCLR is Over!\n");
				//errno = ETIMEDOUT;
				return -1;
			}
	
			if (reg32_rsmask(PWRAP_BASE+PWRAP_WACS2_RDATA, 16, 7) == 6)
				break;
	
			usleep(10);
		}

		val = reg32_rsmask(PWRAP_BASE+PWRAP_WACS2_RDATA, 0, 0xffff);
		reg32_write(PWRAP_BASE+PWRAP_WACS2_VLDCLR, 1);
	}

	return val;
}

/* ============== PMIC COMMON ============ */
int pmic_read(uint16_t addr) {
	return pwrap_wacs2_io(addr, false, 0);
}

int pmic_write(uint16_t addr, uint16_t val) {
	return pwrap_wacs2_io(addr, true, val);
}

int pmic_wsmask(uint16_t addr, int shift, uint16_t mask, uint16_t val) {
	int rd = pmic_read(addr);
	if (rd < 0) return rd;
	return pmic_write(addr, (rd & ~(mask << shift)) | ((val & mask) << shift));
}

int pmic_rsmask(uint16_t addr, int shift, uint16_t mask) {
	int rd = pmic_read(addr);
	if (rd < 0) return rd;
	return (rd >> shift) & mask;
}

/* ============== PMIC SPECIFICS ============ */
int pmic_rtc_write_trigger(void) {
	int rc = pmic_write(0x803c, 1); /* write trigger */
	if (rc < 0) {
		//loge("PMIC RTC", "Failed to write the Write Trigger register! %d", rc);
		return rc;
	}
	
	for (int i = 1000; i >= 0; i--) {
		if (i == 0) {
			//loge("PMIC RTC", "Wait for the RTC CBUSY timed out !");
			//errno = ETIMEDOUT;
			return -1;
		}
		
		rc = pmic_rsmask(0x8000, 6, 1);
		if (rc < 0) {
			//loge("PMIC RTC", "Read status reg Fail %d!", rc);
			return rc;
		}
		
		if (!rc)
			break;
	}
	
	return 0;
}
