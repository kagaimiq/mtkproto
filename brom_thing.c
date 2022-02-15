#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "mtk_pl.h"

int Upload1st(char *path, uint32_t addr) {
	FILE *fp;
	uint16_t tmp16;
	int rc = -1;
	
	if (!(fp = fopen(path, "rb"))) {
		printf("couldn't open file [%s]!\n", path);
		goto Exit;
	}
	
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	printf("Address: 0x%x, Size: %d\n", addr, size);
	
	/* ------------CMD_SEND_DA------------ */
	if (mtk_pl_sendByteChk(0xd7))
		goto ExitCloseFile;
	
	/* address */
	if (mtk_pl_sendDWordChk(addr))
		goto ExitCloseFile;
	
	/* size */
	if (mtk_pl_sendDWordChk(size))
		goto ExitCloseFile;
	
	/* hash size (no hash) */
	if (mtk_pl_sendDWordChk(0))
		goto ExitCloseFile;
	
	/* RC */
	if (mtk_pl_recvWord(&tmp16))
		goto ExitCloseFile;
	
	if (tmp16 != 0x0000) {
		printf("CMD_SEND_DA initial checks failed : %04x\n", tmp16);
		goto ExitCloseFile;
	}
	
	/* Data */
	puts("");

	for (int n = 0;;) {
		uint8_t tmp[4096];
		
		int rdn = fread(tmp, 1, sizeof(tmp), fp);
		if (rdn <= 0) break;
		
		struct timespec tss, tse;
		clock_gettime(CLOCK_MONOTONIC, &tss);
		
		if (mtk_pl_sendBytes(tmp, rdn)) {
			printf("failed");
			break;
		}
		
		clock_gettime(CLOCK_MONOTONIC, &tse);
		
		double rtime = (1. * tse.tv_sec + 1e-9 * tse.tv_nsec) - (1. * tss.tv_sec + 1e-9 * tss.tv_nsec);
		
		printf("\e[1A\e[2K[0x%x] %d (%.3f kB/s)\n",
			addr+n, rdn, rdn / rtime / 1000.);
		
		n += rdn;
	}
	
	/* Checksum */
	if (mtk_pl_recvWord(&tmp16))
		goto ExitCloseFile;
	
	printf("Data checksum : %04x\n", tmp16);
	
	/* RC */
	if (mtk_pl_recvWord(&tmp16))
		goto ExitCloseFile;
	
	if (tmp16 != 0x0000) {
		printf("CMD_SEND_DA data checks failed : %04x\n", tmp16);
		goto ExitCloseFile;
	}
	
	/* ---------------CMD_JUMP_DA--------------- */
	if (mtk_pl_sendByteChk(0xd5))
		goto ExitCloseFile;
	
	/* address */
	if (mtk_pl_sendDWordChk(addr))
		goto ExitCloseFile;
	
	/* RC */
	if (mtk_pl_recvWord(&tmp16))
		goto ExitCloseFile;
	
	if (tmp16 != 0x0000) {
		printf("CMD_JUMP_DA failed : %04x\n", tmp16);
		goto ExitCloseFile;
	}
	
	rc = 0;

ExitCloseFile:
	fclose(fp);
Exit:
	return rc;
}

