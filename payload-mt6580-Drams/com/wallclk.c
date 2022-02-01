#include "mtk_regs.h"
#include "wallclk.h"

void wallclk_init(void) {
	/* setup GPT4 to tick once per microsecond */
	reg32_write(APXGPT_BASE+APXGPT_CLK(3), 26/2 - 1);
	reg32_write(APXGPT_BASE+APXGPT_CON(3), (3<<4)|(1<<1)|(1<<0)); /* opmode=3, clr=1, en=1 */
}


uint64_t micros() {
	/* TODO: handle wraparounds */
	return reg32_read(APXGPT_BASE+APXGPT_CNT(3));
}

void usleep(uint64_t us) {
	for (uint64_t target = micros() + us; micros() < target; );
}


uint64_t millis() {
	return micros() / 1000;
}

void delay(uint64_t ms) {
	for (uint64_t target = millis() + ms; millis() < target; );
}
