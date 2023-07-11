#ifndef _MTK_PL_H
#define _MTK_PL_H

#include <stdint.h>

/* Mainly for the error codes, for now. */
#include "mtk_dev.h"

enum {
	MTKPL_CMD_LEGACY_WRITE		= 0xA1,
	MTKPL_CMD_LEGACY_READ,

	MTKPL_CMD_I2C_INIT		= 0xB0,
	MTKPL_CMD_I2C_DEINIT,
	MTKPL_CMD_I2C_WRITE8,
	MTKPL_CMD_I2C_READ8,
	MTKPL_CMD_I2C_SET_SPEED,

	MTKPL_CMD_PWR_INIT		= 0xC4,
	MTKPL_CMD_PWR_DEINIT,
	MTKPL_CMD_PWR_READ16,
	MTKPL_CMD_PWR_WRITE16,

	MTKPL_CMD_READ16		= 0xD0,
	MTKPL_CMD_READ32,
	MTKPL_CMD_WRITE16,
	MTKPL_CMD_WRITE16_NO_ECHO,
	MTKPL_CMD_WRITE32,
	MTKPL_CMD_JUMP_DA,
	MTKPL_CMD_JUMP_BL,
	MTKPL_CMD_SEND_DA,
	MTKPL_CMD_GET_TARGET_CONFIG,

	MTKPL_CMD_UART1_LOG_EN		= 0xDB,

	MTKPL_CMD_GET_HW_SW_VER		= 0xFC,
	MTKPL_CMD_HW_CODE,
	MTKPL_CMD_BL_VER,
	MTKPL_CMD_VERSION
};

int mtk_pl_handshake(void);

int mtk_pl_read16(uint32_t addr, int cnt, uint16_t *ptr);
int mtk_pl_read32(uint32_t addr, int cnt, uint32_t *ptr);
int mtk_pl_write16(uint32_t addr, int cnt, uint16_t *ptr);
int mtk_pl_write32(uint32_t addr, int cnt, uint32_t *ptr);

int mtk_pl_jump_da(uint32_t addr);
int mtk_pl_send_da(uint32_t addr, uint32_t size, uint32_t siglen);

/*-----------------------------------*/

int mtk_pl_send8(uint8_t val);
int mtk_pl_send16(uint16_t val);
int mtk_pl_send32(uint32_t val);

int mtk_pl_recv8(uint8_t *val);
int mtk_pl_recv16(uint16_t *val);
int mtk_pl_recv32(uint32_t *val);

int mtk_pl_send8_echo(uint8_t val);
int mtk_pl_send16_echo(uint16_t val);
int mtk_pl_send32_echo(uint32_t val);

#endif
