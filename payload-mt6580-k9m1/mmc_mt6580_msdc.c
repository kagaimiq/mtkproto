/*
 * Disk Driver for the MT6580's MSDC
 *
 * by Kagami Hiiragi, 2021-11-19
 *
 *  based off the mmc_lpc23xx_mci.c by ChaN
 *   (FatFs - http://elm-chan.org/fsw/ff/00index_e.html)
 */
#include "ff.h"
#include "diskio.h"
#include <string.h>
#include <mtk_regs.h>
#include <wallclk.h>
#include <xprintf.h>

typedef struct {
	volatile uint32_t CFG;
	volatile uint32_t IOCON;
	volatile uint32_t PS;
	volatile uint32_t INT;
	volatile uint32_t INTEN;
	volatile uint32_t FIFOCS;
	volatile uint32_t TXDATA;
	volatile uint32_t RXDATA;
	uint32_t Reserved_20[4];
	volatile uint32_t SDC_CFG;
	volatile uint32_t SDC_CMD;
	volatile uint32_t SDC_ARG;
	volatile uint32_t SDC_STS;
	volatile uint32_t SDC_RESP0;
	volatile uint32_t SDC_RESP1;
	volatile uint32_t SDC_RESP2;
	volatile uint32_t SDC_RESP3;
	volatile uint32_t SDC_BLK_NUM;
	uint32_t Reserved_54;
	volatile uint32_t SDC_CSTS;
	volatile uint32_t SDC_CSTS_EN;
	volatile uint32_t SDC_DCRC_STS;
	uint32_t Reserved_64[3];
	volatile uint32_t EMMC_CFG0;
	volatile uint32_t EMMC_CFG1;
	volatile uint32_t EMMC_STS;
	volatile uint32_t EMMC_IOCON;
	volatile uint32_t SDC_ACMD_RESP;
	volatile uint32_t SDC_ACMD19_TRG;
	volatile uint32_t SDC_ACMD19_STS;
	uint32_t Reserved_8C;
	volatile uint32_t DMA_SA;
	volatile uint32_t DMA_CA;
	volatile uint32_t DMA_CTRL;
	volatile uint32_t DMA_CFG;
	volatile uint32_t DBG_SEL;
	volatile uint32_t DBG_OUT;
	volatile uint32_t DMA_LEN;
	uint32_t Reserved_AC;
	volatile uint32_t PATCH_BIT;
	volatile uint32_t PATCH_BIT1;
	uint32_t Reserved_B8[2];
	volatile uint32_t DAT0_TUNE_CRC;
	volatile uint32_t DAT1_TUNE_CRC;
	volatile uint32_t DAT2_TUNE_CRC;
	volatile uint32_t DAT3_TUNE_CRC;
	volatile uint32_t CMD_TUNE_CRC;
	volatile uint32_t SDIO_TUNE_WIND;
	uint32_t Reserved_D8[2];
	volatile uint32_t PAD_CTL0;
	volatile uint32_t PAD_CTL1;
	volatile uint32_t PAD_CTL2;
	volatile uint32_t PAD_TUNE;
	volatile uint32_t DAT_RDDLY0;
	volatile uint32_t DAT_RDDLY1;
	volatile uint32_t HW_DBG;
	uint32_t Reserved_FC;
	volatile uint32_t VERSION;
	volatile uint32_t ECO_VER;
} msdc_regs_t;

/*
 0x11120000 -> MSDC0
 0x11130000 -> MSDC1
*/
static msdc_regs_t *msdc = (msdc_regs_t *)0x11130000;

