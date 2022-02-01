#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>

#include "mtk_pl.h"

//-------------------- platform dependent --------------------

int mtk_pl_fd = -1;

int mtk_pl_connect(char *path) {
	printf("waiting for connection to `%s`", path);
	while (1) {
		mtk_pl_fd = open(path, O_RDWR | O_NOCTTY);
		if (mtk_pl_fd < 0) {
			if (errno == ENOENT || errno == EACCES) {
				printf(".");
				fflush(stdout);
				usleep(250000);
			} else {
				puts("failed");
				printf("Error: %d (%s)\n", errno, strerror(errno));
				return MTK_PL_CONN_ERR;
			}
		} else {
			puts("done");
			break;
		}
	}
	
	struct termios newtio = {
		.c_cflag = B19200 | CS8 | CLOCAL | CREAD,
		.c_iflag = IGNPAR,
		.c_oflag = 0,
		.c_lflag = 0,
		.c_cc[VTIME] = 50,
		.c_cc[VMIN] = 0
	};
	if (tcsetattr(mtk_pl_fd, TCSANOW, &newtio) < 0) {
		printf("Failed to set termios: %d (%s)\n", errno, strerror(errno));
		close(mtk_pl_fd);
		return MTK_PL_IO_ERR;
	}
	
	return MTK_PL_OK;
}

void mtk_pl_disconnect(void) {
	if (mtk_pl_fd > 0) {
		close(mtk_pl_fd);
		mtk_pl_fd = -1;
	}
}

int mtk_pl_term(void) {
	if (mtk_pl_fd > 0) {
	/* code borrowed from BusyBox microcom */
		int rc = MTK_PL_OK;
		struct termios tio0, tio1;
		
		if (isatty(STDIN_FILENO)) {
			tcgetattr(STDIN_FILENO, &tio0);
			tio1 = tio0;
			
			tio1.c_lflag &= ~(ICANON | ECHO | ECHONL);
			tio1.c_lflag &= ~(ISIG);
			tio1.c_cc[VMIN] = 5;
			tio1.c_cc[VTIME] = 0;
			tio1.c_iflag &= ~(IXON | ICRNL);
			tio1.c_oflag &= ~(ONLCR);
			tio1.c_iflag &= ~(IXOFF|IXON|IXANY|BRKINT|INLCR|IUCLC|IMAXBEL);
			
			tcsetattr(STDIN_FILENO, TCSAFLUSH, &tio1);
		}
		
		
		struct pollfd pfd[2] = {
			{
				.fd = mtk_pl_fd,
				.events = POLLIN
			},
			{
				.fd = STDIN_FILENO,
				.events = POLLIN
			}
		};
		
		while (rc == MTK_PL_OK && poll(pfd, 2, -1) > 0) {
			if (pfd[1].revents) {
				char c;
				if (read(STDIN_FILENO, &c, 1) < 1) {
				//maybe do something smart next time ...
					rc = MTK_PL_IO_ERR;
					break;
				} else {
					//Ctrl+X
					if (c == 24)
						break;
					
					int x = write(mtk_pl_fd, &c, 1);
				}
			}
			
			if (pfd[0].revents) {
				char tmpbuf[1024];
				int rlen = read(mtk_pl_fd, tmpbuf, sizeof tmpbuf);
				if (rlen > 0) {
					int x = write(STDOUT_FILENO, tmpbuf, rlen);
				} else {
					rc = MTK_PL_IO_ERR;
					break;
				}
			}
		}

		if (isatty(STDIN_FILENO))
			tcsetattr(STDIN_FILENO, TCSAFLUSH, &tio0);
		
mtkExit:
		return rc;
	}
	return MTK_PL_NOT_CONN;
}

void mtk_pl_flush(void) {
	if (mtk_pl_fd > 0) {
		tcflush(mtk_pl_fd, TCIFLUSH);
	}
}

int mtk_pl_sendByte(uint8_t val) {
	if (mtk_pl_fd > 0) {
		if (write(mtk_pl_fd, &val, 1) < 1)
			return MTK_PL_IO_ERR;
		
		return MTK_PL_OK;
	}
	return MTK_PL_NOT_CONN;
}

