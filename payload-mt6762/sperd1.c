#include <stdint.h>
#include <string.h>
#include <xprintf.h>
#include <wallclk.h>
#include <mtk_regs.h>


void uputc(int c) {
	while ((reg32_read(0x11002014) & 0x60) != 0x60);
	reg32_write(0x11002000, c);
}

int ugetc(void) {
	while ((reg32_read(0x11002014) & 0x01) != 0x01);
	return reg32_read(0x11002000);
}


void hexdump(void *ptr, int len) {
	for (int i = 0; i < len; i += 16) {
		xprintf("%08x: ", ptr+i);
		
		for (int j = 0; j < 16; j++) {
			if (i+j < len)
				xprintf("%02X ", *(uint8_t*)(ptr+i+j));
			else
				xputs("-- ");
		}
		
		xputs(" |");
		
		for (int j = 0; j < 16; j++) {
			uint8_t c = ' ';
			if (i+j < len) {
				c = *(uint8_t*)(ptr+i+j);
				if (c < 0x20 || c >= 0x7f) c = '.';
			}
			xputc(c);
		}
		
		xputs("|\n");
	}
}

void ArmExceptionHandler(uint32_t *stack, int type) {
	static char *types[3] = {"Undefined Instruction", "Prefetch Abort", "Data Abort"};
	xprintf("Program Dead! %s\n", types[type]);
	xprintf("  r0:<%08x>   r1:<%08x>   r2:<%08x>   r3:<%08x>\n", stack[0],  stack[1],  stack[2],  stack[3] );
	xprintf("  r4:<%08x>   r5:<%08x>   r6:<%08x>   r7:<%08x>\n", stack[4],  stack[5],  stack[6],  stack[7] );
	xprintf("  r8:<%08x>   r9:<%08x>   sl:<%08x>   fp:<%08x>\n", stack[8],  stack[9],  stack[10], stack[11]);
	xprintf("  ip:<%08x>   sp:<%08x>   lr:<%08x>   pc:<%08x>\n", stack[12], stack[13], stack[14], stack[15]);
	xprintf("  lr:<%08x> spsr:<%08x>                        \n", stack[16], stack[17]                      );
	
	xputs("=== press any key to try again ===\n");
	ugetc();
}

void main2(void) {
	uint32_t tmp;
	asm volatile ("mrc p15, 0, %0, c0, c0, 5" :: "r"(tmp));
	
	xprintf("Who i am? %08x!\n", tmp);
}


/* ---------------------------------------- */
struct cmdtable_elm {
	char *cmdname;
	char *cmdbrief;
	char *cmdusage;

	int (*cmdhandler)(const struct cmdtable_elm *cmd, int argc, char **argv);
};

int execute_cmd(char *cmd);
int do_help(const struct cmdtable_elm *cmd, int argc, char **argv);
int do_mm(const struct cmdtable_elm *cmd, int argc, char **argv);
int do_nm(const struct cmdtable_elm *cmd, int argc, char **argv);
int do_base(const struct cmdtable_elm *cmd, int argc, char **argv);

const struct cmdtable_elm cmdtable[] = {
	{
		"help",
		"print all commands list and their brief info or specific command's usage.",
		"[<command>]\n"
		"   <command> - optional command name to print its usage, if not specified,\n"
		"      print all commands and their brief info",
		do_help
	},
	{
		"mm",
		"memory modify (auto incrementing address)",
		"[<addr>]\n"
		"   <addr> - optional address, if not specified then it uses the last address\n"
		"      or the one that was set with `addr` command.\n",
		do_mm
	},
	{
		"nm",
		"memory modify (fixed address)",
		"[<addr>]\n"
		"   <addr> - optional address, if not specified then it uses the last address\n"
		"      or the one that was set with `addr` command.\n",
		do_nm
	},
	{
		NULL
	}
};

uint32_t base_addr;