/* ----- MMC/SDC command ----- */
#define CMD0	(0)				/* GO_IDLE_STATE */
#define CMD1	(1)				/* SEND_OP_COND (MMC) */
#define CMD2	(2)				/* ALL_SEND_CID */
#define CMD3	(3)				/* SEND_RELATIVE_ADDR */
#define CMD6	(6)				/* SWITCH */
#define ACMD6	(6|0x80)		/* SET_BUS_WIDTH (SDC) */
#define CMD7	(7)				/* SELECT_CARD */
#define CMD8	(8)				/* SEND_IF_COND (SDCv2) / SEND_EXT_CSD (MMCv4) */
#define CMD9	(9)				/* SEND_CSD */
#define CMD10	(10)			/* SEND_CID */
#define CMD12	(12)			/* STOP_TRANSMISSION */
#define CMD13	(13)			/* SEND_STATUS */
#define ACMD13	(13|0x80)		/* SD_STATUS (SDC) */
#define CMD16	(16)			/* SET_BLOCKLEN */
#define CMD17	(17)			/* READ_SINGLE_BLOCK */
#define CMD18	(18)			/* READ_MULTIPLE_BLOCK */
#define	CMD23	(23)			/* SET_BLK_COUNT (MMC) */
#define	ACMD23	(23|0x80)		/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)			/* WRITE_BLOCK */
#define CMD25	(25)			/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)			/* ERASE_ER_BLK_START */
#define CMD33	(33)			/* ERASE_ER_BLK_END */
#define CMD38	(38)			/* ERASE */
#define	ACMD41	(41|0x80)		/* SEND_OP_COND (SDC) */
#define CMD55	(55)			/* APP_CMD */

static volatile DSTATUS Stat = STA_NOINIT;

static WORD CardRCA;
static BYTE CardType;
static BYTE CardCSD[16];
static BYTE CardCID[16];
static BYTE CardOCR[4];

static WORD BlockSize;
static UINT BlockCount;

/*=========================================================================*/

static int send_cmd(UINT idx, DWORD arg, UINT rt, DWORD *buff) {
	if (idx & 0x80) {
		if (!send_cmd(CMD55, CardRCA << 16, 1, buff) || !(buff[0] & 0x00000020))
			return 0;
	}
	
	for (uint64_t st = millis();;) {
		if (millis() - st >= 1000)
			return 0;
		
		if (msdc->SDC_STS & (1<<0)) continue;
		if (msdc->SDC_STS & (1<<1)) continue;
		
		break;
	}
	
	/* ==== SDC_CMD ====
	 * b30     : VOLSWTH     [voltage switch command]
	 * b28     : AUTOCMD     [automatic command]
	 * b27~b16 : BLKLEN      [block length]
	 * b15     : GOIRQ       [go irq]
	 * b14     : STOP        [stop command]
	 * b13     : RW          [write command]
	 * b12~b11 : DTYP        [data type]
	 * b9~b7   : RSPTYP      [response type]
	 * b6      : BRK         [break]
	 * b5~b0   : OPC         [opcode]
	 *
	 * RSPTYP:
	 *   0 <- no response       [rt == 0 (none)]    None
	 *   1 <- R1, R5, R6, R7    [rt == 1 (short)]   Response, CRC, Opcode
	 *   2 <- R2                [rt == 2 (long)]    Response, CRC, 136-bit response
	 *   3 <- R3                [rt == 1 (short)]   Response
	 *   4 <- R4                [rt == 1 (short)]   Response
	 *   7 <- R1b               [rt == 1 (short)]   Respones, CRC, Opcode, Busy
	 *
	 * DTYP:
	 *   0 <- no data
	 *   1 <- single block
	 *   2 <- multiple blocks
	 */
	DWORD cmd = idx & 0x3f;
	
	/* Response type */
	switch (rt) {
	case 1: /* Short */
		switch (idx) {
		case CMD1: /* SEND_OP_COND (MMC) */
		case ACMD41: /* SEND_OP_COND (SDC) */
			cmd |= (3<<7); /* R3 */
			break;
		case CMD6: /* SWITCH */
		case CMD12: /* STOP_TRANSMISSION */
		case CMD38: /* ERASE */
			cmd |= (7<<7); /* R1b */
			break;
		default:
			cmd |= (1<<7); /* R1/R5/R6/R7 */
			break;
		}
		break;
	case 2: /* Long */
		cmd |= (2<<7); /* R2 */
		break;
	}
	
	/* Other command properties */
	switch (idx) {
	case CMD12: /* STOP_TRANSMISSION */
		cmd |= (1<<14);
		break;
	case CMD17: /* READ_SINGLE_BLOCK */
		cmd |= (BlockSize<<16) | (1<<11);
		break;
	case CMD18: /* READ_MULTIPLE_BLOCK */
		cmd |= (BlockSize<<16) | (2<<11);
		break;
	case CMD24: /* WRITE_BLOCK */
		cmd |= (BlockSize<<16) | (1<<11) | (1<<13);
		break;
	case CMD25: /* WRITE_MULTIPLE_BLOCK */
		cmd |= (BlockSize<<16) | (2<<11) | (1<<13);
		break;
	}
	
	msdc->SDC_BLK_NUM = BlockCount;
	msdc->SDC_ARG = arg;
	msdc->SDC_CMD = cmd;
	
	for (uint64_t st = millis();;) {
		if (millis() - st >= 100) return 0;
		
		DWORD stat = msdc->INT & 0x00000700;
		msdc->INT = stat;
		
		if (rt == 0) {
			if (stat & 0x00000100) return 1; /* cmd done */
		} else {
			if (stat & 0x00000100) break; /* cmd done */
			if (stat & 0x00000400) { /* crc error */
				if (idx == CMD1 || idx == CMD12 || idx == ACMD41) break;
				return 0;
			}
			if (stat & 0x00000200) return 0; /* time out */
		}
	}
	
	if (rt == 2) {
		buff[0] = msdc->SDC_RESP3;
		buff[1] = msdc->SDC_RESP2;
		buff[2] = msdc->SDC_RESP1;
		buff[3] = msdc->SDC_RESP0;
	} else {
		buff[0] = msdc->SDC_RESP0;
	}

	return 1;
}