int mtk_pl_sendWord(uint16_t val) {
	if (mtk_pl_fd > 0) {
		uint8_t tmp[2] = {val>>8, val};
		
		if (write(mtk_pl_fd, tmp, 2) < 2)
			return MTK_PL_IO_ERR;
		
		return MTK_PL_OK;
	}
	return MTK_PL_NOT_CONN;
}

int mtk_pl_sendDWord(uint32_t val) {
	if (mtk_pl_fd > 0) {
		uint8_t tmp[4] = {val>>24, val>>16, val>>8, val};
		
		if (write(mtk_pl_fd, tmp, 4) < 4)
			return MTK_PL_IO_ERR;
		
		return MTK_PL_OK;
	}
	return MTK_PL_NOT_CONN;
}

int mtk_pl_recvByte(uint8_t *val) {
	if (mtk_pl_fd > 0) {
		if (read(mtk_pl_fd, val, 1) < 1)
			return MTK_PL_IO_ERR;
		
		return MTK_PL_OK;
	}
	return MTK_PL_NOT_CONN;
}

int mtk_pl_recvWord(uint16_t *val) {
	if (mtk_pl_fd > 0) {
		uint8_t tmp[2];
		if (read(mtk_pl_fd, tmp, 2) < 2)
			return MTK_PL_IO_ERR;
		
		*val = tmp[0] << 8 | tmp[1];
		
		return MTK_PL_OK;
	}
	return MTK_PL_NOT_CONN;
}

int mtk_pl_recvDWord(uint32_t *val) {
	if (mtk_pl_fd > 0) {
		uint8_t tmp[4];
		if (read(mtk_pl_fd, tmp, 4) < 4)
			return MTK_PL_IO_ERR;
		
		*val = tmp[0] << 24 | tmp[1] << 16 | tmp[2] << 8 | tmp[3];
		
		return MTK_PL_OK;
	}
	return MTK_PL_NOT_CONN;
}

int mtk_pl_sendBytes(const uint8_t *ptr, int len) {
	if (mtk_pl_fd > 0) {
		if (write(mtk_pl_fd, ptr, len) < len)
			return MTK_PL_IO_ERR;
		
		return MTK_PL_OK;
	}
	return MTK_PL_NOT_CONN;
}

int mtk_pl_recvBytes(uint8_t *ptr, int len) {
	if (mtk_pl_fd > 0) {
		//if (read(mtk_pl_fd, ptr, len) < len)
		//	return MTK_PL_IO_ERR;
		
		/* the data may be chunked so read it in a loop */
		int cnt = 0;
		while (cnt < len) {
			int n = read(mtk_pl_fd, ptr + cnt, len - cnt);
			if (n <= 0) return MTK_PL_IO_ERR;
			cnt += n;
		}
		
		return MTK_PL_OK;
	}
	return MTK_PL_NOT_CONN;
}

//------------------- platform independent -------------------

int mtk_pl_sendByteChk(uint8_t val) {
	uint8_t tmp;
	int rc;
	
	if ((rc = mtk_pl_sendByte(val))) return rc;
	if ((rc = mtk_pl_recvByte(&tmp))) return rc;
	if (tmp != val) return MTK_PL_COMM_ERR;
	
	return MTK_PL_OK;
}

int mtk_pl_sendWordChk(uint16_t val) {
	uint16_t tmp;
	int rc;
	
	if ((rc = mtk_pl_sendWord(val))) return rc;
	if ((rc = mtk_pl_recvWord(&tmp))) return rc;
	if (tmp != val) return MTK_PL_COMM_ERR;
	
	return MTK_PL_OK;
}

int mtk_pl_sendDWordChk(uint32_t val) {
	uint32_t tmp;
	int rc;
	
	if ((rc = mtk_pl_sendDWord(val))) return rc;
	if ((rc = mtk_pl_recvDWord(&tmp))) return rc;
	if (tmp != val) return MTK_PL_COMM_ERR;
	
	return MTK_PL_OK;
}
