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

int Upload2nd(char *path, uint32_t addr) {
	FILE *fp;
	uint8_t tmp8;
	uint16_t tmp16;
	uint32_t tmp32;
	int rc = -1;

	if (mtk_pl_recvDWord(&tmp32)) {
		puts("didn't receive the status of the DRAM init!");
		goto Exit;
	}
	
	if (tmp32 != 0) {
		printf("Looks like we DO need to init DRAM... (it sent %x)\n", tmp32);
		
		uint8_t xxx[16];
		
		/* EMMC somthing (presence?) */
		if (mtk_pl_recvDWord(&tmp32))
			goto Exit;
		
		printf("EMMC whatever-->%08x\n", tmp32);
		
		/* EMMC CID */
		for (int i = 0; i < 4; i++) {
			if (mtk_pl_recvDWord(&tmp32))
				goto Exit;
			
			printf("     CID [%d]-->%08x\n", i, tmp32);
		}
		
		/* NAND something (presence?) */
		if (mtk_pl_recvDWord(&tmp32))
			goto Exit;
		
		printf("NAND whatever-->%08x\n", tmp32);

		/* Count of NAND ID? */
		if (mtk_pl_recvWord(&tmp16))
			goto Exit;

		/* NAND ID */
		for (int i = 0; i < tmp16; i++) {
			uint16_t tmp16x;
			
			if (mtk_pl_recvWord(&tmp16x))
				goto Exit;
			
			printf("     ID [%d]-->%04x\n", i, tmp16x);
		}
		
		/* OK, I got you, let's proceed to the DRAM init !!! */
		if (mtk_pl_sendByte(0x5a))
			goto Exit;
		
		puts("======= Init DRAM! =======");
		mtk_pl_sendDWord(21); //EMI Version
		if (mtk_pl_recvByte(&tmp8) && tmp8 == 0x5a) {
			puts("didn't receive (correct) dram init begin ack!!");
			goto Exit;
		}
		
		/* Size of the single EMI setting entry */
		if (mtk_pl_recvDWord(&tmp32))
			goto Exit;
		
		printf("Sizeof entry-->%d\n", tmp32);
		
		/* OK */
		if (mtk_pl_sendByte(0x5a))
			goto Exit;
		
		uint8_t emisetting[] = {
			/* No.1 (512 MiB x 2) */
			0x00,0x00,0x00,0x00,
			0x03,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x52,0x50,0x02,0x00,
			0x00,0xAA,0x00,0xAA,
			0x00,0xAA,0x00,0xAA,
			0x93,0x44,0x58,0x44,
			0x00,0x00,0x00,0x01,
			0x83,0x86,0x04,0xF0,
			0xD1,0x31,0x06,0xA0,
			0x01,0x04,0x08,0xBF,
			0x3F,0x6C,0x80,0x01,
			0x42,0x23,0x64,0xD1,
			0x88,0x88,0x00,0x00,
			0x88,0x88,0x88,0x88,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x10,0x05,0x00,0x11,
			0x00,0x00,0x80,0x07,
			0x00,0x26,0x00,0x04,
			0x00,0x00,0x00,0x20,
			0x00,0x00,0x00,0x20,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x01,0x00,0xC3,0x00,
			0x02,0x00,0x06,0x00,
			0x03,0x00,0x02,0x00,
			0x03,0x00,0x00,0x00,
			0x0A,0x00,0xFF,0x00,
			0x3F,0x00,0x00,0x00,
			
			/* No.2 (256 MiB x 1) */
			0x00,0x00,0x00,0x00,
			0x03,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x2E,0x21,0x00,0x00,
			0x00,0xAA,0x00,0xAA,
			0x00,0xAA,0x00,0xAA,
			0x93,0x44,0x58,0x44,
			0x00,0x00,0x00,0x01,
			0x83,0x86,0x04,0xF0,
			0xF1,0x32,0x06,0xA0,
			0x01,0x04,0x08,0xBF,
			0x3F,0x63,0x40,0x03,
			0x42,0x23,0x64,0x51,
			0x88,0x88,0x00,0x00,
			0xEE,0xEE,0xEE,0xEE,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x10,0x05,0x00,0x11,
			0x00,0x00,0x80,0x07,
			0x00,0x26,0x00,0x04,
			0x00,0x00,0x00,0x10,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x01,0x00,0xC3,0x00,
			0x02,0x00,0x06,0x00,
			0x03,0x00,0x02,0x00,
			0x06,0x00,0x00,0x00,
			0x0A,0x00,0xFF,0x00,
			0x3F,0x00,0x00,0x00,
		};
		
		/* EMI setting size */
		if (mtk_pl_sendDWord(sizeof(emisetting)))
			goto Exit;
		
		/* EMI setting data */
		if (mtk_pl_sendBytes(emisetting, sizeof(emisetting)))
			goto Exit;
		
		/* Checksum */
		if (mtk_pl_recvWord(&tmp16))
			goto Exit;
		
		printf("Checksum-->%04x\n", tmp16);
		
		/* OK, everyting is correct, let's do it! */
		if (mtk_pl_sendByte(0x5a))
			goto Exit;
		
		/* Whatever */
		if (mtk_pl_sendDWord(0))
			goto Exit;
		
		/* Init status */
		if (mtk_pl_recvDWord(&tmp32))
			goto Exit;
		
		if (tmp32 != 0) {
			printf("Hi Boy, init dram failed!! (it returned %x)\n", tmp32);
			goto Exit;
		}
		
		/* Something [2125f8] */
		if (mtk_pl_recvByte(&tmp8))
			goto Exit;
		
		printf("Y1-->%02x\n", tmp8);
		
		/* Something [2125f9] */
		if (mtk_pl_recvByte(&tmp8))
			goto Exit;
		
		printf("Y2-->%02x\n", tmp8);
		
		/* Size */
		uint64_t tmp64;
		
		/* --- high */
		if (mtk_pl_recvDWord(&tmp32))
			goto Exit;
		tmp64 = (uint64_t)tmp32 << 32;
		
		/* --- low */
		if (mtk_pl_recvDWord(&tmp32))
			goto Exit;
		tmp64 |= tmp32;
		
		printf("Size-->%lx (%.2f GiB)\n",
			tmp64, tmp64 / 1048576. / 1024.);
	} else {
		puts("Looks like we don't need to init DRAM!");
	}

	puts("======= Upload! =======");
	if (!(fp = fopen(path, "rb"))) {
		printf("couldn't open file [%s]!\n", path);
		goto Exit;
	}

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	uint8_t tmp[4096];
	
	printf("Address: 0x%x, Size: %d bytes, Block: %ld bytes\n",
		addr, size, sizeof(tmp));
	
	mtk_pl_sendDWord(addr);
	mtk_pl_sendDWord(size);
	mtk_pl_sendDWord(sizeof(tmp));
	
	if (mtk_pl_recvByte(&tmp8) || tmp8 != 0x5a) {
		puts("failed to receive the start ack!");
		goto ExitCloseFile;
	}
	
	puts("");

	for (int n = 0;;) {
		int rdn = fread(tmp, 1, sizeof(tmp), fp);
		if (rdn <= 0) break;
		
		struct timespec tss, tse;
		clock_gettime(CLOCK_MONOTONIC, &tss);
		
		if (mtk_pl_sendBytes(tmp, rdn) || mtk_pl_recvByte(&tmp8) || tmp8 != 0x5a) {
			printf("failed");
			break;
		}
		
		clock_gettime(CLOCK_MONOTONIC, &tse);
		
		double rtime = (1. * tse.tv_sec + 1e-9 * tse.tv_nsec) - (1. * tss.tv_sec + 1e-9 * tss.tv_nsec);
		
		printf("\e[1A\e[2K[0x%x] %d (%.3f kB/s)\n",
			addr+n, rdn, rdn / rtime / 1000.);
		
		n += rdn;
	}
	
	puts("");
	
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
		printf("Usage: %s <mtk tty> [<payload addr> <payload file> [<payload 2 addr> <payload 2 file>]]\n", argv[0]);
		puts(
			"\n"
			" The first payload is loaded with the BootROM/Preloader protocol,\n"
			" If the second payload is specified, then it is loaded using the\n"
			" stripped down Download Agent protocol (Only the DRAM init step is present)\n"
			"\n"
		);
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
		if (mtk_pl_sendByte(0xa0) || mtk_pl_recvByte(&tmp) || (tmp != 0x5f)) continue;
		if (mtk_pl_sendByte(0x0a) || mtk_pl_recvByte(&tmp) || (tmp != 0xf5)) continue;
		if (mtk_pl_sendByte(0x50) || mtk_pl_recvByte(&tmp) || (tmp != 0xaf)) continue;
		if (mtk_pl_sendByte(0x05) || mtk_pl_recvByte(&tmp) || (tmp != 0xfa)) continue;
		
		break;
	}
	
	puts("---------- Get some infos! ------------");
	if (getSomeInfos()) {
		printf("**********couldn't get some infos!***********");
		rc = 2;
		goto Exit;
	}
	
	if (argc >= 4) {
		puts("---------- First payload upload! ------------");
		if (Upload1st(argv[3], strtoul(argv[2], NULL, 0))) {
			puts("**********couldn't upload first payload!***********");
			rc = 2;
			goto Exit;
		}
	}
	
	if (argc >= 6) {
		puts("---------- Second payload upload! ------------");
		if ((rc = Upload2nd(argv[5], strtoul(argv[4], NULL, 0)))) {
			printf("**********couldn't upload second payload!***********");
			rc = 2;
			goto Exit;
		}
	}
	
	puts("---------- Term! ---------");
	//mtk_pl_term();
	
	#if 0
	{
		FILE *fp;
		if ((fp = fopen("screw", "wb"))) {
			uint8_t tmp[4096];
			for (int i = 0; i < 0x400000; i += sizeof(tmp)) {
				printf("%08x\n", i);

				if (mtk_pl_recvBytes(tmp, sizeof(tmp)))
					break;

				fwrite(tmp, 1, sizeof(tmp), fp);
			}
			fclose(fp);
		}
	}
	#endif
	
Exit:
	mtk_pl_disconnect();
	return rc;
}