static void ready_reception(UINT cnt, WORD bsize) {
	BlockCount = cnt;
	BlockSize = bsize;
}

static int wait_ready(WORD tmr) {
	DWORD rc;
	for (uint64_t st = millis();;) {
		if (millis() - st >= tmr) return 0;
		
		if (send_cmd(CMD13, CardRCA << 16, 1, &rc) && ((rc & 0x1e00) == 0x800))
			break;
	}
	
	return 1;
}

static void bswap_cp (BYTE *dst, const DWORD *src)
{
	DWORD d;


	d = *src;
	*dst++ = (BYTE)(d >> 24);
	*dst++ = (BYTE)(d >> 16);
	*dst++ = (BYTE)(d >> 8);
	*dst++ = (BYTE)(d >> 0);
}

/*=========================================================================*/

DSTATUS MSDC_initialize(void) {
	if (Stat & STA_NODISK) return Stat;
	if (!(Stat & STA_NOINIT)) return Stat;
	
	/* Reset */
	msdc->CFG |= (1<<2);
	while (msdc->CFG & (1<<2));
	/* Clear FIFO */
	msdc->FIFOCS |= (1<<31);
	while (msdc->FIFOCS & (1<<31));
	/* Clear interrupts */
	msdc->INT = msdc->INT;
	/* Disable all interrupts */
	msdc->INTEN = 0;
	
	delay(100);
	
	/* Clock = 400khz, PIO mode, Clock power up, SD/MMC mode */
	msdc->CFG = ((26000000/400000/4-1) << 8) | (1<<3) | (1<<1) | (1<<0);
	delay(10);
	/* Clock drive enable */
	msdc->CFG |= (1<<4);
	
	/* -------------- We're ready to rock! --------------- */
	DWORD arg, resp[4];
	BYTE ty, cmd;
	UINT n;
	
	send_cmd(CMD0, 0, 0, 0); /* put card into idle state */
	
	/* --- card is in idle state --- */
	if (send_cmd(CMD8, 0x1aa, 1, resp) && (resp[0] & 0xfff) == 0x1aa) {
		ty = CT_WIDE | CT_SDC2;
		cmd = ACMD41; arg = 0x40ff8000;
	} else {
		if (send_cmd(ACMD41, 0x00ff8000, 1, resp)) {
			ty = CT_WIDE | CT_SDC1;
			cmd = ACMD41; arg = 0x00ff8000;
		} else {
			ty = CT_MMC3;
			cmd = CMD1; arg = 0x40ff8000;
		}
	}

	for (uint64_t st = millis();;) {
		if (millis() - st >= 1000) goto di_fail;
		/* wait until card goes ready state */
		if (send_cmd(cmd, arg, 1, resp) && (resp[0] & 0x80000000))
			break;
	}
	
	if (resp[0] & 0x40000000) ty |= CT_BLOCK;
	bswap_cp(&CardOCR[0], resp);
	
	/* ---- card is in ready state ---- */
	
	if (!send_cmd(CMD2, 0, 2, resp)) goto di_fail;
	for (n = 0; n < 4; n++) bswap_cp(&CardCID[n * 4], &resp[n]);
	
	/* ---- card is in ident state ---- */
	
	if (ty & CT_SDC) {
		if (!send_cmd(CMD3, 0, 1, resp)) goto di_fail;
		CardRCA = resp[0] >> 16;
	} else {
		if (!send_cmd(CMD3, 1 << 16, 1, resp)) goto di_fail;
		CardRCA = 1;
	}
	
	/* ---- card is in stby state ---- */
	
	if (!send_cmd(CMD9, CardRCA << 16, 2, resp)) goto di_fail;
	for (n = 0; n < 4; n++) bswap_cp(&CardCSD[n * 4], &resp[n]);
	
	if (ty & CT_MMC3) {
		if ((CardCSD[0] & 0xbc) == 0x90) ty = (ty & CT_BLOCK) | CT_WIDE | CT_MMC4;
	}
	
	if (!send_cmd(CMD7, CardRCA << 16, 1, resp)) goto di_fail;
	
	/* ---- card is in tran state ---- */
	
	if (!(ty & CT_BLOCK)) {
		if (!send_cmd(CMD16, 512, 1, resp) || (resp[0] & 0xfdf90000)) goto di_fail;
	}
	
	if (ty & CT_WIDE) {
		int rc = (ty & CT_SDC) ? send_cmd(ACMD6, 2, 1, resp) : send_cmd(CMD6, 0x03b70100, 1, resp);
		if (!rc || (resp[0] & 0xfdf90080)) goto di_fail;
		msdc->SDC_CFG = (msdc->SDC_CFG & ~(0x3 << 16)) | (0x1 << 16); /* 4-bit wide bus */
	}
	
	//msdc->CFG = (msdc->CFG & ~(0xff << 8)) | ((26000000/1000000/4-1)<<8);
	
	CardType = ty;
	Stat &= ~STA_NOINIT;
	return Stat;

di_fail:
	Stat |= STA_NOINIT;
	return Stat;
}

