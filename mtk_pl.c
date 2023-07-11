#include "mtk_pl.h"

int mtk_pl_handshake(void) {
	uint8_t tmp;

	/*
	 * Try to send some commands to determine whether the connection
	 * is still open
	 */

	/* CMD_BL_VER usually returns 0x01 on preloader or 0xFE on BROM */
	if (!mtk_pl_send8(MTKPL_CMD_BL_VER) && !mtk_pl_recv8(&tmp)) {
		if (tmp == 0x01 || tmp == 0xfe)
			return MTK_PL_OK;
	}

	/*
	 * Try to send a sync string, after all
	 */
	for (int try = 0; try < 10; try++) {
		mtk_dev_flush();

		uint8_t sync[4] = {0xa0,0x0a,0x50,0x05};

		for (int i = 0; i <= 4; i++) {
			if (i == 4)
				return MTK_PL_OK;

			/* Send a sync byte and receive it */
			if (mtk_pl_send8(sync[i]) || mtk_pl_recv8(&tmp))
				break;

			/* The response should be its inverse */
			if ((sync[i] ^ tmp) != 0xff)
				break;
		}
	}

	return MTK_PL_CONN_ERR;
}

/*-------------------------------------------------------------------*/

int mtk_pl_read16(uint32_t addr, int cnt, uint16_t *ptr) {
	uint16_t ret;
	int rc;

	if ((rc = mtk_pl_send8_echo(MTKPL_CMD_READ16)))
		return rc;

	if ((rc = mtk_pl_send32_echo(addr)))
		return rc;
	if ((rc = mtk_pl_send32_echo(cnt)))
		return rc;

	if ((rc = mtk_pl_recv16(&ret)))
		return rc;
	if (ret >= 0x100)
		return ret;

	while (cnt-- > 0) {
		if ((rc = mtk_pl_recv16(ptr++)))
			return rc;
	}

	if ((rc = mtk_pl_recv16(&ret)))
		return rc;
	if (ret >= 0x100)
		return ret;

	return MTK_PL_OK;
}

int mtk_pl_read32(uint32_t addr, int cnt, uint32_t *ptr) {
	uint16_t ret;
	int rc;

	if ((rc = mtk_pl_send8_echo(MTKPL_CMD_READ32)))
		return rc;

	if ((rc = mtk_pl_send32_echo(addr)))
		return rc;
	if ((rc = mtk_pl_send32_echo(cnt)))
		return rc;

	if ((rc = mtk_pl_recv16(&ret)))
		return rc;
	if (ret >= 0x100)
		return ret;

	while (cnt-- > 0) {
		if ((rc = mtk_pl_recv32(ptr++)))
			return rc;
	}

	if ((rc = mtk_pl_recv16(&ret)))
		return rc;
	if (ret >= 0x100)
		return ret;

	return MTK_PL_OK;
}

int mtk_pl_write16(uint32_t addr, int cnt, uint16_t *ptr) {
	uint16_t ret;
	int rc;

	if ((rc = mtk_pl_send8_echo(MTKPL_CMD_WRITE16)))
		return rc;

	if ((rc = mtk_pl_send32_echo(addr)))
		return rc;
	if ((rc = mtk_pl_send32_echo(cnt)))
		return rc;

	if ((rc = mtk_pl_recv16(&ret)))
		return rc;
	if (ret >= 0x100)
		return ret;

	while (cnt-- > 0) {
		if ((rc = mtk_pl_send16_echo(*ptr++)))
			return rc;
	}

	if ((rc = mtk_pl_recv16(&ret)))
		return rc;
	if (ret >= 0x100)
		return ret;

	return MTK_PL_OK;
}

