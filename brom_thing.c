#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "mtk_dev.h"
#include "mtk_pl.h"



void hexdump(uint8_t *ptr, int len) {
	for (int i = 0; i < len; i += 16) {
		printf("%p: ", ptr + i);

		for (int j = 0; j < 16; j++) {
			if (i+j >= len) {
				printf("-- ");
			} else {
				printf("%02X ", ptr[i+j]);
			}
		}

		printf(" |");

		for (int j = 0; j < 16; j++) {
			if (i+j >= len) {
				putchar(' ');
			} else {
				uint8_t b = ptr[i+j];
				putchar(b<0x20||b>=0x7f?'.':b);
			}
		}

		printf("|\n");
	}
}



void do_the_fun(void) {
	uint32_t tmp[32] = {};
	printf("res: %04x\n", mtk_pl_read32(0x00200000, sizeof tmp / 4, tmp));

	for (int i = 0; i < sizeof tmp / 4; i += 4) {
		printf("[+%04x] ", i * 4);

		for (int j = 0; j < 4; j++) {
			printf(" <%08x>", tmp[i+j]);
		}

		putchar('\n');
	}
}



int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Usage: %s <mtk tty>\n", argv[0]);
		return 1;
	}

	int rc;
	if ((rc = mtk_dev_connect(argv[1]))) {
		printf("Failed to connect. (%d)\n", rc);
		return 2;
	}

	if ((rc = mtk_pl_handshake())) {
		printf("Failed to send handshake. (%d)\n", rc);
		rc = 2;
		goto Exit;
	}

	do_the_fun();

Exit:
	mtk_dev_disconnect();
	return rc;
}
