#include "mtk_regs.h"
#include "mtk_gpio.h"

/* ============ GPIO =========== */
void gpio_iomux_cfg(int pad, int mode) {
	if (pad < 0 || pad > 96) return; /* sanity check */
	reg32_wsmask(GPIO_BASE+GPIO_MODEn(pad/8), (pad%8)*4, 0xf, mode);
}

void gpio_set_direction(int pad, bool output) {
	if (pad < 0 || pad > 96) return; /* sanity check */
	reg32_wsmask(GPIO_BASE+GPIO_DIRn(pad/32), pad%32, 1, output);
}

void gpio_set_level(int pad, bool level) {
	if (pad < 0 || pad > 96) return; /* sanity check */
	reg32_wsmask(GPIO_BASE+GPIO_OUTn(pad/32), pad%32, 1, level);
}

bool gpio_get_level(int pad) {
	if (pad < 0 || pad > 96) return false; /* sanity check */
	return reg32_rsmask(GPIO_BASE+GPIO_INn(pad/32), pad%32, 1);
}

/* ============ IOCFG =========== */