int mtk_pl_write32(uint32_t addr, int cnt, uint32_t *ptr) {
	uint16_t ret;
	int rc;

	if ((rc = mtk_pl_send8_echo(MTKPL_CMD_WRITE32)))
		return rc;

	if ((rc = mtk_pl_send32_echo(addr)))
		return rc;
	if ((rc = mtk_pl_send32_echo(cnt)))
		return rc;

	if ((rc = mtk_pl_recv16(&ret)))
		return rc;
	if (ret >= 0x100)
		return ret;

	while (cnt-- > 0) {
		if ((rc = mtk_pl_send32_echo(*ptr++)))
			return rc;
	}

	if ((rc = mtk_pl_recv16(&ret)))
		return rc;
	if (ret >= 0x100)
		return ret;

	return MTK_PL_OK;
}

/*-------------------------------------------------------------------*/

int mtk_pl_jump_da(uint32_t addr) {
	uint16_t ret;
	int rc;

	if ((rc = mtk_pl_send8_echo(MTKPL_CMD_JUMP_DA)))
		return rc;

	if ((rc = mtk_pl_send32_echo(addr)))
		return rc;

	if ((rc = mtk_pl_recv16(&ret)))
		return rc;
	if (ret >= 0x100)
		return ret;

	return MTK_PL_OK;
}

int mtk_pl_send_da(uint32_t addr, uint32_t size, uint32_t siglen) {
	uint16_t ret;
	int rc;

	if ((rc = mtk_pl_send8_echo(MTKPL_CMD_SEND_DA)))
		return rc;

	if ((rc = mtk_pl_send32_echo(addr)))
		return rc;
	if ((rc = mtk_pl_send32_echo(size)))
		return rc;
	if ((rc = mtk_pl_send32_echo(siglen)))
		return rc;

	if ((rc = mtk_pl_recv16(&ret)))
		return rc;
	if (ret >= 0x100)
		return ret;

	return MTK_PL_OK;
}

/*-------------------------------------------------------------------*/

/*
 * Send a 8/16/32-bit value
 */

int mtk_pl_send8(uint8_t val) {
	return mtk_dev_send(&val, 1);
}

int mtk_pl_send16(uint16_t val) {
	uint8_t tmp[2] = {val >> 8, val};
	return mtk_dev_send(tmp, sizeof tmp);
}

int mtk_pl_send32(uint32_t val) {
	uint8_t tmp[4] = {val>>24, val>>16, val>>8, val};
	return mtk_dev_send(tmp, sizeof tmp);
}

/*
 * Receive a 8/16/32-bit value
 */

int mtk_pl_recv8(uint8_t *val) {
	return mtk_dev_recv(val, 1);
}

int mtk_pl_recv16(uint16_t *val) {
	uint8_t tmp[2];
	int rc = mtk_dev_recv(tmp, sizeof tmp);
	if (rc) return rc;

	*val = tmp[0] << 8 | tmp[1];
	return MTK_PL_OK;
}

int mtk_pl_recv32(uint32_t *val) {
	uint8_t tmp[4];
	int rc = mtk_dev_recv(tmp, sizeof tmp);
	if (rc) return rc;

	*val = tmp[0] << 24 | tmp[1] << 16 | tmp[2] << 8 | tmp[3];
	return MTK_PL_OK;
}

/*
 * Send a 8/16/32-bit value and check the echoed back value
 */

int mtk_pl_send8_echo(uint8_t val) {
	uint8_t tmp;
	int rc;

	if ((rc = mtk_pl_send8(val))) return rc;
	if ((rc = mtk_pl_recv8(&tmp))) return rc;
	if (tmp != val) return MTK_PL_COMM_ERR;

	return MTK_PL_OK;
}

int mtk_pl_send16_echo(uint16_t val) {
	uint16_t tmp;
	int rc;

	if ((rc = mtk_pl_send16(val))) return rc;
	if ((rc = mtk_pl_recv16(&tmp))) return rc;
	if (tmp != val) return MTK_PL_COMM_ERR;

	return MTK_PL_OK;
}

int mtk_pl_send32_echo(uint32_t val) {
	uint32_t tmp;
	int rc;

	if ((rc = mtk_pl_send32(val))) return rc;
	if ((rc = mtk_pl_recv32(&tmp))) return rc;
	if (tmp != val) return MTK_PL_COMM_ERR;

	return MTK_PL_OK;
}
