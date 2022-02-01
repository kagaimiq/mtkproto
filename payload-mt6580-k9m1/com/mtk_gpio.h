#ifndef _MTK_GPIO_H
#define _MTK_GPIO_H

#include <stdbool.h>

/* GPIO */
void gpio_iomux_cfg(int pad, int mode);
void gpio_set_direction(int pad, bool output);
void gpio_set_level(int pad, bool level);
bool gpio_get_level(int pad);

/* IOCFG */

#endif
