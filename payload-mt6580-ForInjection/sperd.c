#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <xprintf.h>
#include <mtk_regs.h>
#include <mtk_gpio.h>


void uputc(char c) {
	while ((reg32_read(UART1_BASE+0x014) & 0x60) != 0x60);
	reg32_write(UART1_BASE+0x000, c);
}

char ugetc(void) {
	while ((reg32_read(UART1_BASE+0x014) & 0x01) != 0x01);
	return reg32_read(UART1_BASE+0x000);
}


void ArmExceptionHandler(uint32_t *stack, int type) {
	static char *types[3] = {"Undefined Instruction", "Prefetch Abort", "Data Abort"};
	xprintf("Program Dead! %s\n", types[type]);
	xprintf("  r0:<%08x>   r1:<%08x>   r2:<%08x>   r3:<%08x>\n", stack[0],  stack[1],  stack[2],  stack[3] );
	xprintf("  r4:<%08x>   r5:<%08x>   r6:<%08x>   r7:<%08x>\n", stack[4],  stack[5],  stack[6],  stack[7] );
	xprintf("  r8:<%08x>   r9:<%08x>   sl:<%08x>   fp:<%08x>\n", stack[8],  stack[9],  stack[10], stack[11]);
	xprintf("  ip:<%08x>   sp:<%08x>   lr:<%08x>   pc:<%08x>\n", stack[12], stack[13], stack[14], stack[15]);
	xprintf("  lr:<%08x> spsr:<%08x>                        \n", stack[16], stack[17]                      );
}

void ArmIrqHandler(void) {
	xputs("[[ IRQ ]]\n");
}

void ArmFiqHandler(void) {
	xputs("[[ FIQ ]]\n");
}


void preinit(void) {
	/*** Config the MSDCs ! */ {
		/* msdc0 pads - even though works without it !! its woowo magic !! */
		gpio_iomux_cfg(41, 1); /* CLK */
		gpio_iomux_cfg(42, 1); /* CMD */
		gpio_iomux_cfg(43, 1); /* DAT0 */
		gpio_iomux_cfg(44, 1); /* DAT1 */
		gpio_iomux_cfg(45, 1); /* DAT2 */
		gpio_iomux_cfg(46, 1); /* DAT3 */
		gpio_iomux_cfg(47, 1); /* DAT4 */
		gpio_iomux_cfg(48, 1); /* DAT5 */
		gpio_iomux_cfg(49, 1); /* DAT6 */
		gpio_iomux_cfg(50, 1); /* DAT7 */
		gpio_iomux_cfg(51, 1); /* RSTB */
		
		/* woowo magic ! msdc0 - apparenly without it Linux fail to set 8bit ! and 4bit !  */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(2),  0, 0x7ff, 0x100); /* a)pull only CLK somehow */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(3),  0, 0x7ff, 0x6ff); /* a)??? */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(4),  0, 0x7ff, 0x100); /* a)enable pull only CLK */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(11), 0, 0x0fc, 0x000); /* a)??? */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(12), 0, 0x0f0, 0x0a0); /* a)??? */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(14), 0, 0x7ff, 0x000); /* a)??? */
		reg32_wsmask(IOCFG_B_BASE+IOCFG_XXXn(7),  0, 0x3ff, 0x3ff); /* b)whatever SMT thing except CLK */
		/* c) TODO */
		
		/* msdc1 pads - just in case !! */
		gpio_iomux_cfg(52, 1); /* CLK */
		gpio_iomux_cfg(53, 1); /* CMD */
		gpio_iomux_cfg(57, 1); /* DAT0 */
		gpio_iomux_cfg(56, 1); /* DAT1 */
		gpio_iomux_cfg(55, 1); /* DAT2 */
		gpio_iomux_cfg(54, 1); /* DAT3 */
		
		/* woowo magic ! msdc1 - apparenly without it SD Card does not work ! */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(2),  0, 0x3f, 0x01); /* a)pull only CLK somehow */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(3),  0, 0x3f, 0x3e); /* a)??? */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(4),  0, 0x3f, 0x01); /* a)enable pull only CLK */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(11), 0, 0x3f, 0x0c); /* a)??? */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(12), 0, 0x0f, 0x0a); /* a)??? */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(13), 0, 0x3f, 0x00); /* a)??? */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(7),  0, 0x3f, 0x3e); /* b)whatever SMT thing except CLK */
		reg32_wsmask(IOCFG_R_BASE+IOCFG_XXXn(9),  0, 0xfff, 0x249); /* c)??? */
		
		/* tweak clocks, route to the 208Mhz clock both MSDCs */
		//reg32_wsmask(TOPCKGEN_BASE+0x000, 11, 0x7, 0x7); /* MSDC0_HCLK = UPLL/6 */
		//reg32_wsmask(TOPCKGEN_BASE+0x000, 20, 0x7, 0x7); /* MSDC1_HCLK = UPLL/6 */
	}
}


int main(void) {
	/* setup UART2 (again some 16550A-compatible stuff?? great!!!) */ {
		reg32_write(TOPCKGEN_BASE+0x084, (1<<11)); /* enable uart2 clk */
	
		/* wait for all pending transmissions to end, to avoid garbage characters */
		while ((reg32_read(UART1_BASE+0x014) & 0x60) != 0x60);
	
		/* configure the format and baudrate */
		uint16_t dl = 26000000 / 4 / 921600;
		reg32_write(UART1_BASE+0x024, 2); /* HIGHSPEED = 1 */
		reg32_wsmask(UART1_BASE+0x00C, 7, 1, 1); /* access to DLL/DLH */
		reg32_write(UART1_BASE+0x000, dl);
		reg32_write(UART1_BASE+0x004, dl >> 8);
		reg32_wsmask(UART1_BASE+0x00C, 7, 1, 0); /* regular access */
		
		reg32_write(UART1_BASE+0x00C, 0x03); /* 8n1 */
		
		/* config pins */
		gpio_iomux_cfg(9,  4); /* G9  = URXD2 */
		gpio_iomux_cfg(10, 4); /* G10 = UTXD2 */
	}
	
	xdev_out(uputc); //xdev_in(ugetc);
	xputs("\n\e[1;33m*******Hello Mediatek!*******\e[0m\n");
	
	preinit();
	
	/* Jump */
	((void (*)())0x201000)();
	
	return 0;
}