DSTATUS MSDC_status(void) {
	return Stat;
}

DSTATUS MSDC_read(BYTE *buff, LBA_t lba, UINT cnt) {
	DWORD resp, d, sect = lba;
	UINT cmd;

	if (cnt < 1 || cnt > 127) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	
	if (!(CardType & CT_BLOCK)) sect *= 512;
	if (!wait_ready(500)) return RES_ERROR;
	
	ready_reception(cnt, 512);
	cmd = (cnt > 1) ? CMD18 : CMD17;
	if (send_cmd(cmd, sect, 1, &resp) && !(resp & 0xc0580000)) {
		do {
			for (UINT nbytes = 512; nbytes;) {
				while (((msdc->FIFOCS>>0) & 0xff) > 0 && nbytes) {
					*buff++ = *(uint8_t*)&msdc->RXDATA;
					nbytes--;
				}
			}
		} while (--cnt);
	}

	if (cnt || cmd == CMD18) {
		send_cmd(CMD12, 0, 1, &resp);
	}
	
	return cnt ? RES_ERROR : RES_OK;
}

DRESULT MSDC_write(const BYTE *buff, LBA_t lba, UINT cnt) {
	DWORD resp, d, sect = lba;
	UINT cmd;

	if (cnt < 1 || cnt > 127) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;
	
	if (!(CardType & CT_BLOCK)) sect *= 512;
	if (!wait_ready(500)) return RES_ERROR;
	
	if (cnt == 1) {
		cmd = CMD24;
	} else {
		cmd = (CardType & CT_SDC) ? ACMD23 : CMD23;
		if (!send_cmd(cmd, cnt, 1, &resp) || (resp & 0xc0580000)) {
			return RES_ERROR;
		}
		cmd = CMD25;
	}
	
	if (!send_cmd(cmd, sect, 1, &resp) || (resp & 0xc0580000)) {
		return RES_ERROR;
	}
	
	do {
		for (UINT nbytes = 512; nbytes;) {
			while (((msdc->FIFOCS>>16) & 0xff) < 64 && nbytes) {
				*(uint8_t*)&msdc->TXDATA = *buff++;
				nbytes--;
			}
		}
	} while (--cnt);
	
	if (cnt || (cmd == CMD25 && (CardType & CT_SDC))) {
		send_cmd(CMD12, 0, 1, &resp);
	}
	
	return cnt ? RES_ERROR : RES_OK;
}

