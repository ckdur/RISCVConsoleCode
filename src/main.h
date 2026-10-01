#ifndef _MAIN_H
#define _MAIN_H

#include <stdint.h>

extern unsigned long uart_reg;
extern int timescale_freq;
extern int tlclk_freq;

extern volatile uint32_t* spi;
extern volatile uint32_t* gpio;
extern volatile uint32_t* adc;

void test();

#endif // _MAIN_H
