#ifndef _MTK_PL_H
#define _MTK_PL_H

#include <stdint.h>

enum mtk_pl_rc {
	MTK_PL_OK = 0,		//ok (should be 0 for use in if statement!!)
	MTK_PL_CONN_ERR,	//connection error
	MTK_PL_IO_ERR,		//io error
	MTK_PL_NOT_CONN,	//not connected
	MTK_PL_COMM_ERR,	//communcation error
	MTK_PL_CMD_ERR,	//command error
};

int mtk_pl_connect(char *path);
void mtk_pl_disconnect(void);
void mtk_pl_flush(void);
int mtk_pl_term(void);

int mtk_pl_sendByte(uint8_t val);
int mtk_pl_sendWord(uint16_t val);
int mtk_pl_sendDWord(uint32_t val);

int mtk_pl_recvByte(uint8_t *val);
int mtk_pl_recvWord(uint16_t *val);
int mtk_pl_recvDWord(uint32_t *val);

int mtk_pl_sendByteChk(uint8_t val);
int mtk_pl_sendWordChk(uint16_t val);
int mtk_pl_sendDWordChk(uint32_t val);

int mtk_pl_sendBytes(const uint8_t *ptr, int len);
int mtk_pl_recvBytes(uint8_t *ptr, int len);

#endif