DRESULT MSDC_ioctl(BYTE cmd, void *buff) {
	DRESULT res = RES_ERROR;
	BYTE b, *ptr = buff, sdstat[64];
	DWORD resp, d, st, ed;
	LBA_t *dp;
	static const DWORD au_size[] = {1, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 24576, 32768, 49152, 65536, 131072};

	if (Stat & STA_NOINIT) return RES_NOTRDY;

	switch (cmd) {
	case CTRL_SYNC:
		if (wait_ready(500)) res = RES_OK;
		break;
	
	case GET_SECTOR_COUNT:
		if (CardType & CT_SDC2 && CardType & CT_BLOCK) {
			d = CardCSD[9] + (CardCSD[8] << 8) + ((CardCSD[7] & 0x3f) << 16) + 1;
			*(LBA_t*)buff = d << 10;
			res = RES_OK;
		} else {
			d = (CardCSD[8] >> 6) + (CardCSD[7] << 2) + ((CardCSD[6] & 0x3) << 10) + 1;
			if (d == 4096 && (CardType & CT_MMC4)) {
				xputs(">>> Damn it, it's the MMCv4 ! <<<\n");
			} else {
				b = (CardCSD[5] & 0xf) + ((CardCSD[10] & 0x80) >> 7) + ((CardCSD[9] & 0x3) << 1) - 7;
				*(LBA_t*)buff = d << b;
				res = RES_OK;
			}
		}
		break;
	
	case GET_BLOCK_SIZE:
		if (CardType & CT_SDC2) {
			//if (MSDC_ioctl(MMC_GET_SDSTAT, sdstat)) break;
			*(DWORD*)buff = 1; //au_size[sdstat[10] >> 4];
		} else {
			if (CardType & CT_SDC1) {
				*(DWORD*)buff = (((CardCSD[10] & 0x3f) << 1) + ((CardCSD[11] & 0x80) >> 7) + 1) << ((CardCSD[13] >> 6) - 1); 
			} else {
				*(DWORD*)buff = (((CardCSD[10] & 0x7c) >> 2) + 1) * (((CardCSD[11] & 0x3) << 3) + ((CardCSD[11] & 0xe0) >> 5) + 1);
			}
		}
		res = RES_OK;
		break;
	
	case CTRL_TRIM:
		if ((CardType & CT_MMC) && ((CardCSD[0] & 0xfc) != 0xd0)) break;
		if ((CardType & CT_SDC) && !(CardCSD[10] & 0x40)) break;
		dp = buff; st = dp[0]; ed = dp[1];
		if (!(CardType & CT_BLOCK)) {
			st *= 512; ed *= 512;
		}
		if (send_cmd(CMD32, st, 1, &resp) && send_cmd(CMD33, ed, 1, &resp) && send_cmd(CMD38, 1, 1, &resp) && wait_ready(30000)) {
			res = RES_OK;
		}
		break;
	
	case MMC_GET_TYPE:
		*ptr = CardType;
		res = RES_OK;
		break;
	
	case MMC_GET_CSD:
		memcpy(buff, CardCSD, 16);
		res = RES_OK;
		break;
	
	case MMC_GET_CID:
		memcpy(buff, CardCID, 16);
		res = RES_OK;
		break;
	
	case MMC_GET_OCR:
		memcpy(buff, CardOCR, 4);
		res = RES_OK;
		break;
	
	default:
		res = RES_PARERR;
		break;
	}
	
	return res;
}