int getSomeInfos(void) {
	uint8_t tmp8;
	uint16_t tmp16;
	uint16_t aphw_code, aphw_subcode, aphw_ver, apsw_ver;
	uint32_t tgt_cfg;
	int rc = -1;
	
	/* ---------- Get BL Version ---------- */
	if (mtk_pl_sendByte(0xfe))
		goto Exit;
	
	if (mtk_pl_recvByte(&tmp8))
		goto Exit;
	
	printf("BLVersion=%d\n", tmp8);
	
	/* ---------- Get HW code ---------- */
	if (mtk_pl_sendByteChk(0xfd))
		goto Exit;
	
	/* APHW_CODE */
	if (mtk_pl_recvWord(&aphw_code))
		goto Exit;
	
	/* RC */
	if (mtk_pl_recvWord(&tmp16))
		goto Exit;
	
	if (tmp16 != 0) { /* Unlikely to happen but why not */
		printf("GetHwCode Fail=%x\n", tmp16);
		goto Exit;
	}
	
	/* ---------- Get HW/SW version ---------- */
	if (mtk_pl_sendByteChk(0xfc))
		goto Exit;
	
	/* APHW_SUBCODE */
	if (mtk_pl_recvWord(&aphw_subcode))
		goto Exit;
	
	/* APHW_VER */
	if (mtk_pl_recvWord(&aphw_ver))
		goto Exit;

	/* APSW_VER */
	if (mtk_pl_recvWord(&apsw_ver))
		goto Exit;
	
	/* RC */
	if (mtk_pl_recvWord(&tmp16))
		goto Exit;
	
	if (tmp16 != 0) { /* Unlikely to happen but why not */
		printf("GetHwSwVer Fail=%x\n", tmp16);
		goto Exit;
	}
	
	printf("APHW_CODE == %04x\n", aphw_code);
	printf("APHW_SUBCODE == %04x\n", aphw_subcode);
	printf("APHW_VER == %04x\n", aphw_ver);
	printf("APSW_VER == %04x\n", apsw_ver);
	
	/* ---------- Get Target Config ---------- */
	if (mtk_pl_sendByteChk(0xd8))
		goto Exit;
	
	/* target config */
	if (mtk_pl_recvDWord(&tgt_cfg))
		goto Exit;
	
	/* RC */
	if (mtk_pl_recvWord(&tmp16))
		goto Exit;
	
	if (tmp16 != 0) { /* Unlikely to happen but why not */
		printf("GetTgtCfg Fail=%x\n", tmp16);
		goto Exit;
	}
	
	printf("Target Config --> %x\n", tgt_cfg);
	if (tgt_cfg & 0x00000001) puts(".... Secure Boot Present!!!");
	if (tgt_cfg & 0x00000002) puts(".... Serial Link Auth!!!!!");
	if (tgt_cfg & 0x00000004) puts(".... Download Agent Auth!!!!!");
	
	if (tgt_cfg & 0x00000006) {
		printf("Oh No! we have the Authes! We need to do the Exploit! for %04x-%04x-%04x-%04x\n",
			aphw_code, aphw_subcode, aphw_ver, apsw_ver);
	}
	
	rc = 0;
Exit:
	return rc;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Usage: %s <mtk tty> [<payload addr> <payload file>]\n", argv[0]);
		return 1;
	}
	
	int rc;
	if ((rc = mtk_pl_connect(argv[1]))) {
		printf("failed to connect. (%d)\n", rc);
		return 2;
	}
	
	for (int try = 10; try >= 0; try--) {
		if (try == 0) {
			puts("send token fail...");
			rc = 2;
			goto Exit;
		}
		
		uint8_t tmp;
		
		mtk_pl_flush();
		
		int fail = 0;
		if (mtk_pl_sendByte(0xa0) || mtk_pl_recvByte(&tmp) || (tmp != 0x5f)) fail++;
		if (mtk_pl_sendByte(0x0a) || mtk_pl_recvByte(&tmp) || (tmp != 0xf5)) fail++;
		if (mtk_pl_sendByte(0x50) || mtk_pl_recvByte(&tmp) || (tmp != 0xaf)) fail++;
		if (mtk_pl_sendByte(0x05) || mtk_pl_recvByte(&tmp) || (tmp != 0xfa)) fail++;
		
		if (fail) {
			mtk_pl_flush();
			
			/* check for open connection of bootrom */
			if (!mtk_pl_sendByte(0xff) && !mtk_pl_recvByte(&tmp)) {
				if (tmp == 0x05) break;
			}

			/* check for open connection of preloader/bootrom */
			if (!mtk_pl_sendByte(0xfe) && !mtk_pl_recvByte(&tmp)) {
				if (tmp == 0x01 || tmp == 0xfe) break;
			}
		} else {
			break;
		}
	}

	puts("---------- Get some infos! ------------");
	if (getSomeInfos()) {
		printf("**********couldn't get some infos!***********");
		rc = 2;
		goto Exit;
	}
	
	if (argc >= 4) {
		puts("---------- Payload upload! ------------");
		if (Upload1st(argv[3], strtoul(argv[2], NULL, 0))) {
			puts("**********couldn't upload payload!***********");
			rc = 2;
			goto Exit;
		}
	}
	
	puts("---------- Term! ---------");
	//mtk_pl_term();
	
Exit:
	mtk_pl_disconnect();
	return rc;
}