int do_mm_common(const struct cmdtable_elm *cmd, int argc, char **argv, int type) {
	if (argc > 1) {
		char *a = argv[1];
		xatoi(&a, &base_addr);
	}
	
	for (;;) {
		xprintf("%08x: %08x ? ", base_addr, reg32_read(base_addr));
		
		char tmp[64];
		xgets(tmp, sizeof(tmp));
		
		if (tmp[0] != 0) {
			uint32_t val;
			char *a = tmp;
			if (!xatoi(&a, &val)) break;
			reg32_write(base_addr, val);
		}
		
		if ((type >> 2) == 0) base_addr += (1 << (type & 3));
	}
	
	return 0;
}

int do_mm(const struct cmdtable_elm *cmd, int argc, char **argv) {
	return do_mm_common(cmd, argc, argv, 2);
}

int do_nm(const struct cmdtable_elm *cmd, int argc, char **argv) {
	return do_mm_common(cmd, argc, argv, 5);
}

int do_help(const struct cmdtable_elm *cmd, int argc, char **argv) {
	if (argc < 2) {
		for (const struct cmdtable_elm *elm = cmdtable; elm->cmdname; elm++) {
			xprintf(
				"[%s]\n"
				" - %s"
				"\n"
				"\n", 
				elm->cmdname, elm->cmdbrief
			);
		}
	} else {
		for (const struct cmdtable_elm *elm = cmdtable; elm->cmdname; elm++) {
			if (!strcmp(argv[1], elm->cmdname)) {
				xprintf(
					"[%s]\n"
					" - %s\n"
					"\n"
					"Usage: %s\n", 
					elm->cmdname, elm->cmdbrief, elm->cmdusage
				);
				return 0;
			}
		}

		xprintf("the command `%s` was not found.\n", argv[1]);
		return 1;
	}

	return 0;
}


int execute_cmd(char *cmd) {
	int xargc = 0;
	char *xargv[16];
	char cmdstr[512];

	// -------grab arguments------
	char *sp = cmd, *sp2 = cmdstr;
	int isEscaped = 0, len;
	
	/* parse string, split args, and handle escape sequences */
	while (*sp) {
		xargv[xargc++] = sp2;
		len = 0;

		while (*sp) {
			if ((*sp == ' ') && !isEscaped) {
				sp++;
				
				/* if the arg is not empty then we will exit, otherwise dont make empty args!! */
				if (len > 0) break;
				else         continue;
			}

			if (*sp == '"') { 
				sp++; 
				isEscaped = !isEscaped;
			}

			if ((*sp == '\\') && !isEscaped) 
				sp++;

			*sp2++ = *sp++;
			len++;
		}

		*sp2++ = '\0';

		if (xargc >= 16) {
			xprintf("ignoring all args after the %dth...\n", xargc);
			break;
		}
	}

	// -----------run commands------------
	if (xargc >= 1) {
		// lookup commands
		for (const struct cmdtable_elm *elm = cmdtable; elm->cmdname; elm++) {
			if (!strcmp(xargv[0], elm->cmdname)) {
				if (elm->cmdhandler)
					return elm->cmdhandler(elm, xargc, xargv);
			}
		}

		xprintf("the command `%s` was not found or has no handler.\n", xargv[0]);
	}

	return 0;
}

/* ---------------------------------------- */


int main(void) {
	xdev_out(uputc); xdev_in(ugetc);
	xputs("\n\e[1;40;33;7m <<    Mediatek MT6762V 32-bits    >> \e[0m\n");
	
	wallclk_init();
	
	{
		uint32_t tmp;
		asm volatile("mov %0, pc" :: "r"(tmp));
		xprintf("We are at %08x!\n", tmp);
	}
	
	xputs("======================================\n");
	
	for (;;) {
		xputs("=> ");
		
		char str[512];
		xgets(str, sizeof(str));
		
		int rc = execute_cmd(str);
		if (rc != 0) xprintf("[rc->%d]\n", rc);
	}
	
	return 0;
}
