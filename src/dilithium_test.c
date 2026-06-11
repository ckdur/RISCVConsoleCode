#include <main.h>
#include <kprintf/kprintf.h>
#include <clkutils/clkutils.h>
#include "dilithium_test.h"

volatile uint32_t* dilithium_ctrl = (uint32_t*)0x0;
volatile uint32_t* dilithium_sign_mem = (uint32_t*)0x0;
volatile uint32_t* dilithium_pk_mem = (uint32_t*)0x0;
volatile uint32_t* dilithium_sk_mem = (uint32_t*)0x0;
volatile uint32_t* dilithium_msg_mem = (uint32_t*)0x0;

void dilithium_test() {
    while(dilithium_ctrl && dilithium_msg_mem && dilithium_sign_mem && dilithium_pk_mem && dilithium_sk_mem) {
        kputs("Dilithium (on-chip) test!\r\n");

        // TODO: Ask for test vectors

        kputs("Done\r\n");

        if(is_htif) break;

        // Wait for 1000 ms 
        clkutils_delay_ns(1000000000, 1000000000 / timescale_freq);
    }   
}
