#ifndef _MAIN_H
#define _MAIN_H

#include <inttypes.h>
extern unsigned long uart_reg;
extern int timescale_freq;
extern int tlclk_freq;
extern volatile uint32_t* pusher;
extern volatile uint32_t* bls12381_ctrl;
extern volatile uint32_t* bls12381_imem;
extern volatile uint32_t* bls12381_omem;

void test();

#endif // _MAIN_H
