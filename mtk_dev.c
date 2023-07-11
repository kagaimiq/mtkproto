#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include "mtk_dev.h"

int mtk_dev_fd = -1;

int mtk_dev_connect(char *path) {
	printf("Waiting for `%s`.", path);

	while (mtk_dev_fd < 0) {
		mtk_dev_fd = open(path, O_RDWR | O_NOCTTY);

		if (mtk_dev_fd < 0) {
			if (errno == ENOENT || errno == EACCES) {
				printf(".");
				fflush(stdout);
				usleep(250000);
			} else {
				printf("error: %d (%s)\n", errno, strerror(errno));
				return MTK_PL_CONN_ERR;
			}
		}
	}

	puts("ok");

	/*
	 * Setup for something sensible
	 */
	struct termios newtio = {
		.c_cflag = B115200 | CS8 | CLOCAL | CREAD,
		.c_iflag = IGNPAR,
		.c_oflag = 0,
		.c_lflag = 0,
		.c_cc[VTIME] = 50,	/* 5 sec timeout */
		.c_cc[VMIN] = 0
	};
	if (tcsetattr(mtk_dev_fd, TCSANOW, &newtio) < 0) {
		printf("Failed to set termios: %d (%s)\n", errno, strerror(errno));
		goto Failed;
	}

	return MTK_PL_OK;
Failed:
	close(mtk_dev_fd);
	mtk_dev_fd = -1;
	return MTK_PL_CONN_ERR;
}

void mtk_dev_disconnect(void) {
	if (mtk_dev_fd < 0) return;

	close(mtk_dev_fd);
	mtk_dev_fd = -1;
}

void mtk_dev_flush(void) {
	if (mtk_dev_fd < 0) return;

	/* Flush the input buffer */
	tcflush(mtk_dev_fd, TCIFLUSH);
}

int mtk_dev_send(void *ptr, int len) {
	if (mtk_dev_fd < 0)
		return MTK_PL_NOT_CONN;

	if (write(mtk_dev_fd, ptr, len) < len)
		return MTK_PL_IO_ERR;

	return 0;
}

int mtk_dev_recv(void *ptr, int len) {
	if (mtk_dev_fd < 0)
		return MTK_PL_NOT_CONN;

	while (len > 0) {
		int n = read(mtk_dev_fd, ptr, len);
		if (n <= 0)
			return MTK_PL_IO_ERR;

		/*printf("%p:%d /", ptr, n);
		for (int i = 0; i < n; i++) printf(" %02x", *(uint8_t *)(ptr + i));
		puts(" \\");*/

		ptr += n; len -= n;
	}

	return 0;
}
