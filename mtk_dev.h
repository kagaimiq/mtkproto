#ifndef _MTK_DEV_H
#define _MTK_DEV_H

#include <stdint.h>

/* Some error codes, maybe abandon them instead? */
enum {
	MTK_PL_OK = 0,
	MTK_PL_CONN_ERR,	/* connection error */
	MTK_PL_IO_ERR,		/* io error */
	MTK_PL_NOT_CONN,	/* not connected */
	MTK_PL_COMM_ERR,	/* communcation error */
	MTK_PL_CMD_FAILED	/* command failed */
};

int mtk_dev_connect(char *path);
void mtk_dev_disconnect(void);

void mtk_dev_flush(void);

int mtk_dev_send(void *ptr, int len);
int mtk_dev_recv(void *ptr, int len);

#endif
