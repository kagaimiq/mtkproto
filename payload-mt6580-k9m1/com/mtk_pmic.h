#ifndef _PMIC_H
#define _PMIC_H

#include <stdint.h>
#include <stdbool.h>

/* PWRAP */
int pwrap_init(void);
int pwrap_wacs2_io(uint16_t addr, bool write, uint16_t val);

/* PMIC Common */
int pmic_read(uint16_t addr);
int pmic_write(uint16_t addr, uint16_t val);
int pmic_wsmask(uint16_t addr, int shift, uint16_t mask, uint16_t val);
int pmic_rsmask(uint16_t addr, int shift, uint16_t mask);

/* PMIC Specifics */
int pmic_rtc_write_trigger(void);

#endif
